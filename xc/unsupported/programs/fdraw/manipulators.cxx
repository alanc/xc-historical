/*
 * manipulators.cxx
 */

#include "commands.h"
#include "figviewer.h"
#include "globals.h"
#include "manipulators.h"
#include "selection.h"
#include "tools.h"
#include "visitors.h"
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/viewer.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/action.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/Xdrawing.h>
#include <X11/Fresco/OS/math.h>
#include <X11/Fresco/Ox/typeobjs.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

class PrintAction : public ActionImpl {
public:
    PrintAction(const char*);
    virtual ~PrintAction();

    virtual void execute(); 
private:
    char* string_;
};

PrintAction::PrintAction (const char* s) {
    string_ = strdup(s);
}

PrintAction::~PrintAction () {
    delete string_;
}

void PrintAction::execute () {
    printf("%s\n", string_);
}

static Coord dot = 4.0;
static double tol = 0.000005;

static ColorImpl* black = nil;

static void _translate (TransformObjRef tr, Vertex& v) {
    tr->translate(v);
}

static void _scale (TransformObjRef tr, Vertex& center, Vertex& s) {
    Vertex ncenter = center;
    ncenter.x = -ncenter.x;
    ncenter.y = -ncenter.y;
    ncenter.z = -ncenter.z;

    tr->translate(ncenter);
    tr->scale(s);
    tr->translate(center);
}

static void _rotate(TransformObjRef tr, double angle, Axis a, Vertex& center) {
    Vertex ncenter = center;
    ncenter.x = -ncenter.x;
    ncenter.y = -ncenter.y;
    ncenter.z = -ncenter.z;

    tr->translate(ncenter);
    tr->rotate(angle, a);
    tr->translate(center);
}

implementPtrList(ManipList, Manipulator);

ManipInfo::ManipInfo (SelectInfo* sinfo) {
    sinfo_ = sinfo;
    tooltype_ = undefined;
}

ManipInfo::~ManipInfo () {
    delete sinfo_;
}

class Inserter : public GlyphVisitorImpl {
public:
    Inserter(ManipList& m);
    Boolean visit(GlyphRef, GlyphOffsetRef);
private:
    ManipList* m_;
};

Inserter::Inserter (ManipList& m) {
    m_ = &m;
}

Boolean Inserter::visit (GlyphRef, GlyphOffsetRef go) {
    for (long i = 0; i < m_->count(); i++) {
        go->insert(m_->item(i));
    }
    return true;
}

