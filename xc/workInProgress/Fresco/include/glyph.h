/* DO NOT EDIT -- Automatically generated from Interfaces/glyph.idl */

#ifndef Interfaces_glyph_h
#define Interfaces_glyph_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>

class DamageObjType;
typedef DamageObjType* DamageObjRef;
class DamageObj;
class _DamageObjExpr;
class _DamageObjElem;

class DamageObj {
public:
    DamageObjRef _obj;

    DamageObj() { _obj = 0; }
    DamageObj(DamageObjRef p) { _obj = p; }
    DamageObj& operator =(DamageObjRef p);
    DamageObj(const DamageObj&);
    DamageObj& operator =(const DamageObj& r);
    DamageObj(const _DamageObjExpr&);
    DamageObj& operator =(const _DamageObjExpr&);
    DamageObj(const _DamageObjElem&);
    DamageObj& operator =(const _DamageObjElem&);
    ~DamageObj();

    operator DamageObjRef() const { return _obj; }
    DamageObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static DamageObjRef _narrow(BaseObjectRef p);
    static _DamageObjExpr _narrow(const BaseObject& r);

    static DamageObjRef _duplicate(DamageObjRef obj);
    static _DamageObjExpr _duplicate(const DamageObj& r);
};

class _DamageObjExpr : public DamageObj {
public:
    _DamageObjExpr(DamageObjRef p) { _obj = p; }
    _DamageObjExpr(const DamageObj& r) { _obj = r._obj; }
    _DamageObjExpr(const _DamageObjExpr& r) { _obj = r._obj; }
    ~_DamageObjExpr();
};

class _DamageObjElem {
public:
    DamageObjRef _obj;

    _DamageObjElem(DamageObjRef p) { _obj = p; }
    operator DamageObjRef() const { return _obj; }
    DamageObjRef operator ->() { return _obj; }
};

class DamageObjType : public FrescoObjectType {
protected:
    DamageObjType();
    virtual ~DamageObjType();
public:
    virtual void incur();
    virtual void extend(RegionRef r);
    _RegionExpr current() {
        return _c_current();
    }
    virtual RegionRef _c_current();

    _DamageObjExpr _ref();
    virtual TypeObjId _tid();
};

class DamageObjStub : public DamageObjType {
public:
    DamageObjStub(Exchange*);
    ~DamageObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline DamageObjRef DamageObj::_duplicate(DamageObjRef obj) {
    return (DamageObjRef)_BaseObject__duplicate(obj, &DamageObjStub::_create);
}
inline DamageObj& DamageObj::operator =(DamageObjRef p) {
    _BaseObject__release(_obj);
    _obj = DamageObj::_duplicate(p);
    return *this;
}
inline DamageObj::DamageObj(const DamageObj& r) {
    _obj = DamageObj::_duplicate(r._obj);
}
inline DamageObj& DamageObj::operator =(const DamageObj& r) {
    _BaseObject__release(_obj);
    _obj = DamageObj::_duplicate(r._obj);
    return *this;
}
inline DamageObj::DamageObj(const _DamageObjExpr& r) {
    _obj = r._obj;
    ((_DamageObjExpr*)&r)->_obj = 0;
}
inline DamageObj& DamageObj::operator =(const _DamageObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_DamageObjExpr*)&r)->_obj = 0;
    return *this;
}
inline DamageObj::DamageObj(const _DamageObjElem& e) {
    _obj = DamageObj::_duplicate(e._obj);
}
inline DamageObj& DamageObj::operator =(const _DamageObjElem& e) {
    _BaseObject__release(_obj);
    _obj = DamageObj::_duplicate(e._obj);
    return *this;
}
inline DamageObj::~DamageObj() {
    _BaseObject__release(_obj);
}
inline _DamageObjExpr DamageObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _DamageObjExpr DamageObj::_duplicate(const DamageObj& r) {
    return _duplicate(r._obj);
}
inline DamageObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _DamageObjExpr::~_DamageObjExpr() { }
inline _DamageObjExpr DamageObjType::_ref() { return this; }

class GlyphOffsetType;
typedef GlyphOffsetType* GlyphOffsetRef;
class GlyphOffset;
class _GlyphOffsetExpr;
class _GlyphOffsetElem;

