/*
 * fdraw.c
 */

#include "commands.h"
#include "fdraw.h"
#include "figviewer.h"
#include "globals.h"
#include "manipulators.h"
#include "selection.h"
#include "tools.h"
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/action.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/transform.h>
#include <stdio.h>

class Flipper : public GlyphVisitorImpl {
public:
    Flipper(DeckObjRef deck);
    virtual ~Flipper();
    void flip_to(long);
    Boolean visit(GlyphRef, GlyphOffsetRef);
private:
    DeckObjRef deck_;
    long card_no_;
    long cur_card_;
};

Flipper::Flipper (DeckObjRef deck) {
    deck_ = DeckObj::_duplicate(deck);
}

Flipper::~Flipper () {
    Fresco::unref(deck_);
}

void Flipper::flip_to (long c) {
    card_no_ = c;
    cur_card_ = 0;
    deck_->visit_children(this);
}

Boolean Flipper::visit(GlyphRef, GlyphOffsetRef go) {
    if (card_no_ == cur_card_) {
        deck_->flip_to(go);
        return false;
    } else {
        cur_card_++;
        return true;
    }
}

class TransformSetter : public MonoGlyph {
public:
    TransformSetter(GlyphRef g, TransformObjRef t);
    ~TransformSetter();
    
    TransformObjRef _c_transform(); //+ Glyph::transform
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void request(Glyph::Requisition& r); //+ Glyph::request
    void child_allocate(Glyph::AllocationInfo&);
protected:
    TransformObjRef t_;
    RegionImpl natural_;
};

TransformSetter::TransformSetter(GlyphRef g, TransformObjRef t) {
    body(g);
    t_ = TransformObj::_duplicate(t);
}

TransformSetter::~TransformSetter() { 
    Fresco::unref(t_);
}

TransformObjRef TransformSetter::_c_transform () {
    Fresco::ref(t_);
    return t_;
}

//+ TransformSetter(Glyph::request)
void TransformSetter::request (Glyph::Requisition& req) {
    MonoGlyph::request(req);

    RegionImpl region;
    region.lower_.x = 0.0;
    region.upper_.x = req.x.natural;
    region.xalign_ = req.x.align;

    region.lower_.y = 0.0;
    region.upper_.y = req.y.natural;
    region.yalign_ = req.y.align;

    natural_.copy(&region);
    region.transform(t_);

    Coord x_lead = -region.lower_.x, x_trail = region.upper_.x;
    Coord y_lead = -region.lower_.y, y_trail = region.upper_.y;
    GlyphImpl::require_lead_trail(
        req.x, x_lead, x_lead, x_lead, x_trail, x_trail, x_trail
    );
    GlyphImpl::require_lead_trail(
        req.y, y_lead, y_lead, y_lead, y_trail, y_trail, y_trail
    );
}

//+ TransformSetter(Glyph::traverse)
void TransformSetter::traverse(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    p->push_matrix();
    Glyph::AllocationInfo a;
    a.transform = nil;
    a.allocation = t->allocation();
    a.damage = nil;
    child_allocate(a);
    p->transform(a.transform);
    MonoGlyph::traverse(t);
    p->pop_matrix();
    Fresco::unref(a.transform);
}

void TransformSetter::child_allocate(Glyph::AllocationInfo& a) {
    if (is_nil(a.transform)) {
        a.transform = new TransformImpl;
    }
    TransformImpl tx;
    tx.load(t_);
    if (is_not_nil(a.allocation)) {
        Vertex lower, upper;
        a.allocation->bounds(lower, upper);
        tx.translate(lower);
        a.allocation->copy(&natural_);
    }
    a.transform->premultiply(&tx);
}

class AllocationTranslator : public MonoGlyph {
    //. During traversal, an AllocationTranslator converts its
    //. given allocation's origin into a matrix translation.
    //. This is useful as a gateway between layout objects,
    //. who specify position using an allocation, and figure
    //. objects, who ignore allocation during traversal.
public:
    AllocationTranslator(GlyphRef g);
    ~AllocationTranslator();
    
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void child_allocate(Glyph::AllocationInfo&);
};

AllocationTranslator::AllocationTranslator(GlyphRef g) {
    body(g);
}

AllocationTranslator::~AllocationTranslator() { }

//+ AllocationTranslator(Glyph::traverse)
void AllocationTranslator::traverse(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    p->push_matrix();
    Glyph::AllocationInfo a;
    a.transform = new TransformImpl;
    a.allocation = t->allocation();
    a.damage = nil;
    child_allocate(a);

    p->transform(a.transform);
    MonoGlyph::traverse(t);
    p->pop_matrix();

    Fresco::unref(a.transform);
    Fresco::unref(a.allocation);
}

