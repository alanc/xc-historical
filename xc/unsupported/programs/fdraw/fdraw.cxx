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

class TransformSetter : public MonoGlyph {
public:
    TransformSetter(GlyphRef g, TransformRef t);
    ~TransformSetter();
    
    Transform_return transformation(); //+ Glyph::transformation
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
    void request(Glyph::Requisition& r); //+ Glyph::request
    void child_allocate(Glyph::AllocationInfo&);
protected:
    TransformRef t_;
    RegionImpl natural_;
};

TransformSetter::TransformSetter(GlyphRef g, TransformRef t) {
    body(g);
    t_ = Transform::_duplicate(t);
}

TransformSetter::~TransformSetter() { 
    Fresco::unref(t_);
}

//+ TransformSetter(Glyph::transformation)
Transform_return TransformSetter::transformation() {
    return Transform::_duplicate(t_);
}

//+ TransformSetter(Glyph::request)
void TransformSetter::request(Glyph::Requisition& req) {
    MonoGlyph::request(req);

    RegionImpl region;
    region.lower_.x = -req.x.natural*req.x.align;
    region.upper_.x = region.lower_.x + req.x.natural;
    region.xalign_ = req.x.align;
 
    region.lower_.y = -req.y.natural*req.y.align;
    region.upper_.y = region.lower_.y + req.y.natural;
    region.yalign_ = req.y.align;
    region.defined_ = true;
 
    natural_.copy(&region);
    region.apply_transform(t_);
 
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
void TransformSetter::traverse(GlyphTraversal_in t) {
    Painter_var p = t->current_painter();
    p->push_matrix();
    Glyph::AllocationInfo a;
    a.transformation = nil;
    a.allocation = t->allocation();
    a.damaged = nil;
    child_allocate(a);
    p->premultiply(a.transformation);
    MonoGlyph::traverse(t);
    p->pop_matrix();
    Fresco::unref(a.transformation);
}

void TransformSetter::child_allocate(Glyph::AllocationInfo& a) {
    if (is_nil(a.transformation)) {
        a.transformation = new TransformImpl;
    }
    TransformImpl tx;
    tx.load(t_);
    if (is_not_nil(a.allocation)) {
        Vertex lower, upper;
        a.allocation->bounds(lower, upper);
        tx.translate(lower);
        a.allocation->copy(&natural_);
    }
    a.transformation->premultiply(&tx);
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
    
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
    void child_allocate(Glyph::AllocationInfo&);
};

AllocationTranslator::AllocationTranslator(GlyphRef g) {
    body(g);
}

AllocationTranslator::~AllocationTranslator() { }

//+ AllocationTranslator(Glyph::traverse)
void AllocationTranslator::traverse(GlyphTraversal_in t) {
    Painter_var p = t->current_painter();
    p->push_matrix();
    Glyph::AllocationInfo a;
    a.transformation = new TransformImpl;
    a.allocation = t->allocation();
    a.damaged = nil;
    child_allocate(a);

    p->premultiply(a.transformation);
    MonoGlyph::traverse(t);
    p->pop_matrix();

    Fresco::unref(a.transformation);
    Fresco::unref(a.allocation);
}

void AllocationTranslator::child_allocate(Glyph::AllocationInfo& a) {
    TransformImpl t;
    Vertex v;
    if (is_nil(a.transformation)) {
        a.transformation = new TransformImpl;
    }
    if (is_nil(a.allocation)) {
        a.allocation = new RegionImpl;
    }
    TransformRef tx = a.transformation;
    RegionRef agiven = a.allocation;
    agiven->origin(v);
    t.translate(v);
    tx->premultiply(&t);
 
    t.load_identity();
    v.x = -v.x; v.y = -v.y; v.z = -v.z;
    t.translate(v);
    agiven->apply_transform(&t);
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
            gt_ = GlyphTraversal::_return_ref(gt->trail());
            Fresco::ref(gt_);
        } 
        r_.copy(gt->allocation());
        tx_.load(gt->current_painter()->current_matrix());
    } else {
        MonoGlyph::traverse(gt);
    }
}