class GlyphTraversalType;
typedef GlyphTraversalType* GlyphTraversalRef;
class GlyphTraversal;
class _GlyphTraversalExpr;
class _GlyphTraversalElem;

class GlyphVisitorType;
typedef GlyphVisitorType* GlyphVisitorRef;
class GlyphVisitor;
class _GlyphVisitorExpr;
class _GlyphVisitorElem;

class GlyphType;
typedef GlyphType* GlyphRef;
class Glyph;
class _GlyphExpr;
class _GlyphElem;

class Glyph {
public:
    GlyphRef _obj;

    Glyph() { _obj = 0; }
    Glyph(GlyphRef p) { _obj = p; }
    Glyph& operator =(GlyphRef p);
    Glyph(const Glyph&);
    Glyph& operator =(const Glyph& r);
    Glyph(const _GlyphExpr&);
    Glyph& operator =(const _GlyphExpr&);
    Glyph(const _GlyphElem&);
    Glyph& operator =(const _GlyphElem&);
    ~Glyph();

    operator GlyphRef() const { return _obj; }
    GlyphRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static GlyphRef _narrow(BaseObjectRef p);
    static _GlyphExpr _narrow(const BaseObject& r);

    static GlyphRef _duplicate(GlyphRef obj);
    static _GlyphExpr _duplicate(const Glyph& r);
    struct Requirement {
        Boolean defined;
        Coord natural, maximum, minimum;
        Alignment align;
    };
    struct Requisition {
        Requirement x, y, z;
        Boolean preserve_aspect;
    };
    struct AllocationInfo {
        RegionRef allocation;
        TransformObjRef transform;
        DamageObjRef damage;
    };
    class AllocationInfoList {
    public:
        long _maximum, _length; AllocationInfo* _buffer;

        AllocationInfoList() { _maximum = _length = 0; _buffer = 0; }
        AllocationInfoList(long m, long n, AllocationInfo* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        AllocationInfoList(const AllocationInfoList& _s) { _buffer = 0; *this = _s; }
        AllocationInfoList& operator =(const AllocationInfoList&);
        ~AllocationInfoList() { delete [] _buffer; }
    };
};

class _GlyphExpr : public Glyph {
public:
    _GlyphExpr(GlyphRef p) { _obj = p; }
    _GlyphExpr(const Glyph& r) { _obj = r._obj; }
    _GlyphExpr(const _GlyphExpr& r) { _obj = r._obj; }
    ~_GlyphExpr();
};

class _GlyphElem {
public:
    GlyphRef _obj;

    _GlyphElem(GlyphRef p) { _obj = p; }
    operator GlyphRef() const { return _obj; }
    GlyphRef operator ->() { return _obj; }
};

class GlyphType : public FrescoObjectType {
protected:
    GlyphType();
    virtual ~GlyphType();
public:
    _GlyphExpr clone_glyph() {
        return _c_clone_glyph();
    }
    virtual GlyphRef _c_clone_glyph();
    _StyleObjExpr style() {
        return _c_style();
    }
    virtual StyleObjRef _c_style();
    void style(StyleObjRef _p) {
        _c_style(_p);
    }
    virtual void _c_style(StyleObjRef _p);
    _TransformObjExpr transform() {
        return _c_transform();
    }
    virtual TransformObjRef _c_transform();
    virtual void request(Glyph::Requisition& r);
    virtual void extension(const Glyph::AllocationInfo& a, RegionRef r);
    virtual void shape(RegionRef r);
    virtual void traverse(GlyphTraversalRef t);
    virtual void draw(GlyphTraversalRef t);
    virtual void pick(GlyphTraversalRef t);
    _GlyphExpr body() {
        return _c_body();
    }
    virtual GlyphRef _c_body();
    void body(GlyphRef _p) {
        _c_body(_p);
    }
    virtual void _c_body(GlyphRef _p);
    _GlyphOffsetExpr append(GlyphRef g);
    virtual GlyphOffsetRef _c_append(GlyphRef g);
    _GlyphOffsetExpr prepend(GlyphRef g);
    virtual GlyphOffsetRef _c_prepend(GlyphRef g);
    virtual Tag add_parent(GlyphOffsetRef parent_offset);
    virtual void remove_parent(Tag add_tag);
    virtual void visit_children(GlyphVisitorRef v);
    virtual void visit_children_reversed(GlyphVisitorRef v);
    virtual void visit_parents(GlyphVisitorRef v);
    virtual void allocations(Glyph::AllocationInfoList& a);
    virtual void need_redraw();
    virtual void need_redraw_region(RegionRef r);
    virtual void need_resize();