void AllocationTranslator::child_allocate(Glyph::AllocationInfo& a) {
    TransformImpl t;
    Vertex v;
    if (is_nil(a.transform)) {
        a.transform = new TransformImpl;
    }
    if (is_nil(a.allocation)) {
        a.allocation = new RegionImpl;
    }
    TransformRef tx = a.transform;
    RegionRef agiven = a.allocation;
    agiven->origin(v);
    t.translate(v);
    tx->premultiply(&t);
 
    t.load_identity();
    v.x = -v.x; v.y = -v.y; v.z = -v.z;
    t.translate(v);
    agiven->transform(&t);
}

class Requestor : public MonoGlyph {
    //. A Requestor always returns a constant requisition. It is
    //. useful as a wrapper for glyphs whose requisition changes
    //. but who wish to be laid out as though their requisition
    //. were constant.
public:
    Requestor(
        GlyphRef g, Alignment xalign = .5, Alignment yalign = .5,
        Coord xspan = 1, Coord yspan = 1
    );
    Requestor(GlyphRef g, const Glyph::Requisition& r);
    ~Requestor();
    void request(Glyph::Requisition& r); //+ Glyph::request

protected:
    Glyph::Requisition req_;
};

Requestor::Requestor(GlyphRef g, const Glyph::Requisition& r) {
    body(g);
    req_ = r;
}

Requestor::~Requestor() { }

Requestor::Requestor(
    GlyphRef g, Alignment xalign, Alignment yalign, Coord xspan, Coord yspan
) {
    body(g);
    GlyphImpl::default_requisition(req_);
    Glyph::Requirement* rx = GlyphImpl::requirement(req_, X_axis);
    Glyph::Requirement* ry = GlyphImpl::requirement(req_, Y_axis);
    rx->align = xalign;
    ry->align = yalign;
    rx->natural = xspan;
    ry->natural = yspan; 
}

//+ Requestor(Glyph::request)
void Requestor::request(Glyph::Requisition& r) {
    r = req_;
}

class Patch : public MonoGlyph {
public:
    Patch(GlyphRef);

    virtual void traverse(GlyphTraversalRef);
    virtual void redraw();
private:
    GlyphTraversalRef gt_;
    TransformImpl tx_;
    RegionImpl r_;
};

Patch::Patch(GlyphRef g) {
    gt_ = nil;
    body(g);
}

void Patch::traverse (GlyphTraversalRef gt) {
    if (gt->op() == GlyphTraversal::draw) {
        if (is_nil(gt_)) {
            gt_ = gt->_c_trail();
            Fresco::ref(gt_);
        } 
        r_.copy(gt->allocation());
        tx_.load(gt->painter()->matrix());
    } else {
        MonoGlyph::traverse(gt);
    }
}

void Patch::redraw () {
    gt_->allocation()->copy(&r_);
    PainterObj p = gt_->painter();
    p->push_matrix();
    p->matrix()->load(&tx_);
    MonoGlyph::traverse(gt_);
    p->pop_matrix();
}

class ToolSetter : public ActionImpl{
public:
    ToolSetter(FigViewer*, Tool*, TelltaleRef bs);
    ~ToolSetter();

    virtual void execute();

    static TelltaleRef old_bs_;
private:
    FigViewer* viewer_;
    Tool* tool_;
    TelltaleRef bs_;
};

TelltaleRef ToolSetter::old_bs_ = nil;

ToolSetter::ToolSetter(FigViewer* v, Tool* t, TelltaleRef bs) {
    viewer_ = v;
    tool_ = t;
    bs_ = Telltale::_duplicate(bs);
}

ToolSetter::~ToolSetter() {
    delete tool_;
    Fresco::unref(bs_);
}

void ToolSetter::execute () {
    viewer_->current_tool(tool_);
    old_bs_->clear(Telltale::active);
    bs_->set(Telltale::active);
    old_bs_ = bs_;
}

FDraw::FDraw (Fresco* f) : ViewerImpl (f) {
    viewer_ = new FigViewer(f);
    append_viewer(viewer_);
    subviews_ = new ViewerImpl(f);
    editing_ = true;
    body(interior());
}

FDraw::~FDraw () {
    Fresco::unref((GlyphRef)viewer_);
}

Boolean FDraw::key_press (GlyphTraversalRef, EventRef e) {
    if (e->character() == '\033') {
        if (editing_) {
            editing_ = false;
            flipper_->flip_to(1);
            append_viewer(subviews_);
            need_redraw();

            SelectCmd unselect(viewer_, false);
            Selection* sel = viewer_->selection()->copy();
            for (long i = 0; i < sel->count(); i++) {
                sel->item(i)->execute(&unselect);
            }
            delete sel;
        } else {
            editing_ = true;
            flipper_->flip_to(0);
            subviews_->remove_viewer();
            need_redraw();
        }
    }
    return true;
}