void Patch::redraw () {
    gt_->allocation()->copy(&r_);
    Painter_var p = gt_->current_painter();
    p->push_matrix();
    p->current_matrix()->load(&tx_);
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
    // old_bs_->clear(Telltale::active);
    // bs_->set(Telltale::active);
    // old_bs_ = bs_;
}

FDraw::FDraw (Fresco* f) : ViewerImpl(f, true) {
    viewer_ = new FigViewer(f, &editing_);
    append_viewer(viewer_);
    editing_ = true;
    body(interior());
}

FDraw::~FDraw () {
    Fresco::unref(ViewerRef(viewer_));
}

Boolean FDraw::key_press (GlyphTraversalRef, EventRef e) {
    if (e->character() == '\033') {
        if (editing_) {
            editing_ = false;
            flip_to(1);
            need_redraw();

            SelectCmd unselect(viewer_, false);
            Selection* sel = viewer_->selection()->copy();
            for (long i = 0; i < sel->count(); i++) {
                sel->item(i)->execute(&unselect);
            }
            delete sel;
        } else {
            editing_ = true;
            flip_to(1);
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

Glyph_return FDraw::interior () {
    Fresco* f = fresco_instance();
    LayoutKit_var layouts = f->layout_kit();
    WidgetKit_var widgets = f->widget_kit();

    deck_ = Glyph::_return_ref(layouts->deck());

    /* bottom layer: run mode */
    deck_->append(
	_tmp(layouts->center_aligned(
	    _tmp(new AllocationTranslator(
		layouts->natural(viewer_, 5*72.0, 7*72.0)
	    )), 0.0, 1.0
	))
    );

    /* top layer: edit mode */
    Glyph_var top = layouts->hbox();

    patch_ = new Patch(_tmp(tools()));
    top->append(patch_);
    top->append(
	_tmp(layouts->vcenter_aligned(
	    _tmp(new AllocationTranslator(
		layouts->natural(viewer_, 5*72.0, 7*72.0)
	    )), 1.0
	))
    );
    top->append(_tmp(commands()));

    deck_->append(top);

    return layouts->flexible_fil(deck_);
}
    
declareActionCallback(Command)
implementActionCallback(Command)

Glyph_return FDraw::commands() {
    Fresco* f = fresco_instance();
    LayoutKit_var layouts = f->layout_kit();
    Glyph_return box = Glyph::_return_ref(layouts->vbox());

    box->append(_tmp(command_button("Group", new GroupCmd(viewer_))));
    box->append(_tmp(command_button("Ungroup", new UngroupCmd(viewer_))));
    box->append(_tmp(command_button("Instance", new InstanceCmd(viewer_))));
    box->append(_tmp(command_button("Copy", new CopyCmd(viewer_))));
    box->append(_tmp(command_button("Delete", new DeleteCmd(viewer_))));

    box->append(
        _tmp(command_button("HBox", new GroupCmd(viewer_, new HBoxManip)))
    );
    box->append(
        _tmp(command_button("VBox", new GroupCmd(viewer_, new VBoxManip)))
    );
    box->append(_tmp(layouts->vfil()));
    return box;
}

static Glyph_return create_label(const char* label, FigureKitRef figures) {
    return figures->label(
        _tmp(figures->default_style()), Fresco::tmp_string_ref(label)
    );
}

Glyph_return FDraw::tools() {
    Fresco* fresco = fresco_instance();

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

    ButtonManip* button_manip = new ButtonManip(&editing_);

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

    LayoutKit_var layouts = fresco->layout_kit();
    WidgetKit_var widgets = fresco->widget_kit();
    FigureKit_var figures = fresco->figure_kit();

    Telltale_var group = widgets->telltale_group();

    Button_var line_button = tool_button(
        _tmp(layouts->lmargin(
	    _tmp(figures->line(_tmp(figures->default_style()), 0, 0, 15, 15)),
	    9.0
	)), line_tool, group
    );
    Button_var multiline_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->multiline(
                FigureKit::stroke, _tmp(figures->default_style()), v_open
            )), 9.0
        )), multiline_tool, group
    );
    Button_var openbspline_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->open_bspline(
                FigureKit::stroke, _tmp(figures->default_style()), v_open
            )), 9.0
        )), openbspline_tool, group
    );
    Button_var rect_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->rectangle(
                FigureKit::stroke, _tmp(figures->default_style()), 0, 0, 18, 15
            )), 9.0
        )), rect_tool, group
    );
    Button_var ellipse_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->ellipse(
                FigureKit::stroke, _tmp(figures->default_style()), 0, 0, 12, 8
            )), 9.0
        )), ellipse_tool, group
    );
    Button_var polygon_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->polygon(
                FigureKit::stroke, _tmp(figures->default_style()), v_closed
            )), 9.0
        )), polygon_tool, group
    );
    vclosed[3].x = 9, vclosed[3].y = 18;
    Button_var closedbspline_button = tool_button(
        _tmp(layouts->lmargin(
            _tmp(figures->closed_bspline(
                FigureKit::stroke, _tmp(figures->default_style()), v_closed
            )), 9.0
        )), closedbspline_tool, group
    );

    TransformImpl* t = new TransformImpl;
    Vertex s; s.x = 0.55; s.y = 0.55; s.z = 0;
    t->scale(s);
    Button_var push_button = tool_button(
        _tmp(layouts->center(
            _tmp(new TransformSetter(
                _tmp(widgets->palette_button(
		    _tmp(layouts->vspace(10)), nil
		)), t
            ))
        )), button_tool, group
    );
    Button_var choose = tool_button(
        _tmp(create_label("Select", figures)), selectTool, group
    );
    Button_var move = tool_button(
        _tmp(create_label("Move", figures)), moveTool, group
    );
    Button_var scale = tool_button(
        _tmp(create_label("Scale", figures)), scaleTool, group
    );
    Button_var rotate = tool_button(
        _tmp(create_label("Rotate", figures)), rotateTool, group
    );

    Telltale_var bs = choose->state();
    bs->set(Telltale::chosen);
    viewer_->current_tool(selectTool);
    ToolSetter::old_bs_ = bs;

    Glyph_return g = Glyph::_return_ref(layouts->vbox());
    g->append(_tmp(choose));
    g->append(_tmp(move));
    g->append(_tmp(scale));
    g->append(_tmp(rotate));
    g->append(_tmp(line_button));
    g->append(_tmp(multiline_button));
    g->append(_tmp(openbspline_button));
    g->append(_tmp(rect_button));
    g->append(_tmp(ellipse_button));
    g->append(_tmp(polygon_button));
    g->append(_tmp(closedbspline_button));
    g->append(_tmp(push_button));
    g->append(_tmp(layouts->vfil()));
    return g;
}

Glyph_return FDraw::command_button(const char* label, Command* c) {
    Fresco* f = fresco_instance();

    FigureKit_var figures = f->figure_kit();
    WidgetKit_var widgets = f->widget_kit();

    Button_var button = widgets->push_button(
	figures->label(
	    _tmp(figures->default_style()), Fresco::tmp_string_ref(label)
        ),
	new ActionCallback(Command)(c, &Command::execute)
    );
    append_viewer(button);
    return Glyph::_duplicate((GlyphRef)button);
}
	
Button_return FDraw::tool_button(
    GlyphRef g, Tool* t, TelltaleRef  group
) {
    Fresco* f = fresco_instance();
    WidgetKit_var widgets = f->widget_kit();

    Button_return b = widgets->palette_button(g, nil);
    Telltale_var bs = b->state();
    bs->current(group);
    b->click_action(_tmp(new ToolSetter(viewer_, t, bs)));
    append_viewer(b);
    return b;
}

void FDraw::flip_to(long card) {
    Long c = 0;
    GlyphOffset_var g = deck_->last_child_offset();
    for (; is_not_nil(g); g = g->prev_child(), c++) {
	if (c == card) {
	    deck_->append(g->child());
	    g->remove();
	    break;
	}
    }
}