    _GlyphExpr _ref();
    virtual TypeObjId _tid();
};

class GlyphStub : public GlyphType {
public:
    GlyphStub(Exchange*);
    ~GlyphStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline GlyphRef Glyph::_duplicate(GlyphRef obj) {
    return (GlyphRef)_BaseObject__duplicate(obj, &GlyphStub::_create);
}
inline Glyph& Glyph::operator =(GlyphRef p) {
    _BaseObject__release(_obj);
    _obj = Glyph::_duplicate(p);
    return *this;
}
inline Glyph::Glyph(const Glyph& r) {
    _obj = Glyph::_duplicate(r._obj);
}
inline Glyph& Glyph::operator =(const Glyph& r) {
    _BaseObject__release(_obj);
    _obj = Glyph::_duplicate(r._obj);
    return *this;
}
inline Glyph::Glyph(const _GlyphExpr& r) {
    _obj = r._obj;
    ((_GlyphExpr*)&r)->_obj = 0;
}
inline Glyph& Glyph::operator =(const _GlyphExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_GlyphExpr*)&r)->_obj = 0;
    return *this;
}
inline Glyph::Glyph(const _GlyphElem& e) {
    _obj = Glyph::_duplicate(e._obj);
}
inline Glyph& Glyph::operator =(const _GlyphElem& e) {
    _BaseObject__release(_obj);
    _obj = Glyph::_duplicate(e._obj);
    return *this;
}
inline Glyph::~Glyph() {
    _BaseObject__release(_obj);
}
inline _GlyphExpr Glyph::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _GlyphExpr Glyph::_duplicate(const Glyph& r) {
    return _duplicate(r._obj);
}
inline Glyph::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _GlyphExpr::~_GlyphExpr() { }
inline _GlyphExpr GlyphType::_ref() { return this; }

class GlyphVisitor {
public:
    GlyphVisitorRef _obj;

    GlyphVisitor() { _obj = 0; }
    GlyphVisitor(GlyphVisitorRef p) { _obj = p; }
    GlyphVisitor& operator =(GlyphVisitorRef p);
    GlyphVisitor(const GlyphVisitor&);
    GlyphVisitor& operator =(const GlyphVisitor& r);
    GlyphVisitor(const _GlyphVisitorExpr&);
    GlyphVisitor& operator =(const _GlyphVisitorExpr&);
    GlyphVisitor(const _GlyphVisitorElem&);
    GlyphVisitor& operator =(const _GlyphVisitorElem&);
    ~GlyphVisitor();

    operator GlyphVisitorRef() const { return _obj; }
    GlyphVisitorRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static GlyphVisitorRef _narrow(BaseObjectRef p);
    static _GlyphVisitorExpr _narrow(const BaseObject& r);

    static GlyphVisitorRef _duplicate(GlyphVisitorRef obj);
    static _GlyphVisitorExpr _duplicate(const GlyphVisitor& r);
};

class _GlyphVisitorExpr : public GlyphVisitor {
public:
    _GlyphVisitorExpr(GlyphVisitorRef p) { _obj = p; }
    _GlyphVisitorExpr(const GlyphVisitor& r) { _obj = r._obj; }
    _GlyphVisitorExpr(const _GlyphVisitorExpr& r) { _obj = r._obj; }
    ~_GlyphVisitorExpr();
};

class _GlyphVisitorElem {
public:
    GlyphVisitorRef _obj;

    _GlyphVisitorElem(GlyphVisitorRef p) { _obj = p; }
    operator GlyphVisitorRef() const { return _obj; }
    GlyphVisitorRef operator ->() { return _obj; }
};

class GlyphVisitorType : public FrescoObjectType {
protected:
    GlyphVisitorType();
    virtual ~GlyphVisitorType();
public:
    virtual Boolean visit(GlyphRef glyph, GlyphOffsetRef offset);

    _GlyphVisitorExpr _ref();
    virtual TypeObjId _tid();
};