void FDraw::traverse (GlyphTraversalRef gt) {
    ViewerImpl::traverse(gt);
    if (gt->op() == GlyphTraversal::draw && editing_) {
        patch_->redraw();
    }
}

Glyph_tmp FDraw::interior () {
    Fresco* f = fresco_instance();
    LayoutKit layouts = f->layout_kit();

    Coord fil = layouts->fil();
    Glyph hbox1 = layouts->hbox();
    Glyph hbox2 = layouts->hbox();
    Glyph hbox3 = layouts->hbox();

    Glyph vbox1 = layouts->vbox();
    Glyph vbox2 = layouts->vbox();
    DeckObj deck = layouts->deck();
    flipper_ = new Flipper(deck);

    Glyph tool = tools();
    patch_ = new Patch(tool);

    hbox1->append(patch_);
    hbox2->append(
        new AllocationTranslator(layouts->natural(viewer_, 300, 300))
    );
    hbox3->append(commands());

    vbox1->append(hbox1);
    vbox1->append(hbox2);
    vbox1->append(hbox3);

    vbox2->append(
        new AllocationTranslator(layouts->natural(viewer_, 300, 300))
    );
    deck->append(vbox1);
    deck->append(layouts->hcenter_aligned(vbox2, 0.0));
    flipper_->flip_to(0);

    return DeckObj::_duplicate(deck->_obj());
}
    
declareActionCallback(Command)
implementActionCallback(Command)

Glyph_tmp FDraw::commands() {
    Fresco* f = fresco_instance();
    LayoutKit layouts = f->layout_kit();
    Glyph hbox = layouts->hbox();

    hbox->append(command_button("Group", new GroupCmd(viewer_)));
    hbox->append(command_button("Ungroup", new UngroupCmd(viewer_)));
    hbox->append(command_button("Instance", new InstanceCmd(viewer_)));
    hbox->append(command_button("Copy", new CopyCmd(viewer_)));
    hbox->append(command_button("Delete", new DeleteCmd(viewer_)));

    hbox->append(
        command_button("HTile", new GroupCmd(viewer_, new HBoxManip))
    );
    hbox->append(
        command_button("VTile", new GroupCmd(viewer_, new VBoxManip))
    );
    return Glyph::_duplicate(hbox);
}

static Glyph_tmp create_label(const char* label, FigureKitRef figures) {
    return figures->label(
        figures->default_style(), Fresco::string_ref(label)
    );
}