//+ Transformer(GlyphImpl)
extern TypeObj_Descriptor _XfGlyphImpl_type;
TypeObj_Descriptor* _XfTransformer_parents[] = { &_XfGlyphImpl_type, nil };
extern TypeObjId _XfTransformer_tid;
TypeObj_Descriptor _XfTransformer_type = {
    /* type */ 0,
    /* id */ &_XfTransformer_tid,
    "Transformer",
    _XfTransformer_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

Transformer* Transformer::_narrow(BaseObjectRef o) {
    return (Transformer*)_BaseObject_tnarrow(
        o, _XfTransformer_tid, 0
    );
}
TypeObjId Transformer::_tid() { return _XfTransformer_tid; }
//+

Transformer::Transformer (TransformObjRef tx) {
    tx_.load(tx);
}
    
Transformer::Transformer () {}

Transformer::~Transformer () {}

//+ Transformer(Glyph::extension)
void Transformer::extension(const Glyph::AllocationInfo& a, Region_in r) {
    Glyph::AllocationInfo ga;
    ga.transform = a.transform;
    ga.allocation = a.allocation;
    ga.damage = a.damage;
    if (is_nil(ga.transform)) {
        ga.transform = new TransformImpl;
    } else {
        Fresco::ref(ga.transform);
    }
    ga.transform->premultiply(&tx_);
    MonoGlyph::extension(ga, r);
    Fresco::unref(ga.transform);
}

//+ Transformer(Glyph::request)
void Transformer::request(Glyph::Requisition& r) {
    Glyph::AllocationInfo a;
    a.transform = nil;
    a.allocation = nil;
    a.damage = nil;
    RegionImpl region;
    extension(a, &region);
    Coord x_lead = -region.lower_.x, x_trail = region.upper_.x;
    Coord y_lead = -region.lower_.y, y_trail = region.upper_.y;
    GlyphImpl::require_lead_trail(
        r.x, x_lead, x_lead, x_lead, x_trail, x_trail, x_trail
    );
    GlyphImpl::require_lead_trail(
        r.y, y_lead, y_lead, y_lead, y_trail, y_trail, y_trail
    );
}

//+ Transformer(Glyph::traverse)
void Transformer::traverse(GlyphTraversal_in t) {
    PainterObj p = t->painter();
    if (is_not_nil(p)) {
        if (tx_.identity()) {
            MonoGlyph::traverse(t);
        } else {
            p->push_matrix();
            p->transform(&tx_);
            MonoGlyph::traverse(t);
            p->pop_matrix();
        }
    }
}

//+ Transformer(Glyph::transform)
TransformObjRef Transformer::_c_transform() {
    Fresco::ref(&tx_);
    return &tx_;
}

void Transformer::child_allocate(Glyph::AllocationInfo& a) {
    a.transform->premultiply(&tx_);
}

//+ Manipulator(Transformer)
extern TypeObj_Descriptor _XfTransformer_type;
TypeObj_Descriptor* _XfManipulator_parents[] = { &_XfTransformer_type, nil };
extern TypeObjId _XfManipulator_tid;
TypeObj_Descriptor _XfManipulator_type = {
    /* type */ 0,
    /* id */ &_XfManipulator_tid,
    "Manipulator",
    _XfManipulator_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

Manipulator* Manipulator::_narrow(BaseObjectRef o) {
    return (Manipulator*)_BaseObject_tnarrow(
        o, _XfManipulator_tid, 0
    );
}
TypeObjId Manipulator::_tid() { return _XfManipulator_tid; }
//+

Manipulator::Manipulator (Manipulator* m) {
    selected_ = false;
    info_ = nil;
    if (is_not_nil(m)) {
        tx_.load(&m->tx_);
        selected_ = m->selected_;
        if (m->info_ != nil) {
            info_ = new ManipInfo(m->info_->sinfo_->copy());
        }
    }
}
    
Manipulator::~Manipulator () {
    delete info_;
}
    
void Manipulator::info (Manipulator* m, ManipInfo* i) {
    m->info_ = i;
}

ManipInfo* Manipulator::info (Manipulator* m) {
    return m->info_;
}

Boolean Manipulator::grasp (
    Tool& tool, FigViewer& fv, SelectInfo* si, EventRef e
) {
    Boolean ok = false;
    
    if (info_ == nil) {
        info_ = new ManipInfo(si);
    }
    if (is_not_nil(SelectTool::_narrow(&tool))) {
        info_->tooltype_ = choose;
    } else if (is_not_nil(CreateTool::_narrow(&tool))) {
        info_->tooltype_ = create;
    } else if (is_not_nil(MoveTool::_narrow(&tool))) {
        info_->tooltype_ = move;
    } else if (is_not_nil(ScaleTool::_narrow(&tool))) {
        info_->tooltype_ = scale;
    } else if (is_not_nil(RotateTool::_narrow(&tool))) {
        info_->tooltype_ = rotate;
    } else {
        info_->tooltype_ = undefined;
    }
    if (info_->tooltype_ != undefined) {
        ok = true;
        GlyphRef g = body();
        info_->obj_manip_ = e->modifier_is_down(Event::control);
        tool.anchor(info_->tool_ax_, info_->tool_ay_);
        info_->t_.load(&si->t_);
        if (info_->obj_manip_) {
            SelectCmd s;
            CmdVisitor cv(&s);
            g->visit_parents(&cv);
            info_->t_.premultiply(transform());
        } 
        Vertex lower, upper;
        Glyph::AllocationInfo a;
        RegionImpl r;

        if (info_->tooltype_ == create) {
            info_->ax_ = e->pointer_x();
            info_->ay_ = e->pointer_y();
            info_->pt_.x = e->pointer_x();
            info_->pt_.y = e->pointer_y();
            info_->pt_.z = 0;
        } else {
            a.allocation = new RegionImpl;
            a.transform = new TransformImpl;
            a.transform->load(&si->t_);
            a.damage = nil;

            extension(a, &r);
            r.bounds(lower, upper);
            info_->ax_ = (lower.x+upper.x)/2.0;
            info_->ay_ = (lower.y+upper.y)/2.0;
            info_->pt_.x = e->pointer_x() + info_->ax_ - info_->tool_ax_;
            info_->pt_.y = e->pointer_y() + info_->ay_ - info_->tool_ay_;
            info_->pt_.z = 0;

            Fresco::unref(a.allocation);
            Fresco::unref(a.transform);
            Fresco::unref(a.damage);
        }
        info_->t_.invert();
        info_->t_.transform(info_->pt_);

        a.transform = new TransformImpl;
        a.allocation = new RegionImpl;
        a.damage = nil;

        if (info_->obj_manip_) {
            body()->extension(a, &r);
        } else {
            extension(a, &r);
        }
        r.bounds(lower, upper);
        info_->center_.x = (lower.x+upper.x)/2.0;
        info_->center_.y = (lower.y+upper.y)/2.0;
        info_->center_.z = 0.0;

        Fresco::unref(a.transform);
        Fresco::unref(a.allocation);
        Fresco::unref(a.damage);

        if (info_->tooltype_ == create) {
            Vertex delta;
 
            delta.x = info_->pt_.x - (lower.x+upper.x)/2.0;
            delta.y = info_->pt_.y - (lower.y+upper.y)/2.0;
            delta.z = 0.0;
 
            if (info_->obj_manip_) {
                _translate(g->transform(), delta);
                g->need_redraw();
                g->need_resize();
            } else {
                _translate(&tx_, delta);
                need_redraw();
                need_resize();
            }
            info_->center_.x += delta.x;
            info_->center_.y += delta.y;
 
        } else if (info_->tooltype_ == choose) {
            SelectCmd selectcmd(&fv);
            execute(&selectcmd);

        } else if (info_->tooltype_ == scale) {
            Vertex scaler;
            scaler.x = Math::abs(
                (info_->pt_.x-info_->center_.x)*2.0/(upper.x-lower.x)
            );
            scaler.y = Math::abs(
                (info_->pt_.y-info_->center_.y)*2.0/(upper.y-lower.y)
            );
            scaler.z = 1.0;

            if (info_->obj_manip_) {
                _scale(g->transform(), info_->center_, scaler);
                g->need_redraw();
                g->need_resize();
            } else {
                _scale(&tx_, info_->center_, scaler);
                need_redraw();
                need_resize();
            }
        }
    }
    return ok;
}

Boolean Manipulator::manipulate (EventRef e) {
    if (e->type() != Event::motion) {
        return false;
    }

    Vertex pt;
    pt.x = e->pointer_x() + info_->ax_ - info_->tool_ax_;
    pt.y = e->pointer_y() + info_->ay_ - info_->tool_ay_;
    pt.z = 0;
    info_->t_.transform(pt);

    switch(info_->tooltype_) {
    case create:
    case move:
        {
            Vertex orig = pt;

            pt.x -= info_->pt_.x;
            pt.y -= info_->pt_.y;

            if (info_->obj_manip_) {
                GlyphRef g = body();
                _translate(g->transform(), pt);
                g->need_redraw();
                g->need_resize();
            } else {
                _translate(&tx_, pt);
                need_redraw();
                need_resize();
            }
            info_->center_.x += pt.x;
            info_->center_.y += pt.y;
            info_->pt_ = orig;
        }
        break;
    case rotate:
        {
            double pi = 3.14159;
            double den = pt.x-info_->center_.x;
            if (Math::abs(den) < tol) {
                return true;
            }
            double cur = atan((pt.y-info_->center_.y)/den)/pi*180.0;
            float last = atan(
                (info_->pt_.y-info_->center_.y)/
                (info_->pt_.x-info_->center_.x)
            )/pi*180.0;
            if ((pt.x-info_->center_.x) < 0.0) {
                cur += 180.0;
            }
            if ((info_->pt_.x-info_->center_.x) < 0.0) {
                last += 180.0;
            }
            if (info_->obj_manip_) { 
                GlyphRef g = body();
                _rotate(
                    g->transform(), cur-last, Z_axis, info_->center_
                );
                g->need_redraw();
                g->need_resize();
            } else {
                _rotate(&tx_, cur-last, Z_axis, info_->center_);
                need_redraw();
                need_resize();
            }
            info_->pt_ = pt;
        }
        break;
    case scale:
        {
            Vertex scaler;
            double den_x = info_->pt_.x-info_->center_.x;
            double den_y = info_->pt_.y-info_->center_.y;
            double num_x = pt.x-info_->center_.x;
            double num_y = pt.y-info_->center_.y;
            if (Math::abs(num_x) < tol || Math::abs(num_y) < tol) {
                return true;
            }
            scaler.x = num_x/den_x;
            scaler.y = num_y/den_y;
            scaler.z = 1.00;

            if (info_->obj_manip_) {
                GlyphRef g = body();
                _scale(g->transform(), info_->center_, scaler);
                g->need_redraw();
                g->need_resize();
            } else {
                _scale(&tx_, info_->center_, scaler);
                need_redraw();
                need_resize();
            }
            info_->pt_ = pt;
        }
        break;
    case choose:
        break;
    default:
        break;
    }
    return true;
}

Command* Manipulator::effect (EventRef) {
    if (info_->obj_manip_) {
        selected_ = false;
        GlyphRef g = body();
        SelectCmd s(nil, false);
        CmdVisitor cv(&s);
        g->visit_parents(&cv);
        selected_ = true;
    }
    return nil;  // commands for undo and redo not supported yet
}

void Manipulator::draw_handles (GlyphTraversalRef gt) {
    if (selected_) {
        PainterObj po = gt->painter();
        if (is_nil(black)) {
            black = new ColorImpl(0.0, 0.0, 0.0);
        }
        po->color_attr(black);

        RegionImpl bbox;
        Glyph::AllocationInfo a;
        po->push_matrix();

        a.transform = new TransformImpl;
        a.allocation = new RegionImpl;
        a.damage = nil;

        a.transform->load(po->matrix());
        po->matrix()->load_identity();
        GlyphRef child = body();
        child->extension(a, &bbox);

        Coord l, b, r, t, cx, cy;
        l = bbox.lower_.x; b = bbox.lower_.y;
        r = bbox.upper_.x; t = bbox.upper_.y;
        cx = l; cy = b;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = l; cy = (b+t)/2;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = l; cy = t;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = (l+r)/2; cy = t;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = r; cy = t;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = r; cy = (t+b)/2;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = r; cy = b;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        cx = (r+l)/2; cy = b;
        po->fill_rect(cx-dot/2, cy-dot/2, cx+dot/2, cy+dot/2); 

        po->pop_matrix();
        Fresco::unref(a.transform);
        Fresco::unref(a.allocation);
        Fresco::unref(a.damage);
    }
}

void Manipulator::execute (Command* cmd) {
    FigViewer* v = cmd->figviewer();
    SelectCmd* selectcmd = SelectCmd::_narrow(cmd);
    if (is_not_nil(selectcmd)) {
        Boolean s = selectcmd->selected();
        if (s != selected_) {
            selected_ = s;
            need_redraw();
            if (!selected_) {
                delete info_;
                info_ = nil;
            }
        }
        if (is_not_nil((GlyphRef)v)) {
            if (selected_) {
                v->selection()->add(this);
            } else {
                v->selection()->remove(this);
            }
        }
        return;
    }
    UngroupCmd* ungroupcmd = UngroupCmd::_narrow(cmd);
    if (is_not_nil(ungroupcmd)) {
        GlyphRef g = body();
        Counter counter;
        g->visit_children(&counter);
        if (counter.count() == 0) {
            return;
        }

        TransformImpl t;
        t.load(&tx_);
        t.premultiply(g->transform());
        ManipCopier mc;
        g->visit_children(&mc);

        ManipList* ml = mc.manipulators();
        for (long i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            TransformObj tr = man->transform();
            tr->postmultiply(&t);
        }
        Inserter inserter(*ml);
        visit_parents(&inserter);
        SelectCmd selectcmd(v);
        for (i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            man->execute(&selectcmd);
            man->info_ = new ManipInfo(info_->sinfo_->copy());
        }
        SelectCmd unselectcmd(v, false);
        execute(&unselectcmd);
        Remover remover;
        visit_parents(&remover);
        return;
    }
    DeleteCmd* deletecmd = DeleteCmd::_narrow(cmd);
    if (is_not_nil(deletecmd)) {
        Glyph g = body();
        if (info_ != nil && info_->obj_manip_) {
            delete info_;
            info_ = nil;
            CmdVisitor cv(deletecmd);
            g->visit_parents(&cv);
        } else {
            SelectCmd unselectcmd(v, false);
            execute(&unselectcmd);
            Remover remover;
            visit_parents(&remover);
        }
        return;
    }   
    InstanceCmd* instancecmd = InstanceCmd::_narrow(cmd);
    CopyCmd* copycmd = CopyCmd::_narrow(cmd);
    if (is_not_nil(instancecmd) || is_not_nil(copycmd)) {
        Manipulator* man;
        if (is_not_nil(instancecmd)) {
            man = shallow_copy();
        } else {
            CopyCmd::glyphmap_->clear();
            man = deep_copy();
        }
        TransformObjRef tr = man->transform();

        Vertex delta;
        delta.x = 4.0;
        delta.y = 4.0;
        delta.z = 0.0;

        tr->translate(delta);
        Appender app(man);
        visit_parents(&app);

        SelectCmd unselectcmd(v, false);
        execute(&unselectcmd);

        SelectCmd selectcmd(v);
        man->execute(&selectcmd);
        return;
    }
    NarrowCmd* narrowcmd = NarrowCmd::_narrow(cmd);
    if (is_not_nil(narrowcmd)) {
        /* not implemented */
        return;
    }
    SelectInfoCmd* scmd = SelectInfoCmd::_narrow(cmd);
    if (is_not_nil(scmd)) {
        scmd->select_info(info_->sinfo_);
        return;
    }
    GroupCmd* groupcmd = GroupCmd::_narrow(cmd);
    if (is_not_nil(groupcmd)) {
        Selection* sel = v->selection()->copy();
        long n = sel->count();
        if (n < 2) {
            delete sel;
            return;
        }
        PolyFigure* pf = new PolyFigure;
        Manipulator* pm = new PolyManip(pf);

        SelectCmd unselectcmd(v, false);
        SelectCmd selectcmd(v);
        pm->execute(&selectcmd);

        Manipulator* m = sel->item(0);
        pm->info_ = new ManipInfo(m->info_->sinfo_->copy());

        Remover remover;
        for (long i = 0; i < sel->count(); i++) {
            Manipulator* m = sel->item(i);
            m->execute(&unselectcmd);
            m->visit_parents(&remover);
            pf->append(m);
        }
        v->root()->body()->append(pm);
        return;
    }
}

void Manipulator::unexecute (Command*) {}

//+ Manipulator(Glyph::traverse)
void Manipulator::traverse(GlyphTraversal_in t) {
    PainterObj p = t->painter();
    if (is_not_nil(p)) {
        if (tx_.identity()) {
            MonoGlyph::traverse(t);
            if (t->op() == GlyphTraversal::draw) {
                draw_handles(t);
            }
        } else {
            p->push_matrix();
            p->transform(&tx_);
            MonoGlyph::traverse(t);
            if (t->op() == GlyphTraversal::draw) {
                draw_handles(t);
            }
            p->pop_matrix();
        }
    }
}

//+ FigureManip(Manipulator)
extern TypeObj_Descriptor _XfManipulator_type;
TypeObj_Descriptor* _XfFigureManip_parents[] = { &_XfManipulator_type, nil };
extern TypeObjId _XfFigureManip_tid;
TypeObj_Descriptor _XfFigureManip_type = {
    /* type */ 0,
    /* id */ &_XfFigureManip_tid,
    "FigureManip",
    _XfFigureManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

FigureManip* FigureManip::_narrow(BaseObjectRef o) {
    return (FigureManip*)_BaseObject_tnarrow(
        o, _XfFigureManip_tid, 0
    );
}
TypeObjId FigureManip::_tid() { return _XfFigureManip_tid; }
//+

FigureManip::FigureManip (FigureManip* m) : Manipulator (m) {
    figure_ = nil;
    if (is_not_nil(m) && is_not_nil(m->figure_)) {
        figure_body(m->figure_);
    }
}

void FigureManip::figure_body (Figure* f) {
    Fresco::unref(figure_);
    figure_ = f;
    Fresco::ref(figure_);
    if (is_not_nil(figure_)) {
        body(figure_);
    }
}

FigureManip::~FigureManip () {
    Fresco::unref(figure_);
}

Boolean FigureManip::grasp (
    Tool& tool, FigViewer& fv, SelectInfo* si, EventRef e
) {
    Boolean ok = Manipulator::grasp(tool, fv, si, e);
    if (info_->tooltype_ == create) {
        TransformObjRef tr = &info_->sinfo_->t_;
        info_->t_.load(tr);
        info_->t_.premultiply(transform());
        GlyphRef g = body();
        info_->t_.premultiply(g->transform());
        info_->t_.invert();

        info_->ax_ = e->pointer_x();
        info_->ay_ = e->pointer_y();
        info_->pt_.x = e->pointer_x();
        info_->pt_.y = e->pointer_y();
        info_->pt_.z = 0;
        info_->t_.transform(info_->pt_);
    }
    return ok;
}

//+ VertexManip(FigureManip)
extern TypeObj_Descriptor _XfFigureManip_type;
TypeObj_Descriptor* _XfVertexManip_parents[] = { &_XfFigureManip_type, nil };
extern TypeObjId _XfVertexManip_tid;
TypeObj_Descriptor _XfVertexManip_type = {
    /* type */ 0,
    /* id */ &_XfVertexManip_tid,
    "VertexManip",
    _XfVertexManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

VertexManip* VertexManip::_narrow(BaseObjectRef o) {
    return (VertexManip*)_BaseObject_tnarrow(
        o, _XfVertexManip_tid, 0
    );
}
TypeObjId VertexManip::_tid() { return _XfVertexManip_tid; }
//+

VertexManip::VertexManip (VertexManip* m) : FigureManip (m) {
    vlist_ = new VertexList;
    if (is_not_nil(m)) {
        for (long i = 0; i < m->vlist_->count(); i++) {
            vlist_->append(m->vlist_->item(i));
        }
    }
}

VertexManip::~VertexManip () { delete vlist_; }

Boolean VertexManip::grasp (
    Tool& tool, FigViewer& fv, SelectInfo* si, EventRef e
) {
    Boolean ok = FigureManip::grasp(tool, fv, si, e);
    if (info_->tooltype_ == create) {
        vlist_->remove_all();
        vlist_->append(info_->pt_);
        vlist_->append(info_->pt_);
    }
    return ok;
}

Boolean VertexManip::manipulate (EventRef e) {
    if (e->type() == Event::down && e->pointer_button() == 2) {
        return false;
    }
    Boolean ok = true;
    if (info_->tooltype_ == create) {
        Vertex pt;
        pt.x = e->pointer_x(); pt.y = e->pointer_y(); pt.z = 0;
        info_->t_.transform(pt);
        if (e->type() == Event::motion) {
            Vertex& last = vlist_->item_ref(vlist_->count()-1);
            last = pt;
        } else if (e->type() == Event::down) {
            if (e->pointer_button() == 1) {
                vlist_->append(pt);
            } else if (e->pointer_button() == 3) {
                if (vlist_->count() > 2) {
                    vlist_->remove(vlist_->count()-1);
                    Vertex& last = vlist_->item_ref(vlist_->count()-1);
                    last = pt;
                }
            }
        }
        recompute_shape();
        info_->pt_ = pt;
        need_redraw();
        need_resize();
    } else {
        ok = Manipulator::manipulate(e);
    }
    return ok;
}

void VertexManip::draw_handles (GlyphTraversalRef gt) {
    if (selected_) {
        PainterObj po = gt->painter();
        if (is_nil(black)) {
            black = new ColorImpl(0.0, 0.0, 0.0);
        }
        po->color_attr(black);
        TransformObj tx = figure_->transform();
        po->push_matrix();
        if (is_not_nil(tx) && !tx->identity()) {
            po->transform(tx);
        }
        TransformImpl ti;
        ti.load(po->matrix());
        po->matrix()->load_identity();

        for (long i = 0; i < vlist_->count(); i++) {
            Vertex vtex = vlist_->item(i);
            ti.transform(vtex);
            po->fill_rect(vtex.x-dot/2,vtex.y-dot/2,vtex.x+dot/2,vtex.y+dot/2);
        }
        po->pop_matrix();
    }
}

void VertexManip::recompute_shape () {
    figure_->reset();
    for (long i = 0; i < vlist_->count(); i++) {
        figure_->add_point(vlist_->item(i).x, vlist_->item(i).y);
    }
}

//+ LineManip(FigureManip)
extern TypeObj_Descriptor _XfFigureManip_type;
TypeObj_Descriptor* _XfLineManip_parents[] = { &_XfFigureManip_type, nil };
extern TypeObjId _XfLineManip_tid;
TypeObj_Descriptor _XfLineManip_type = {
    /* type */ 0,
    /* id */ &_XfLineManip_tid,
    "LineManip",
    _XfLineManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

LineManip* LineManip::_narrow(BaseObjectRef o) {
    return (LineManip*)_BaseObject_tnarrow(
        o, _XfLineManip_tid, 0
    );
}
TypeObjId LineManip::_tid() { return _XfLineManip_tid; }
//+

LineManip::LineManip (LineManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), false, false, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

LineManip::~LineManip () {}

Manipulator* LineManip::shallow_copy () {
    LineManip* manip = new LineManip(this);
    return manip;
}

Manipulator* LineManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        LineManip* manip =  new LineManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        LineManip* manip = new LineManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

Boolean LineManip::manipulate (EventRef e) {
    if (e->type() != Event::motion && e->type() != Event::up) {
        return false;
    }
    Boolean ok = true;
    if (info_->tooltype_ == create) {
        Vertex pt;
        pt.x = e->pointer_x(); pt.y = e->pointer_y(); pt.z = 0;
        info_->t_.transform(pt);
        Vertex& last = vlist_->item_ref(1);
        last = pt;
        if (e->type() == Event::up) {
            ok = false;
        }
        recompute_shape();
        info_->pt_ = pt;
        need_redraw();
        need_resize();
    } else {
        ok = Manipulator::manipulate(e);
    }
    return ok;
}

//+ RectManip(FigureManip)
extern TypeObj_Descriptor _XfFigureManip_type;
TypeObj_Descriptor* _XfRectManip_parents[] = { &_XfFigureManip_type, nil };
extern TypeObjId _XfRectManip_tid;
TypeObj_Descriptor _XfRectManip_type = {
    /* type */ 0,
    /* id */ &_XfRectManip_tid,
    "RectManip",
    _XfRectManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

RectManip* RectManip::_narrow(BaseObjectRef o) {
    return (RectManip*)_BaseObject_tnarrow(
        o, _XfRectManip_tid, 0
    );
}
TypeObjId RectManip::_tid() { return _XfRectManip_tid; }
//+

RectManip::RectManip (RectManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), true, false, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

RectManip::~RectManip () {}

Manipulator* RectManip::shallow_copy () {
    RectManip* manip = new RectManip(this);
    return manip;
}

Manipulator* RectManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        RectManip* manip = new RectManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        RectManip* manip = new RectManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

Boolean RectManip::manipulate (EventRef e) {
    if (e->type() != Event::motion && e->type() != Event::up) {
        return false;
    }
    Boolean ok = true;
    if (info_->tooltype_ == create) {
        Vertex pt, orig;
        pt.x = e->pointer_x(); pt.y = e->pointer_y(); pt.z = 0;
        info_->t_.transform(pt);
        orig = vlist_->item(0);
        vlist_->remove_all();
        if (e->modifier_is_down(Event::shift)) {
            if (Math::abs(pt.x-orig.x) > Math::abs(pt.y-orig.y)) {
                if (
                    (pt.y-orig.y) > 0 && (pt.x-orig.x) < 0 ||
                    (pt.y-orig.y) < 0 && (pt.x-orig.x) > 0
                ) {
                    pt.y = -pt.x;
                } else {
                    pt.y = pt.x;
                }
            } else {
                if (
                    (pt.x-orig.x) > 0 && (pt.y-orig.y) < 0 ||
                    (pt.x-orig.x) < 0 && (pt.y-orig.y) > 0
                ) {
                    pt.x = -pt.y;
                } else {
                    pt.x = pt.y;
                }
            }
        }
        Vertex v = orig;
        vlist_->append(v);
        v.y = pt.y;
        vlist_->append(v);
        v.x = pt.x;
        vlist_->append(v);
        v.y = orig.y;
        vlist_->append(v);
        if (e->type() == Event::up) {
            ok = false;
        }
        recompute_shape();
        info_->pt_ = pt;
        need_redraw();
        need_resize();
    } else {
        ok = Manipulator::manipulate(e);
    }
    return ok;
}

//+ EllipseManip(FigureManip)
extern TypeObj_Descriptor _XfFigureManip_type;
TypeObj_Descriptor* _XfEllipseManip_parents[] = { &_XfFigureManip_type, nil };
extern TypeObjId _XfEllipseManip_tid;
TypeObj_Descriptor _XfEllipseManip_type = {
    /* type */ 0,
    /* id */ &_XfEllipseManip_tid,
    "EllipseManip",
    _XfEllipseManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

EllipseManip* EllipseManip::_narrow(BaseObjectRef o) {
    return (EllipseManip*)_BaseObject_tnarrow(
        o, _XfEllipseManip_tid, 0
    );
}
TypeObjId EllipseManip::_tid() { return _XfEllipseManip_tid; }
//+

EllipseManip::EllipseManip (EllipseManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), true, true, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

EllipseManip::~EllipseManip () {}

Manipulator* EllipseManip::shallow_copy () {
    EllipseManip* manip = new EllipseManip(this);
    return manip;
}

Manipulator* EllipseManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        EllipseManip* manip = new EllipseManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        EllipseManip* manip = new EllipseManip(this);
        manip->figure_body(copy);
        return manip;
    }
}


static const float p0 = 1.00000000;
static const float p1 = 0.89657547;   // cos 30 * sqrt(1 + tan 15 * tan 15)
static const float p2 = 0.70710678;   // cos 45
static const float p3 = 0.51763809;   // cos 60 * sqrt(1 + tan 15 * tan 15)
static const float p4 = 0.26794919;   // tan 15

Boolean EllipseManip::manipulate (EventRef e) {
    if (e->type() != Event::motion && e->type() != Event::up) {
        return false;
    }
    Vertex pt;
    Boolean ok = true;
    if (info_->tooltype_ == create) {
        pt.x = e->pointer_x(); pt.y = e->pointer_y(); pt.z = 0;
        info_->t_.transform(pt);
        Vertex& last = vlist_->item_ref(1);
        Vertex& orig = vlist_->item_ref(0);
        if (e->type() == Event::up) {
            ok = false;
        }
        Coord r1 = Math::abs(pt.x - orig.x);
        Coord r2 = Math::abs(pt.y - orig.y);
        if (e->modifier_is_down(Event::shift)) {
            if (r1 > r2) {
                r2 = r1;
            } else {
                r1 = r2;
            }
            pt.x = r1 + orig.x;
            pt.y = r2 + orig.y;
        }
        last = pt;
        recompute_shape();
        info_->pt_ = pt;
        need_redraw();
        need_resize();
    } else {
        ok = Manipulator::manipulate(e);
    }
    return ok;
}

void EllipseManip::recompute_shape () {
    figure_->reset();

    Vertex& pt = vlist_->item_ref(1);
    Vertex& orig = vlist_->item_ref(0);

    Coord r1 = pt.x - orig.x;
    Coord r2 = pt.y - orig.y;

    float px0 = p0 * r1, py0 = p0 * r2;
    float px1 = p1 * r1, py1 = p1 * r2;
    float px2 = p2 * r1, py2 = p2 * r2;
    float px3 = p3 * r1, py3 = p3 * r2;
    float px4 = p4 * r1, py4 = p4 * r2;
    
    figure_->add_point(orig.x + r1, orig.y);
    figure_->add_curve(
        orig.x + px2, orig.y + py2, orig.x + px0, 
        orig.y + py4, orig.x + px1, orig.y + py3
    );
    figure_->add_curve(
        orig.x, orig.y + r2, orig.x + px3, 
        orig.y + py1, orig.x + px4, orig.y + py0
    );
    figure_->add_curve(
        orig.x - px2, orig.y + py2, orig.x - px4, 
        orig.y + py0, orig.x - px3, orig.y + py1
    );
    figure_->add_curve(
        orig.x - r1, orig.y, orig.x - px1, 
        orig.y + py3, orig.x - px0, orig.y + py4
    );
    figure_->add_curve(
        orig.x - px2, orig.y - py2, orig.x - px0, 
        orig.y - py4, orig.x - px1, orig.y - py3
    );
    figure_->add_curve(
        orig.x, orig.y - r2, 
        orig.x - px3, orig.y - py1, orig.x - px4, orig.y - py0
    );
    figure_->add_curve(
        orig.x + px2, orig.y - py2, orig.x + px4,
        orig.y - py0, orig.x + px3, orig.y - py1
    );
    figure_->add_curve(
        orig.x + r1, orig.y, orig.x + px1, orig.y - py3, 
        orig.x + px0, orig.y - py4
    );
}

void EllipseManip::draw_handles (GlyphTraversalRef gt) {
    Manipulator::draw_handles(gt);
}

//+ Open_BSplineManip(VertexManip)
extern TypeObj_Descriptor _XfVertexManip_type;
TypeObj_Descriptor* _XfOpen_BSplineManip_parents[] = { &_XfVertexManip_type, nil };
extern TypeObjId _XfOpen_BSplineManip_tid;
TypeObj_Descriptor _XfOpen_BSplineManip_type = {
    /* type */ 0,
    /* id */ &_XfOpen_BSplineManip_tid,
    "Open_BSplineManip",
    _XfOpen_BSplineManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

Open_BSplineManip* Open_BSplineManip::_narrow(BaseObjectRef o) {
    return (Open_BSplineManip*)_BaseObject_tnarrow(
        o, _XfOpen_BSplineManip_tid, 0
    );
}
TypeObjId Open_BSplineManip::_tid() { return _XfOpen_BSplineManip_tid; }
//+

Open_BSplineManip::Open_BSplineManip (Open_BSplineManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), false, true, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

Open_BSplineManip::~Open_BSplineManip () {}

Manipulator* Open_BSplineManip::shallow_copy () {
    Open_BSplineManip* manip = new Open_BSplineManip(this);
    return manip;
}

Manipulator* Open_BSplineManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        Open_BSplineManip* manip = new Open_BSplineManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        Open_BSplineManip* manip = new Open_BSplineManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

void Open_BSplineManip::recompute_shape () {
    long n = vlist_->count();
    figure_->reset();
    figure_->Bspline_move_to(
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(0).x, vlist_->item(0).y
    );
    figure_->Bspline_curve_to(
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(1).x, vlist_->item(1).y
    );
    for (long i = 1; i < n - 1; ++i) {
        figure_->Bspline_curve_to(
            vlist_->item(i).x, vlist_->item(i).y, 
            vlist_->item(i-1).x, vlist_->item(i-1).y, 
            vlist_->item(i+1).x, vlist_->item(i+1).y
        );
    }
    figure_->Bspline_curve_to(
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(n-2).x, vlist_->item(n-2).y, 
        vlist_->item(n-1).x, vlist_->item(n-1).y
    );
    figure_->Bspline_curve_to(
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(n-1).x, vlist_->item(n-1).y
    );
}

//+ Closed_BSplineManip(VertexManip)
extern TypeObj_Descriptor _XfVertexManip_type;
TypeObj_Descriptor* _XfClosed_BSplineManip_parents[] = { &_XfVertexManip_type, nil };
extern TypeObjId _XfClosed_BSplineManip_tid;
TypeObj_Descriptor _XfClosed_BSplineManip_type = {
    /* type */ 0,
    /* id */ &_XfClosed_BSplineManip_tid,
    "Closed_BSplineManip",
    _XfClosed_BSplineManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

Closed_BSplineManip* Closed_BSplineManip::_narrow(BaseObjectRef o) {
    return (Closed_BSplineManip*)_BaseObject_tnarrow(
        o, _XfClosed_BSplineManip_tid, 0
    );
}
TypeObjId Closed_BSplineManip::_tid() { return _XfClosed_BSplineManip_tid; }
//+

Closed_BSplineManip::Closed_BSplineManip (Closed_BSplineManip* m) : VertexManip (m){
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), true, true, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

Closed_BSplineManip::~Closed_BSplineManip () {}

Manipulator* Closed_BSplineManip::shallow_copy () {
    Closed_BSplineManip* manip = new Closed_BSplineManip(this);
    return manip;
}

Manipulator* Closed_BSplineManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        Closed_BSplineManip* manip = new Closed_BSplineManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        Closed_BSplineManip* manip = new Closed_BSplineManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

void Closed_BSplineManip::recompute_shape () {
    long n = vlist_->count();
    figure_->reset();
    figure_->Bspline_move_to(
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(1).x, vlist_->item(1).y
    );
    for (long i = 1; i < n - 1; ++i) {
        figure_->Bspline_curve_to(
            vlist_->item(i).x, vlist_->item(i).y, 
            vlist_->item(i-1).x, vlist_->item(i-1).y, 
            vlist_->item(i+1).x, vlist_->item(i+1).y
        );
    }
    figure_->Bspline_curve_to(
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(n-2).x, vlist_->item(n-2).y, 
        vlist_->item(0).x, vlist_->item(0).y
    );
    figure_->Bspline_curve_to(
        vlist_->item(0).x, vlist_->item(0).y, 
        vlist_->item(n-1).x, vlist_->item(n-1).y, 
        vlist_->item(1).x, vlist_->item(1).y
    );
}

//+ MultiLineManip(VertexManip)
extern TypeObj_Descriptor _XfVertexManip_type;
TypeObj_Descriptor* _XfMultiLineManip_parents[] = { &_XfVertexManip_type, nil };
extern TypeObjId _XfMultiLineManip_tid;
TypeObj_Descriptor _XfMultiLineManip_type = {
    /* type */ 0,
    /* id */ &_XfMultiLineManip_tid,
    "MultiLineManip",
    _XfMultiLineManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

MultiLineManip* MultiLineManip::_narrow(BaseObjectRef o) {
    return (MultiLineManip*)_BaseObject_tnarrow(
        o, _XfMultiLineManip_tid, 0
    );
}
TypeObjId MultiLineManip::_tid() { return _XfMultiLineManip_tid; }
//+

MultiLineManip::MultiLineManip (MultiLineManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), false, false, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

MultiLineManip::~MultiLineManip () {}

Manipulator* MultiLineManip::shallow_copy () {
    MultiLineManip* manip = new MultiLineManip(this);
    return manip;
}

Manipulator* MultiLineManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        MultiLineManip* manip = new MultiLineManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        MultiLineManip* manip = new MultiLineManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

//+ PolygonManip(VertexManip)
extern TypeObj_Descriptor _XfVertexManip_type;
TypeObj_Descriptor* _XfPolygonManip_parents[] = { &_XfVertexManip_type, nil };
extern TypeObjId _XfPolygonManip_tid;
TypeObj_Descriptor _XfPolygonManip_type = {
    /* type */ 0,
    /* id */ &_XfPolygonManip_tid,
    "PolygonManip",
    _XfPolygonManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

PolygonManip* PolygonManip::_narrow(BaseObjectRef o) {
    return (PolygonManip*)_BaseObject_tnarrow(
        o, _XfPolygonManip_tid, 0
    );
}
TypeObjId PolygonManip::_tid() { return _XfPolygonManip_tid; }
//+

PolygonManip::PolygonManip (PolygonManip* m) : VertexManip (m) {
    if (is_nil(m) || is_nil(m->figure_)) {
        FigureKit figurekit = fresco_instance()->figure_kit();
        Figure* f = new Figure(
            FigureKit::stroke, figurekit->default_style(), true, false, 1
        );
        f->add_point(0, 0);
        figure_body(f);
    }
}

PolygonManip::~PolygonManip () {}

Manipulator* PolygonManip::shallow_copy () {
    PolygonManip* manip = new PolygonManip(this);
    return manip;
}

Manipulator* PolygonManip::deep_copy () {
    Figure* copy = (Figure*)CopyCmd::glyphmap_->find(figure_);
    if (is_nil(copy)) {
        Figure* f = new Figure(figure_);
        CopyCmd::glyphmap_->map(figure_, f);
        PolygonManip* manip = new PolygonManip(this);
        manip->figure_body(f);
        Fresco::unref(f);
        return manip;
    } else {
        PolygonManip* manip = new PolygonManip(this);
        manip->figure_body(copy);
        return manip;
    }
}

//+ PolyManip(Manipulator)
extern TypeObj_Descriptor _XfManipulator_type;
TypeObj_Descriptor* _XfPolyManip_parents[] = { &_XfManipulator_type, nil };
extern TypeObjId _XfPolyManip_tid;
TypeObj_Descriptor _XfPolyManip_type = {
    /* type */ 0,
    /* id */ &_XfPolyManip_tid,
    "PolyManip",
    _XfPolyManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

PolyManip* PolyManip::_narrow(BaseObjectRef o) {
    return (PolyManip*)_BaseObject_tnarrow(
        o, _XfPolyManip_tid, 0
    );
}
TypeObjId PolyManip::_tid() { return _XfPolyManip_tid; }
//+

PolyManip::PolyManip (PolyFigure* pf, PolyManip* m) : Manipulator (m) {
    polyfigure_ = pf;
    Fresco::ref(polyfigure_);
    body(polyfigure_);
}

PolyManip::~PolyManip () {
    Fresco::unref(polyfigure_);
}

void PolyManip::execute (Command* cmd) {
    FigViewer* v = cmd->figviewer();
    NaturalCmd* natcmd = NaturalCmd::_narrow(cmd);
    if (is_not_nil(natcmd)) {
        CmdVisitor cv(natcmd);
        polyfigure_->visit_children(&cv);
    } else {
        Manipulator::execute(cmd);
    }
}

void PolyManip::unexecute (Command* cmd) {
    FigViewer* v = cmd->figviewer();
    NaturalCmd* natcmd = NaturalCmd::_narrow(cmd);
    if (is_not_nil(natcmd)) {
        CmdVisitor cv(natcmd, false);
        polyfigure_->visit_children(&cv);
    } else {
        Manipulator::unexecute(cmd);
    }
}

Manipulator* PolyManip::shallow_copy () {
    return new PolyManip(polyfigure_, this);
}

Manipulator* PolyManip::deep_copy () {
    PolyFigure* copy = (PolyFigure*)CopyCmd::glyphmap_->find(polyfigure_);
    if (is_nil(copy)) {
        PolyFigure* f = new PolyFigure(polyfigure_);
        CopyCmd::glyphmap_->map(polyfigure_, f);

        ManipCopier dcopier(false);
        GlyphRef g = body();
        g->visit_children(&dcopier);
        ManipList* ml = dcopier.manipulators();
        for (long i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            f->append(man);
            Fresco::unref(man);
        }
        Manipulator* m = new PolyManip(f, this);
        Fresco::unref(f);
        return m;
    } else {
        return new PolyManip(copy, this);
    }
}

//+ ButtonManip(Manipulator)
extern TypeObj_Descriptor _XfManipulator_type;
TypeObj_Descriptor* _XfButtonManip_parents[] = { &_XfManipulator_type, nil };
extern TypeObjId _XfButtonManip_tid;
TypeObj_Descriptor _XfButtonManip_type = {
    /* type */ 0,
    /* id */ &_XfButtonManip_tid,
    "ButtonManip",
    _XfButtonManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

ButtonManip* ButtonManip::_narrow(BaseObjectRef o) {
    return (ButtonManip*)_BaseObject_tnarrow(
        o, _XfButtonManip_tid, 0
    );
}
TypeObjId ButtonManip::_tid() { return _XfButtonManip_tid; }
//+

ButtonManip::ButtonManip (
    ViewerRef b, Boolean* e, Transformer* t, ButtonManip* m
) : Manipulator (m) {
    button_ = b;
    editing_ = e;
    transformer_ = t;

    if (is_nil(transformer_)) {
        Fresco* f = fresco_instance();
        WidgetKit widgets = f->widget_kit();
        LayoutKit layouts = f->layout_kit();
        Button br = widgets->palette_button(layouts->vspace(10), nil);
        button_->append_viewer(br);
        transformer_ = new Transformer;
        Allocator* allocator = new Allocator;
        allocator->body(br);
        transformer_->body(allocator);
        Fresco::unref(allocator);
    } else {
        Fresco::ref(transformer_);
    }

    Fresco::ref(button_);
    body(transformer_);
}

ButtonManip::~ButtonManip () {
    Fresco::unref(transformer_);
    Fresco::unref(button_);
}

Manipulator* ButtonManip::shallow_copy () {
    return new ButtonManip(button_, editing_, transformer_, this);
}

Manipulator* ButtonManip::deep_copy () {
    Transformer* copy = (Transformer*)CopyCmd::glyphmap_->find(transformer_);
    if (is_nil(copy)) {
        Fresco* f = fresco_instance();
        WidgetKit widgets = f->widget_kit();
        LayoutKit layouts = f->layout_kit();
        FigureKit figures = f->figure_kit();

        Button br = widgets->palette_button(
            figures->label(
                figures->default_style(), 
                Fresco::string_ref("Button")
            ), new PrintAction("Pushed")
        );
        button_->append_viewer(br);
        Transformer* transformer = new Transformer(transformer_->transform());
        CopyCmd::glyphmap_->map(transformer_, transformer);
        
        Allocator* allocator = new Allocator;
        allocator->body(br);
        transformer->body(allocator);
        Manipulator* m = new ButtonManip(br, editing_, transformer, this);
        Fresco::unref(allocator);
        Fresco::unref(transformer);
        return m;
    } else {
        return new ButtonManip(button_, editing_, copy, this);
    }
}

//+ ButtonManip(Glyph::traverse)
void ButtonManip::traverse(GlyphTraversal_in t) {
    switch(t->op()) {
    case GlyphTraversal::pick_top:
    case GlyphTraversal::pick_all:
    case GlyphTraversal::pick_any:
        if (*editing_) {
            RegionImpl r;
            Glyph::AllocationInfo a;
            a.allocation = new RegionImpl;
            a.transform = new TransformImpl;
            a.damage = nil;
            extension(a, &r);
            Fresco::unref(a.allocation);
            Fresco::unref(a.transform);
            Fresco::unref(a.damage);
            if (t->painter()->is_visible(&r)) {
                t->hit();
            }
        } else {
            Manipulator::traverse(t);
        }
        break;
    default:
        Manipulator::traverse(t);
        break;
    }
}

void ButtonManip::execute (Command* cmd) {
    UngroupCmd* ungroupcmd = UngroupCmd::_narrow(cmd);
    if (is_nil(ungroupcmd)) {
        Manipulator::execute(cmd);
    }
}

void ButtonManip::unexecute (Command* cmd) {
    UngroupCmd* ungroupcmd = UngroupCmd::_narrow(cmd);
    if (is_nil(ungroupcmd)) {
        Manipulator::unexecute(cmd);
    }
}

//+ BoxManip(Manipulator)
extern TypeObj_Descriptor _XfManipulator_type;
TypeObj_Descriptor* _XfBoxManip_parents[] = { &_XfManipulator_type, nil };
extern TypeObjId _XfBoxManip_tid;
TypeObj_Descriptor _XfBoxManip_type = {
    /* type */ 0,
    /* id */ &_XfBoxManip_tid,
    "BoxManip",
    _XfBoxManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

BoxManip* BoxManip::_narrow(BaseObjectRef o) {
    return (BoxManip*)_BaseObject_tnarrow(
        o, _XfBoxManip_tid, 0
    );
}
TypeObjId BoxManip::_tid() { return _XfBoxManip_tid; }
//+

BoxManip::BoxManip (Transformer* tr, BoxManip* m) : Manipulator(m) {
    transformer_ = tr;
    Fresco::ref(transformer_);
    if (is_not_nil(transformer_)) {
        body(transformer_);
    }
}

BoxManip::~BoxManip () {
    Fresco::unref(transformer_);
}

void BoxManip::execute (Command* cmd) {
    FigViewer* v = cmd->figviewer();
    UngroupCmd* ungroupcmd = UngroupCmd::_narrow(cmd);
    if (is_not_nil(ungroupcmd)) {
        Glyph tr = body();

        TransformImpl t;
        t.load(&tx_);
        t.premultiply(tr->transform());

        Glyph allocator = tr->body();

        RegionImpl r;
        Glyph::AllocationInfo a;
        a.transform = nil;
        a.allocation = nil;
        allocator->extension(a, &r);

        Glyph box = allocator->body();

        TAManipCopier mc(&r);
        box->visit_children(&mc);
        ManipList* ml = mc.manipulators();
        TransformList* tl = mc.transforms();

        for (long i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            TransformObj tr = man->transform();
            tr->postmultiply(tl->item(i));
            tr->postmultiply(&t);
        }
        Inserter inserter(*ml);
        visit_parents(&inserter);
        SelectCmd selectcmd(v);
        for (i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            man->execute(&selectcmd);
            info(man, new ManipInfo(info_->sinfo_->copy()));
        }
        SelectCmd unselectcmd(v, false);
        execute(&unselectcmd);
        Remover remover;
        visit_parents(&remover);
        return;
    }
    GroupCmd* groupcmd = GroupCmd::_narrow(cmd);
    if (is_not_nil(groupcmd)) {
        Selection* sel = v->selection()->copy();
        long n = sel->count();
        if (n < 2) {
            delete sel;
            return;
        }
        CopyCmd::glyphmap_->clear();
        Manipulator* bm = deep_copy();
        Glyph box = bm->body()->body()->body();

        SelectCmd unselectcmd(v, false);
        SelectCmd selectcmd(v);
        bm->execute(&selectcmd);

        Manipulator* m = sel->item(0);
        info(bm, new ManipInfo(info(m)->sinfo_->copy()));

        Remover remover;

        Glyph::AllocationInfo a;
        a.transform = nil;
        a.allocation = nil;
        a.damage = nil;
        RegionImpl r;
        Vertex center;
        center.x = 0; center.y = 0, center.z = 0;
        for (long i = 0; i < sel->count(); i++) {
            Manipulator* m = sel->item(i);
            m->extension(a, &r);
            center.x += (r.upper_.x+r.lower_.x)/2;
            center.y += (r.upper_.y+r.lower_.y)/2;
            
            m->execute(&unselectcmd);
            m->visit_parents(&remover);
            box->append(m);
        }
        center.x /= sel->count(); center.y /= sel->count();
        bm->extension(a, &r);
        center.x -= (r.upper_.x+r.lower_.x)/2;
        center.y -= (r.upper_.y+r.lower_.y)/2;
        bm->transform()->translate(center);
        v->root()->body()->append(bm);
        return;
    }
    NaturalCmd* natcmd = NaturalCmd::_narrow(cmd);
    if (is_not_nil(natcmd)) {
        Fresco* f = fresco_instance();
        LayoutKit layouts = f->layout_kit();
        CmdVisitor cv(natcmd);
        Glyph allocator = body()->body();
        Glyph box = allocator->body();
        allocator->body(layouts->vglue_fil(10));

        box->visit_children(&cv);
        box->need_resize();
        allocator->body(box);

        return;
    }
    Manipulator::execute(cmd);
}

void BoxManip::unexecute (Command*) {}

//+ HBoxManip(BoxManip)
extern TypeObj_Descriptor _XfBoxManip_type;
TypeObj_Descriptor* _XfHBoxManip_parents[] = { &_XfBoxManip_type, nil };
extern TypeObjId _XfHBoxManip_tid;
TypeObj_Descriptor _XfHBoxManip_type = {
    /* type */ 0,
    /* id */ &_XfHBoxManip_tid,
    "HBoxManip",
    _XfHBoxManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

HBoxManip* HBoxManip::_narrow(BaseObjectRef o) {
    return (HBoxManip*)_BaseObject_tnarrow(
        o, _XfHBoxManip_tid, 0
    );
}
TypeObjId HBoxManip::_tid() { return _XfHBoxManip_tid; }
//+

HBoxManip::HBoxManip (Transformer* t, HBoxManip* m) : BoxManip (t, m) {
    if (is_nil(t)) {
        Fresco* f = fresco_instance();
        LayoutKit layouts = f->layout_kit();
        Transformer* th = new Transformer;
        Allocator* ah = new Allocator;
        ah->body(layouts->hbox());
        th->body(ah);
        transformer_ = th;
        body(transformer_);
    }
}

HBoxManip::~HBoxManip () {}

Manipulator* HBoxManip::shallow_copy () {
    return new HBoxManip(transformer_, this);
}

Manipulator* HBoxManip::deep_copy () {
    Transformer* copy = (Transformer*)CopyCmd::glyphmap_->find(transformer_);
    if (is_nil(copy)) {
        Fresco* f = fresco_instance();
        LayoutKit layouts = f->layout_kit();
        Glyph box = layouts->hbox();

        Transformer* transformer = new Transformer(transformer_->transform());
        CopyCmd::glyphmap_->map(transformer_, transformer);
        
        Allocator* allocator = new Allocator;
        allocator->body(box);
        transformer->body(allocator);

        ManipCopier dcopier(false);
        GlyphRef g = body()->body()->body();
        g->visit_children(&dcopier);
        ManipList* ml = dcopier.manipulators();
        for (long i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            box->append(man);
            Fresco::unref(man);
        }

        Manipulator* m = new HBoxManip(transformer, this);
        Fresco::unref(allocator);
        Fresco::unref(transformer);
        return m;
    } else {
        return new HBoxManip(copy, this);
    }
}

//+ VBoxManip(BoxManip)
extern TypeObj_Descriptor _XfBoxManip_type;
TypeObj_Descriptor* _XfVBoxManip_parents[] = { &_XfBoxManip_type, nil };
extern TypeObjId _XfVBoxManip_tid;
TypeObj_Descriptor _XfVBoxManip_type = {
    /* type */ 0,
    /* id */ &_XfVBoxManip_tid,
    "VBoxManip",
    _XfVBoxManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

VBoxManip* VBoxManip::_narrow(BaseObjectRef o) {
    return (VBoxManip*)_BaseObject_tnarrow(
        o, _XfVBoxManip_tid, 0
    );
}
TypeObjId VBoxManip::_tid() { return _XfVBoxManip_tid; }
//+

VBoxManip::VBoxManip (Transformer* t, VBoxManip* m) : BoxManip (t, m) {
    if (is_nil(t)) {
        Fresco* f = fresco_instance();
        LayoutKit layouts = f->layout_kit();
        Transformer* tv = new Transformer;
        Allocator* av = new Allocator;
        av->body(layouts->vbox());
        tv->body(av);
        transformer_ = tv;
        body(transformer_);
    }
}

VBoxManip::~VBoxManip () {}

Manipulator* VBoxManip::shallow_copy () {
    return new VBoxManip(transformer_, this);
}

Manipulator* VBoxManip::deep_copy () {
    Transformer* copy = (Transformer*)CopyCmd::glyphmap_->find(transformer_);
    if (is_nil(copy)) {
        Fresco* f = fresco_instance();
        LayoutKit layouts = f->layout_kit();
        Glyph box = layouts->vbox();

        Transformer* transformer = new Transformer(transformer_->transform());
        CopyCmd::glyphmap_->map(transformer_, transformer);
        
        Allocator* allocator = new Allocator;
        allocator->body(box);
        transformer->body(allocator);

        ManipCopier dcopier(false);
        GlyphRef g = body()->body()->body();
        g->visit_children(&dcopier);
        ManipList* ml = dcopier.manipulators();
        for (long i = 0; i < ml->count(); i++) {
            Manipulator* man = ml->item(i);
            box->append(man);
            Fresco::unref(man);
        }

        Manipulator* m = new VBoxManip(transformer, this);
        Fresco::unref(allocator);
        Fresco::unref(transformer);
        return m;
    } else {
        return new VBoxManip(copy, this);
    }
}

//+ MacroManip(Manipulator)
extern TypeObj_Descriptor _XfManipulator_type;
TypeObj_Descriptor* _XfMacroManip_parents[] = { &_XfManipulator_type, nil };
extern TypeObjId _XfMacroManip_tid;
TypeObj_Descriptor _XfMacroManip_type = {
    /* type */ 0,
    /* id */ &_XfMacroManip_tid,
    "MacroManip",
    _XfMacroManip_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

MacroManip* MacroManip::_narrow(BaseObjectRef o) {
    return (MacroManip*)_BaseObject_tnarrow(
        o, _XfMacroManip_tid, 0
    );
}
TypeObjId MacroManip::_tid() { return _XfMacroManip_tid; }
//+

MacroManip::MacroManip (MacroManip* m) : Manipulator(m) {
    maniplist_ = new ManipList;
}

MacroManip::~MacroManip () {
    delete maniplist_;
}

Boolean MacroManip::grasp(
    Tool& t, FigViewer& fv, SelectInfo* si, EventRef e
) {
    Boolean ok = true;
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        ok = m->grasp(t, fv, si, e) && ok;
    }
    return ok;
}

Boolean MacroManip::manipulate(EventRef e) {
    Boolean ok = true;
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        ok = m->manipulate(e) && ok;
    }
    return ok;
}

Command* MacroManip::effect (EventRef e) {
    MacroCmd* mc = new MacroCmd;
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        Command* cmd = m->effect(e);
        if (is_not_nil(cmd)) {
            mc->append(cmd);
        }
    }
    if (mc->count() == 0) {
        delete mc;
        mc = nil;
    }
    return mc;
}

void MacroManip::execute (Command* cmd) {
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        m->execute(cmd);
    }
}

void MacroManip::unexecute (Command* cmd) {
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        m->unexecute(cmd);
    }
}

Manipulator* MacroManip::shallow_copy () {
    MacroManip* macro = new MacroManip(this);
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i);
        macro->append(m);
    }
    return macro;
}
    
Manipulator* MacroManip::deep_copy () {
    MacroManip* macro = new MacroManip(this);
    for (long i = 0; i < maniplist_->count(); i++) {
        Manipulator* m = maniplist_->item(i)->deep_copy();
        macro->append(m);
        Fresco::unref(m);
    }
    return macro;
}
    
void MacroManip::add (Manipulator* manip) {
    maniplist_->append(manip);
}

void MacroManip::insert (long index, Manipulator* manip) {
    maniplist_->insert(index, manip);
}

void MacroManip::remove(long index) {
    maniplist_->remove(index);
}

void MacroManip::remove_all() {
    maniplist_->remove_all();
}

long MacroManip::count () {
    return maniplist_->count();
}

Manipulator* MacroManip::manipulator (long index) {
    return maniplist_->item(index);
}