class GlyphVisitorStub : public GlyphVisitorType {
public:
    GlyphVisitorStub(Exchange*);
    ~GlyphVisitorStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline GlyphVisitorRef GlyphVisitor::_duplicate(GlyphVisitorRef obj) {
    return (GlyphVisitorRef)_BaseObject__duplicate(obj, &GlyphVisitorStub::_create);
}
inline GlyphVisitor& GlyphVisitor::operator =(GlyphVisitorRef p) {
    _BaseObject__release(_obj);
    _obj = GlyphVisitor::_duplicate(p);
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const GlyphVisitor& r) {
    _obj = GlyphVisitor::_duplicate(r._obj);
}
inline GlyphVisitor& GlyphVisitor::operator =(const GlyphVisitor& r) {
    _BaseObject__release(_obj);
    _obj = GlyphVisitor::_duplicate(r._obj);
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const _GlyphVisitorExpr& r) {
    _obj = r._obj;
    ((_GlyphVisitorExpr*)&r)->_obj = 0;
}
inline GlyphVisitor& GlyphVisitor::operator =(const _GlyphVisitorExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_GlyphVisitorExpr*)&r)->_obj = 0;
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const _GlyphVisitorElem& e) {
    _obj = GlyphVisitor::_duplicate(e._obj);
}
inline GlyphVisitor& GlyphVisitor::operator =(const _GlyphVisitorElem& e) {
    _BaseObject__release(_obj);
    _obj = GlyphVisitor::_duplicate(e._obj);
    return *this;
}
inline GlyphVisitor::~GlyphVisitor() {
    _BaseObject__release(_obj);
}
inline _GlyphVisitorExpr GlyphVisitor::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _GlyphVisitorExpr GlyphVisitor::_duplicate(const GlyphVisitor& r) {
    return _duplicate(r._obj);
}
inline GlyphVisitor::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _GlyphVisitorExpr::~_GlyphVisitorExpr() { }
inline _GlyphVisitorExpr GlyphVisitorType::_ref() { return this; }

class GlyphOffset {
public:
    GlyphOffsetRef _obj;

    GlyphOffset() { _obj = 0; }
    GlyphOffset(GlyphOffsetRef p) { _obj = p; }
    GlyphOffset& operator =(GlyphOffsetRef p);
    GlyphOffset(const GlyphOffset&);
    GlyphOffset& operator =(const GlyphOffset& r);
    GlyphOffset(const _GlyphOffsetExpr&);
    GlyphOffset& operator =(const _GlyphOffsetExpr&);
    GlyphOffset(const _GlyphOffsetElem&);
    GlyphOffset& operator =(const _GlyphOffsetElem&);
    ~GlyphOffset();

    operator GlyphOffsetRef() const { return _obj; }
    GlyphOffsetRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static GlyphOffsetRef _narrow(BaseObjectRef p);
    static _GlyphOffsetExpr _narrow(const BaseObject& r);

    static GlyphOffsetRef _duplicate(GlyphOffsetRef obj);
    static _GlyphOffsetExpr _duplicate(const GlyphOffset& r);
};

class _GlyphOffsetExpr : public GlyphOffset {
public:
    _GlyphOffsetExpr(GlyphOffsetRef p) { _obj = p; }
    _GlyphOffsetExpr(const GlyphOffset& r) { _obj = r._obj; }
    _GlyphOffsetExpr(const _GlyphOffsetExpr& r) { _obj = r._obj; }
    ~_GlyphOffsetExpr();
};

class _GlyphOffsetElem {
public:
    GlyphOffsetRef _obj;

    _GlyphOffsetElem(GlyphOffsetRef p) { _obj = p; }
    operator GlyphOffsetRef() const { return _obj; }
    GlyphOffsetRef operator ->() { return _obj; }
};

class GlyphOffsetType : public FrescoObjectType {
protected:
    GlyphOffsetType();
    virtual ~GlyphOffsetType();
public:
    _GlyphExpr parent() {
        return _c_parent();
    }
    virtual GlyphRef _c_parent();
    _GlyphExpr child() {
        return _c_child();
    }
    virtual GlyphRef _c_child();
    virtual void allocations(Glyph::AllocationInfoList& a);
    _GlyphOffsetExpr insert(GlyphRef g) {
        return _c_insert(g);
    }
    virtual GlyphOffsetRef _c_insert(GlyphRef g);
    virtual void replace(GlyphRef g);
    virtual void remove();
    virtual void notify();