Glyph_tmp FDraw::tools() {
    Fresco* fresco = fresco_instance();

    LayoutKit layouts = fresco->layout_kit();
    WidgetKit widgets = fresco->widget_kit();
    FigureKit figures = fresco->figure_kit();
    Coord fil = layouts->fil();

    Telltale group = widgets->telltale_group();

    SelectTool* selectTool = new SelectTool;
    MoveTool* moveTool = new MoveTool;
    ScaleTool* scaleTool = new ScaleTool;
    RotateTool* rotateTool = new RotateTool;
    
    Manipulator* line_manip = new LineManip;
    Manipulator* rect_manip = new RectManip;
    Manipulator* ellipse_manip = new EllipseManip;
    Manipulator* openbspline_manip = new Open_BSplineManip;
    Manipulator* closedbspline_manip = new Closed_BSplineManip;
    Manipulator* multiline_manip = new MultiLineManip;
    Manipulator* polygon_manip = new PolygonManip;

    ButtonManip* button_manip = new ButtonManip(subviews_, &editing_);

    CreateTool* line_tool = new CreateTool(line_manip);
    CreateTool* multiline_tool = new CreateTool(multiline_manip);
    CreateTool* openbspline_tool = new CreateTool(openbspline_manip);
    CreateTool* rect_tool = new CreateTool(rect_manip);
    CreateTool* ellipse_tool = new CreateTool(ellipse_manip);
    CreateTool* polygon_tool = new CreateTool(polygon_manip);
    CreateTool* closedbspline_tool = new CreateTool(closedbspline_manip);
    CreateTool* button_tool = new CreateTool(button_manip);

    Vertex* vclosed = new Vertex[5];
    vclosed[0].x = 3, vclosed[0].y = 0;
    vclosed[1].x = 16, vclosed[1].y = 3;
    vclosed[2].x = 17, vclosed[2].y = 9;
    vclosed[3].x = 9, vclosed[3].y = 16;
    vclosed[4].x = 0, vclosed[4].y = 6;

    Vertex* vopen = new Vertex[4];
    vopen[0].x = 0, vopen[0].y = 0;
    vopen[1].x = 7.5, vopen[1].y = 3.75;
    vopen[2].x = 7.5, vopen[2].y = 11.25;
    vopen[3].x = 15, vopen[3].y = 15;

    FigureKit::Vertices v_closed(5, 5, vclosed);
    FigureKit::Vertices v_open(4, 4, vopen);

    Button line_button = tool_button(
        layouts->center(
            figures->line(figures->default_style(), 0, 0, 15, 15)
        ), line_tool, group
    );
    Button multiline_button = tool_button(
        layouts->center(
            figures->multiline(
                FigureKit::stroke, figures->default_style(), v_open
            )
        ), multiline_tool, group
    );
    Button openbspline_button = tool_button(
        layouts->center(
            figures->open_bspline(
                FigureKit::stroke, figures->default_style(), v_open
            )
        ), openbspline_tool, group
    );
    Button rect_button = tool_button(
        layouts->center(
            figures->rectangle(
                FigureKit::stroke, figures->default_style(), 0, 0, 18, 15
            )
        ), rect_tool, group
    );
    Button ellipse_button = tool_button(
        layouts->center(
            figures->ellipse(
                FigureKit::stroke, figures->default_style(), 0, 0, 12, 8
            )
        ), ellipse_tool, group
    );
    Button polygon_button = tool_button(
        layouts->center(
            figures->polygon(
                FigureKit::stroke, figures->default_style(), v_closed
            )
        ), polygon_tool, group
    );
    vclosed[3].x = 9, vclosed[3].y = 18;
    Button closedbspline_button = tool_button(
        layouts->center(
            figures->closed_bspline(
                FigureKit::stroke, figures->default_style(), v_closed
            )
        ), closedbspline_tool, group
    );

    TransformImpl* t = new TransformImpl;
    Vertex s; s.x = 0.55; s.y = 0.55; s.z = 0;
    t->scale(s);
    Button push_button = tool_button(
        layouts->center(
            new TransformSetter(
                widgets->palette_button(layouts->vspace(10), nil), t
            )
        ), button_tool, group
    );
    Button choose = tool_button(
        create_label("Select", figures), selectTool, group
    );
    Button move = tool_button(
        create_label("Move", figures), moveTool, group
    );
    Button scale = tool_button(
        create_label("Scale", figures), scaleTool, group
    );
    Button rotate = tool_button(
        create_label("Rotate", figures), rotateTool, group
    );

    Telltale bs = choose->state();
    bs->set(Telltale::active);
    viewer_->current_tool(selectTool);
    ToolSetter::old_bs_ = bs;

    Glyph hbox1 = layouts->hbox();
    Glyph hbox2 = layouts->hbox();
    Glyph vbox = layouts->vbox();

    hbox1->append(layouts->vcenter_aligned(line_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(multiline_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(openbspline_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(rect_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(ellipse_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(polygon_button, 1.0));
    hbox1->append(layouts->vcenter_aligned(closedbspline_button, 1.0));

    hbox2->append(layouts->hglue_fil(0));
    hbox2->append(layouts->vcenter_aligned(choose, 1.0));
    hbox2->append(layouts->vcenter_aligned(move, 1.0));
    hbox2->append(layouts->vcenter_aligned(scale, 1.0));
    hbox2->append(layouts->vcenter_aligned(rotate, 1.0));
    hbox2->append(layouts->vcenter_aligned(push_button, 1.0));
    hbox2->append(layouts->hglue_fil(0));

    vbox->append(layouts->vfixed(hbox1, 26));
    vbox->append(hbox2);

    return Glyph::_duplicate(vbox);
}

Glyph_tmp FDraw::command_button(const char* label, Command* c) {
    Fresco* f = fresco_instance();

    FigureKit figures = f->figure_kit();
    WidgetKit widgets = f->widget_kit();

    Button button = widgets->push_button(
	figures->label(
	    figures->default_style(), Fresco::string_ref(label)
        ),
	new ActionCallback(Command)(c, &Command::execute)
    );
    append_viewer(button);
    return Glyph::_duplicate((GlyphRef)button);
}
	
Button_tmp FDraw::tool_button(
    GlyphRef g, Tool* t, TelltaleRef  group
) {
    Fresco* f = fresco_instance();
    WidgetKit widgets = f->widget_kit();

    Button button = widgets->push_button(g, nil);
    Telltale bs = button->state();
    bs->current(group);

    ToolSetter* ts = new ToolSetter(viewer_, t, bs);
    button->click_action(ts);
    append_viewer(button);
    return Button::_duplicate(button);
}