    _GlyphOffsetExpr _ref();
    virtual TypeObjId _tid();
};

class GlyphOffsetStub : public GlyphOffsetType {
public:
    GlyphOffsetStub(Exchange*);
    ~GlyphOffsetStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline GlyphOffsetRef GlyphOffset::_duplicate(GlyphOffsetRef obj) {
    return (GlyphOffsetRef)_BaseObject__duplicate(obj, &GlyphOffsetStub::_create);
}
inline GlyphOffset& GlyphOffset::operator =(GlyphOffsetRef p) {
    _BaseObject__release(_obj);
    _obj = GlyphOffset::_duplicate(p);
    return *this;
}
inline GlyphOffset::GlyphOffset(const GlyphOffset& r) {
    _obj = GlyphOffset::_duplicate(r._obj);
}
inline GlyphOffset& GlyphOffset::operator =(const GlyphOffset& r) {
    _BaseObject__release(_obj);
    _obj = GlyphOffset::_duplicate(r._obj);
    return *this;
}
inline GlyphOffset::GlyphOffset(const _GlyphOffsetExpr& r) {
    _obj = r._obj;
    ((_GlyphOffsetExpr*)&r)->_obj = 0;
}
inline GlyphOffset& GlyphOffset::operator =(const _GlyphOffsetExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_GlyphOffsetExpr*)&r)->_obj = 0;
    return *this;
}
inline GlyphOffset::GlyphOffset(const _GlyphOffsetElem& e) {
    _obj = GlyphOffset::_duplicate(e._obj);
}
inline GlyphOffset& GlyphOffset::operator =(const _GlyphOffsetElem& e) {
    _BaseObject__release(_obj);
    _obj = GlyphOffset::_duplicate(e._obj);
    return *this;
}
inline GlyphOffset::~GlyphOffset() {
    _BaseObject__release(_obj);
}
inline _GlyphOffsetExpr GlyphOffset::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _GlyphOffsetExpr GlyphOffset::_duplicate(const GlyphOffset& r) {
    return _duplicate(r._obj);
}
inline GlyphOffset::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _GlyphOffsetExpr::~_GlyphOffsetExpr() { }
inline _GlyphOffsetExpr GlyphOffsetType::_ref() { return this; }

class ViewerType;
typedef ViewerType* ViewerRef;
class Viewer;
class _ViewerExpr;
class _ViewerElem;

class GlyphTraversal {
public:
    GlyphTraversalRef _obj;

    GlyphTraversal() { _obj = 0; }
    GlyphTraversal(GlyphTraversalRef p) { _obj = p; }
    GlyphTraversal& operator =(GlyphTraversalRef p);
    GlyphTraversal(const GlyphTraversal&);
    GlyphTraversal& operator =(const GlyphTraversal& r);
    GlyphTraversal(const _GlyphTraversalExpr&);
    GlyphTraversal& operator =(const _GlyphTraversalExpr&);
    GlyphTraversal(const _GlyphTraversalElem&);
    GlyphTraversal& operator =(const _GlyphTraversalElem&);
    ~GlyphTraversal();

    operator GlyphTraversalRef() const { return _obj; }
    GlyphTraversalRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static GlyphTraversalRef _narrow(BaseObjectRef p);
    static _GlyphTraversalExpr _narrow(const BaseObject& r);

    static GlyphTraversalRef _duplicate(GlyphTraversalRef obj);
    static _GlyphTraversalExpr _duplicate(const GlyphTraversal& r);
    enum Operation {
        draw, pick_top, pick_any, pick_all, other
    };
};

class _GlyphTraversalExpr : public GlyphTraversal {
public:
    _GlyphTraversalExpr(GlyphTraversalRef p) { _obj = p; }
    _GlyphTraversalExpr(const GlyphTraversal& r) { _obj = r._obj; }
    _GlyphTraversalExpr(const _GlyphTraversalExpr& r) { _obj = r._obj; }
    ~_GlyphTraversalExpr();
};

class _GlyphTraversalElem {
public:
    GlyphTraversalRef _obj;

    _GlyphTraversalElem(GlyphTraversalRef p) { _obj = p; }
    operator GlyphTraversalRef() const { return _obj; }
    GlyphTraversalRef operator ->() { return _obj; }
};

class GlyphTraversalType : public FrescoObjectType {
protected:
    GlyphTraversalType();
    virtual ~GlyphTraversalType();
public:
    virtual GlyphTraversal::Operation op();
    virtual GlyphTraversal::Operation swap_op(GlyphTraversal::Operation op);
    virtual void begin_trail(ViewerRef v);
    virtual void end_trail();
    virtual void traverse_child(GlyphOffsetRef o, RegionRef allocation);
    virtual void visit();
    _GlyphTraversalExpr trail() {
        return _c_trail();
    }
    virtual GlyphTraversalRef _c_trail();
    _ViewerExpr current_viewer();
    virtual ViewerRef _c_current_viewer();
    _GlyphExpr current_glyph() {
        return _c_current_glyph();
    }
    virtual GlyphRef _c_current_glyph();
    _GlyphOffsetExpr offset() {
        return _c_offset();
    }
    virtual GlyphOffsetRef _c_offset();
    virtual void forward();
    virtual void backward();
    _PainterObjExpr painter() {
        return _c_painter();
    }
    virtual PainterObjRef _c_painter();
    void painter(PainterObjRef _p) {
        _c_painter(_p);
    }
    virtual void _c_painter(PainterObjRef _p);
    _DisplayObjExpr display() {
        return _c_display();
    }
    virtual DisplayObjRef _c_display();
    _ScreenObjExpr screen() {
        return _c_screen();
    }
    virtual ScreenObjRef _c_screen();
    _RegionExpr allocation() {
        return _c_allocation();
    }
    virtual RegionRef _c_allocation();
    virtual Boolean bounds(Vertex& lower, Vertex& upper, Vertex& origin);
    virtual Boolean origin(Vertex& origin);
    virtual Boolean span(Axis a, Region::BoundingSpan& s);
    _DamageObjExpr damage() {
        return _c_damage();
    }
    virtual DamageObjRef _c_damage();
    virtual void hit();
    virtual Long hit_info();
    virtual void hit_info(Long _p);
    _GlyphTraversalExpr picked() {
        return _c_picked();
    }
    virtual GlyphTraversalRef _c_picked();
    virtual void clear();

    _GlyphTraversalExpr _ref();
    virtual TypeObjId _tid();
};

class GlyphTraversalStub : public GlyphTraversalType {
public:
    GlyphTraversalStub(Exchange*);
    ~GlyphTraversalStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline GlyphTraversalRef GlyphTraversal::_duplicate(GlyphTraversalRef obj) {
    return (GlyphTraversalRef)_BaseObject__duplicate(obj, &GlyphTraversalStub::_create);
}
inline GlyphTraversal& GlyphTraversal::operator =(GlyphTraversalRef p) {
    _BaseObject__release(_obj);
    _obj = GlyphTraversal::_duplicate(p);
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const GlyphTraversal& r) {
    _obj = GlyphTraversal::_duplicate(r._obj);
}
inline GlyphTraversal& GlyphTraversal::operator =(const GlyphTraversal& r) {
    _BaseObject__release(_obj);
    _obj = GlyphTraversal::_duplicate(r._obj);
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const _GlyphTraversalExpr& r) {
    _obj = r._obj;
    ((_GlyphTraversalExpr*)&r)->_obj = 0;
}
inline GlyphTraversal& GlyphTraversal::operator =(const _GlyphTraversalExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_GlyphTraversalExpr*)&r)->_obj = 0;
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const _GlyphTraversalElem& e) {
    _obj = GlyphTraversal::_duplicate(e._obj);
}
inline GlyphTraversal& GlyphTraversal::operator =(const _GlyphTraversalElem& e) {
    _BaseObject__release(_obj);
    _obj = GlyphTraversal::_duplicate(e._obj);
    return *this;
}
inline GlyphTraversal::~GlyphTraversal() {
    _BaseObject__release(_obj);
}
inline _GlyphTraversalExpr GlyphTraversal::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _GlyphTraversalExpr GlyphTraversal::_duplicate(const GlyphTraversal& r) {
    return _duplicate(r._obj);
}
inline GlyphTraversal::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _GlyphTraversalExpr::~_GlyphTraversalExpr() { }
inline _GlyphTraversalExpr GlyphTraversalType::_ref() { return this; }

#endif
