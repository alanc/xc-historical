/* DO NOT EDIT -- Automatically generated from Interfaces/glyph.idl */

#ifndef Interfaces_glyph_h
#define Interfaces_glyph_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>

class DamageObjType;
typedef DamageObjType* DamageObjRef;
typedef DamageObjRef DamageObj_in;
class DamageObj;
class DamageObj_tmp;
class DamageObj_var;

class DamageObj {
public:
    DamageObjRef _obj_;

    DamageObj() { _obj_ = 0; }
    DamageObj(DamageObjRef p) { _obj_ = p; }
    DamageObj& operator =(DamageObjRef p);
    DamageObj(const DamageObj&);
    DamageObj& operator =(const DamageObj& r);
    DamageObj(const DamageObj_tmp&);
    DamageObj& operator =(const DamageObj_tmp&);
    DamageObj(const DamageObj_var&);
    DamageObj& operator =(const DamageObj_var&);
    ~DamageObj();

    DamageObjRef operator ->() { return _obj_; }

    operator DamageObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static DamageObjRef _narrow(BaseObjectRef p);
    static DamageObj_tmp _narrow(const BaseObject& r);

    static DamageObjRef _duplicate(DamageObjRef obj);
    static DamageObj_tmp _duplicate(const DamageObj& r);
};

class DamageObj_tmp : public DamageObj {
public:
    DamageObj_tmp(DamageObjRef p) { _obj_ = p; }
    DamageObj_tmp(const DamageObj& r);
    DamageObj_tmp(const DamageObj_tmp& r);
    ~DamageObj_tmp();
};

class DamageObj_var {
public:
    DamageObjRef _obj_;

    DamageObj_var(DamageObjRef p) { _obj_ = p; }
    operator DamageObjRef() const { return _obj_; }
    DamageObjRef operator ->() { return _obj_; }
};

class DamageObjType : public FrescoObjectType {
protected:
    DamageObjType();
    virtual ~DamageObjType();
public:
    virtual void incur();
    virtual void extend(Region_in r);
    Region_tmp current() {
        return _c_current();
    }
    virtual RegionRef _c_current();
    DamageObjRef _obj() { return this; }
    void* _this();
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

class GlyphOffsetType;
typedef GlyphOffsetType* GlyphOffsetRef;
typedef GlyphOffsetRef GlyphOffset_in;
class GlyphOffset;
class GlyphOffset_tmp;
class GlyphOffset_var;

class GlyphTraversalType;
typedef GlyphTraversalType* GlyphTraversalRef;
typedef GlyphTraversalRef GlyphTraversal_in;
class GlyphTraversal;
class GlyphTraversal_tmp;
class GlyphTraversal_var;

class GlyphVisitorType;
typedef GlyphVisitorType* GlyphVisitorRef;
typedef GlyphVisitorRef GlyphVisitor_in;
class GlyphVisitor;
class GlyphVisitor_tmp;
class GlyphVisitor_var;

class GlyphType;
typedef GlyphType* GlyphRef;
typedef GlyphRef Glyph_in;
class Glyph;
class Glyph_tmp;
class Glyph_var;

class Glyph {
public:
    GlyphRef _obj_;

    Glyph() { _obj_ = 0; }
    Glyph(GlyphRef p) { _obj_ = p; }
    Glyph& operator =(GlyphRef p);
    Glyph(const Glyph&);
    Glyph& operator =(const Glyph& r);
    Glyph(const Glyph_tmp&);
    Glyph& operator =(const Glyph_tmp&);
    Glyph(const Glyph_var&);
    Glyph& operator =(const Glyph_var&);
    ~Glyph();

    GlyphRef operator ->() { return _obj_; }

    operator Glyph_in() const { return _obj_; }
    operator FrescoObject() const;
    static GlyphRef _narrow(BaseObjectRef p);
    static Glyph_tmp _narrow(const BaseObject& r);

    static GlyphRef _duplicate(GlyphRef obj);
    static Glyph_tmp _duplicate(const Glyph& r);
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

class Glyph_tmp : public Glyph {
public:
    Glyph_tmp(GlyphRef p) { _obj_ = p; }
    Glyph_tmp(const Glyph& r);
    Glyph_tmp(const Glyph_tmp& r);
    ~Glyph_tmp();
};

class Glyph_var {
public:
    GlyphRef _obj_;

    Glyph_var(GlyphRef p) { _obj_ = p; }
    operator GlyphRef() const { return _obj_; }
    GlyphRef operator ->() { return _obj_; }
};

class GlyphType : public FrescoObjectType {
protected:
    GlyphType();
    virtual ~GlyphType();
public:
    Glyph_tmp clone_glyph() {
        return _c_clone_glyph();
    }
    virtual GlyphRef _c_clone_glyph();
    StyleObj_tmp style() {
        return _c_style();
    }
    virtual StyleObjRef _c_style();
    void style(StyleObj_in _p) {
        _c_style(_p);
    }
    virtual void _c_style(StyleObj_in _p);
    TransformObj_tmp transform() {
        return _c_transform();
    }
    virtual TransformObjRef _c_transform();
    virtual void request(Glyph::Requisition& r);
    virtual void extension(const Glyph::AllocationInfo& a, Region_in r);
    Region_tmp shape() {
        return _c_shape();
    }
    virtual RegionRef _c_shape();
    virtual void traverse(GlyphTraversal_in t);
    virtual void draw(GlyphTraversal_in t);
    virtual void pick(GlyphTraversal_in t);
    Glyph_tmp body() {
        return _c_body();
    }
    virtual GlyphRef _c_body();
    void body(Glyph_in _p) {
        _c_body(_p);
    }
    virtual void _c_body(Glyph_in _p);
    GlyphOffset_tmp append(Glyph_in g);
    virtual GlyphOffsetRef _c_append(Glyph_in g);
    GlyphOffset_tmp prepend(Glyph_in g);
    virtual GlyphOffsetRef _c_prepend(Glyph_in g);
    virtual Tag add_parent(GlyphOffset_in parent_offset);
    virtual void remove_parent(Tag add_tag);
    virtual void visit_children(GlyphVisitor_in v);
    virtual void visit_children_reversed(GlyphVisitor_in v);
    virtual void visit_parents(GlyphVisitor_in v);
    virtual void allocations(Glyph::AllocationInfoList& a);
    virtual void need_redraw();
    virtual void need_redraw_region(Region_in r);
    virtual void need_resize();
    virtual Boolean restore_trail(GlyphTraversal_in t);
    GlyphRef _obj() { return this; }
    void* _this();
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

class GlyphVisitor {
public:
    GlyphVisitorRef _obj_;

    GlyphVisitor() { _obj_ = 0; }
    GlyphVisitor(GlyphVisitorRef p) { _obj_ = p; }
    GlyphVisitor& operator =(GlyphVisitorRef p);
    GlyphVisitor(const GlyphVisitor&);
    GlyphVisitor& operator =(const GlyphVisitor& r);
    GlyphVisitor(const GlyphVisitor_tmp&);
    GlyphVisitor& operator =(const GlyphVisitor_tmp&);
    GlyphVisitor(const GlyphVisitor_var&);
    GlyphVisitor& operator =(const GlyphVisitor_var&);
    ~GlyphVisitor();

    GlyphVisitorRef operator ->() { return _obj_; }

    operator GlyphVisitor_in() const { return _obj_; }
    operator FrescoObject() const;
    static GlyphVisitorRef _narrow(BaseObjectRef p);
    static GlyphVisitor_tmp _narrow(const BaseObject& r);

    static GlyphVisitorRef _duplicate(GlyphVisitorRef obj);
    static GlyphVisitor_tmp _duplicate(const GlyphVisitor& r);
};

class GlyphVisitor_tmp : public GlyphVisitor {
public:
    GlyphVisitor_tmp(GlyphVisitorRef p) { _obj_ = p; }
    GlyphVisitor_tmp(const GlyphVisitor& r);
    GlyphVisitor_tmp(const GlyphVisitor_tmp& r);
    ~GlyphVisitor_tmp();
};

class GlyphVisitor_var {
public:
    GlyphVisitorRef _obj_;

    GlyphVisitor_var(GlyphVisitorRef p) { _obj_ = p; }
    operator GlyphVisitorRef() const { return _obj_; }
    GlyphVisitorRef operator ->() { return _obj_; }
};

class GlyphVisitorType : public FrescoObjectType {
protected:
    GlyphVisitorType();
    virtual ~GlyphVisitorType();
public:
    virtual Boolean visit(Glyph_in glyph, GlyphOffset_in offset);
    GlyphVisitorRef _obj() { return this; }
    void* _this();
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

class GlyphOffset {
public:
    GlyphOffsetRef _obj_;

    GlyphOffset() { _obj_ = 0; }
    GlyphOffset(GlyphOffsetRef p) { _obj_ = p; }
    GlyphOffset& operator =(GlyphOffsetRef p);
    GlyphOffset(const GlyphOffset&);
    GlyphOffset& operator =(const GlyphOffset& r);
    GlyphOffset(const GlyphOffset_tmp&);
    GlyphOffset& operator =(const GlyphOffset_tmp&);
    GlyphOffset(const GlyphOffset_var&);
    GlyphOffset& operator =(const GlyphOffset_var&);
    ~GlyphOffset();

    GlyphOffsetRef operator ->() { return _obj_; }

    operator GlyphOffset_in() const { return _obj_; }
    operator FrescoObject() const;
    static GlyphOffsetRef _narrow(BaseObjectRef p);
    static GlyphOffset_tmp _narrow(const BaseObject& r);

    static GlyphOffsetRef _duplicate(GlyphOffsetRef obj);
    static GlyphOffset_tmp _duplicate(const GlyphOffset& r);
};

class GlyphOffset_tmp : public GlyphOffset {
public:
    GlyphOffset_tmp(GlyphOffsetRef p) { _obj_ = p; }
    GlyphOffset_tmp(const GlyphOffset& r);
    GlyphOffset_tmp(const GlyphOffset_tmp& r);
    ~GlyphOffset_tmp();
};

class GlyphOffset_var {
public:
    GlyphOffsetRef _obj_;

    GlyphOffset_var(GlyphOffsetRef p) { _obj_ = p; }
    operator GlyphOffsetRef() const { return _obj_; }
    GlyphOffsetRef operator ->() { return _obj_; }
};

class GlyphOffsetType : public FrescoObjectType {
protected:
    GlyphOffsetType();
    virtual ~GlyphOffsetType();
public:
    Glyph_tmp parent() {
        return _c_parent();
    }
    virtual GlyphRef _c_parent();
    Glyph_tmp child() {
        return _c_child();
    }
    virtual GlyphRef _c_child();
    virtual void allocations(Glyph::AllocationInfoList& a);
    GlyphOffset_tmp insert(Glyph_in g) {
        return _c_insert(g);
    }
    virtual GlyphOffsetRef _c_insert(Glyph_in g);
    virtual void replace(Glyph_in g);
    virtual void remove();
    virtual void notify();
    virtual void visit_trail(GlyphTraversal_in t);
    virtual void child_allocate(Glyph::AllocationInfo& a);
    GlyphOffsetRef _obj() { return this; }
    void* _this();
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

class ViewerType;
typedef ViewerType* ViewerRef;
typedef ViewerRef Viewer_in;
class Viewer;
class Viewer_tmp;
class Viewer_var;

class GlyphTraversal {
public:
    GlyphTraversalRef _obj_;

    GlyphTraversal() { _obj_ = 0; }
    GlyphTraversal(GlyphTraversalRef p) { _obj_ = p; }
    GlyphTraversal& operator =(GlyphTraversalRef p);
    GlyphTraversal(const GlyphTraversal&);
    GlyphTraversal& operator =(const GlyphTraversal& r);
    GlyphTraversal(const GlyphTraversal_tmp&);
    GlyphTraversal& operator =(const GlyphTraversal_tmp&);
    GlyphTraversal(const GlyphTraversal_var&);
    GlyphTraversal& operator =(const GlyphTraversal_var&);
    ~GlyphTraversal();

    GlyphTraversalRef operator ->() { return _obj_; }

    operator GlyphTraversal_in() const { return _obj_; }
    operator FrescoObject() const;
    static GlyphTraversalRef _narrow(BaseObjectRef p);
    static GlyphTraversal_tmp _narrow(const BaseObject& r);

    static GlyphTraversalRef _duplicate(GlyphTraversalRef obj);
    static GlyphTraversal_tmp _duplicate(const GlyphTraversal& r);
    enum Operation {
        draw, pick_top, pick_any, pick_all, other
    };
};

class GlyphTraversal_tmp : public GlyphTraversal {
public:
    GlyphTraversal_tmp(GlyphTraversalRef p) { _obj_ = p; }
    GlyphTraversal_tmp(const GlyphTraversal& r);
    GlyphTraversal_tmp(const GlyphTraversal_tmp& r);
    ~GlyphTraversal_tmp();
};

class GlyphTraversal_var {
public:
    GlyphTraversalRef _obj_;

    GlyphTraversal_var(GlyphTraversalRef p) { _obj_ = p; }
    operator GlyphTraversalRef() const { return _obj_; }
    GlyphTraversalRef operator ->() { return _obj_; }
};

class GlyphTraversalType : public FrescoObjectType {
protected:
    GlyphTraversalType();
    virtual ~GlyphTraversalType();
public:
    virtual GlyphTraversal::Operation op();
    virtual GlyphTraversal::Operation swap_op(GlyphTraversal::Operation op);
    virtual void begin_trail(Viewer_in v);
    virtual void end_trail();
    virtual void traverse_child(GlyphOffset_in o, Region_in allocation);
    virtual void visit();
    GlyphTraversal_tmp trail() {
        return _c_trail();
    }
    virtual GlyphTraversalRef _c_trail();
    Glyph_tmp current_glyph() {
        return _c_current_glyph();
    }
    virtual GlyphRef _c_current_glyph();
    GlyphOffset_tmp current_offset() {
        return _c_current_offset();
    }
    virtual GlyphOffsetRef _c_current_offset();
    Viewer_tmp current_viewer();
    virtual ViewerRef _c_current_viewer();
    virtual Boolean forward();
    virtual Boolean backward();
    PainterObj_tmp painter() {
        return _c_painter();
    }
    virtual PainterObjRef _c_painter();
    void painter(PainterObj_in _p) {
        _c_painter(_p);
    }
    virtual void _c_painter(PainterObj_in _p);
    DisplayObj_tmp display() {
        return _c_display();
    }
    virtual DisplayObjRef _c_display();
    ScreenObj_tmp screen() {
        return _c_screen();
    }
    virtual ScreenObjRef _c_screen();
    Region_tmp allocation() {
        return _c_allocation();
    }
    virtual RegionRef _c_allocation();
    virtual Boolean bounds(Vertex& lower, Vertex& upper, Vertex& origin);
    virtual Boolean origin(Vertex& origin);
    virtual Boolean span(Axis a, Region::BoundingSpan& s);
    TransformObj_tmp transform() {
        return _c_transform();
    }
    virtual TransformObjRef _c_transform();
    DamageObj_tmp damage() {
        return _c_damage();
    }
    virtual DamageObjRef _c_damage();
    virtual void hit();
    virtual Long hit_info();
    virtual void hit_info(Long _p);
    GlyphTraversal_tmp picked() {
        return _c_picked();
    }
    virtual GlyphTraversalRef _c_picked();
    virtual void clear();
    GlyphTraversalRef _obj() { return this; }
    void* _this();
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

inline DamageObjRef DamageObj::_duplicate(DamageObjRef obj) {
    return (DamageObjRef)_BaseObject__duplicate(obj, &DamageObjStub::_create);
}
inline DamageObj& DamageObj::operator =(DamageObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = DamageObj::_duplicate(p);
    return *this;
}
inline DamageObj::DamageObj(const DamageObj& r) {
    _obj_ = DamageObj::_duplicate(r._obj_);
}
inline DamageObj& DamageObj::operator =(const DamageObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = DamageObj::_duplicate(r._obj_);
    return *this;
}
inline DamageObj::DamageObj(const DamageObj_tmp& r) {
    _obj_ = r._obj_;
    ((DamageObj_tmp*)&r)->_obj_ = 0;
}
inline DamageObj& DamageObj::operator =(const DamageObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((DamageObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline DamageObj::DamageObj(const DamageObj_var& e) {
    _obj_ = DamageObj::_duplicate(e._obj_);
}
inline DamageObj& DamageObj::operator =(const DamageObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = DamageObj::_duplicate(e._obj_);
    return *this;
}
inline DamageObj::~DamageObj() {
    _BaseObject__release(_obj_);
}
inline DamageObj_tmp DamageObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline DamageObj_tmp DamageObj::_duplicate(const DamageObj& r) {
    return _duplicate(r._obj_);
}
inline DamageObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline DamageObj_tmp::DamageObj_tmp(const DamageObj& r) {
    _obj_ = DamageObj::_duplicate(r._obj_);
}
inline DamageObj_tmp::DamageObj_tmp(const DamageObj_tmp& r) {
    _obj_ = r._obj_;
    ((DamageObj_tmp*)&r)->_obj_ = 0;
}
inline DamageObj_tmp::~DamageObj_tmp() { }

inline GlyphRef Glyph::_duplicate(GlyphRef obj) {
    return (GlyphRef)_BaseObject__duplicate(obj, &GlyphStub::_create);
}
inline Glyph& Glyph::operator =(GlyphRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Glyph::_duplicate(p);
    return *this;
}
inline Glyph::Glyph(const Glyph& r) {
    _obj_ = Glyph::_duplicate(r._obj_);
}
inline Glyph& Glyph::operator =(const Glyph& r) {
    _BaseObject__release(_obj_);
    _obj_ = Glyph::_duplicate(r._obj_);
    return *this;
}
inline Glyph::Glyph(const Glyph_tmp& r) {
    _obj_ = r._obj_;
    ((Glyph_tmp*)&r)->_obj_ = 0;
}
inline Glyph& Glyph::operator =(const Glyph_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Glyph_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Glyph::Glyph(const Glyph_var& e) {
    _obj_ = Glyph::_duplicate(e._obj_);
}
inline Glyph& Glyph::operator =(const Glyph_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Glyph::_duplicate(e._obj_);
    return *this;
}
inline Glyph::~Glyph() {
    _BaseObject__release(_obj_);
}
inline Glyph_tmp Glyph::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Glyph_tmp Glyph::_duplicate(const Glyph& r) {
    return _duplicate(r._obj_);
}
inline Glyph::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Glyph_tmp::Glyph_tmp(const Glyph& r) {
    _obj_ = Glyph::_duplicate(r._obj_);
}
inline Glyph_tmp::Glyph_tmp(const Glyph_tmp& r) {
    _obj_ = r._obj_;
    ((Glyph_tmp*)&r)->_obj_ = 0;
}
inline Glyph_tmp::~Glyph_tmp() { }

inline GlyphVisitorRef GlyphVisitor::_duplicate(GlyphVisitorRef obj) {
    return (GlyphVisitorRef)_BaseObject__duplicate(obj, &GlyphVisitorStub::_create);
}
inline GlyphVisitor& GlyphVisitor::operator =(GlyphVisitorRef p) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphVisitor::_duplicate(p);
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const GlyphVisitor& r) {
    _obj_ = GlyphVisitor::_duplicate(r._obj_);
}
inline GlyphVisitor& GlyphVisitor::operator =(const GlyphVisitor& r) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphVisitor::_duplicate(r._obj_);
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const GlyphVisitor_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphVisitor_tmp*)&r)->_obj_ = 0;
}
inline GlyphVisitor& GlyphVisitor::operator =(const GlyphVisitor_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((GlyphVisitor_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline GlyphVisitor::GlyphVisitor(const GlyphVisitor_var& e) {
    _obj_ = GlyphVisitor::_duplicate(e._obj_);
}
inline GlyphVisitor& GlyphVisitor::operator =(const GlyphVisitor_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphVisitor::_duplicate(e._obj_);
    return *this;
}
inline GlyphVisitor::~GlyphVisitor() {
    _BaseObject__release(_obj_);
}
inline GlyphVisitor_tmp GlyphVisitor::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline GlyphVisitor_tmp GlyphVisitor::_duplicate(const GlyphVisitor& r) {
    return _duplicate(r._obj_);
}
inline GlyphVisitor::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline GlyphVisitor_tmp::GlyphVisitor_tmp(const GlyphVisitor& r) {
    _obj_ = GlyphVisitor::_duplicate(r._obj_);
}
inline GlyphVisitor_tmp::GlyphVisitor_tmp(const GlyphVisitor_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphVisitor_tmp*)&r)->_obj_ = 0;
}
inline GlyphVisitor_tmp::~GlyphVisitor_tmp() { }

inline GlyphOffsetRef GlyphOffset::_duplicate(GlyphOffsetRef obj) {
    return (GlyphOffsetRef)_BaseObject__duplicate(obj, &GlyphOffsetStub::_create);
}
inline GlyphOffset& GlyphOffset::operator =(GlyphOffsetRef p) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphOffset::_duplicate(p);
    return *this;
}
inline GlyphOffset::GlyphOffset(const GlyphOffset& r) {
    _obj_ = GlyphOffset::_duplicate(r._obj_);
}
inline GlyphOffset& GlyphOffset::operator =(const GlyphOffset& r) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphOffset::_duplicate(r._obj_);
    return *this;
}
inline GlyphOffset::GlyphOffset(const GlyphOffset_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphOffset_tmp*)&r)->_obj_ = 0;
}
inline GlyphOffset& GlyphOffset::operator =(const GlyphOffset_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((GlyphOffset_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline GlyphOffset::GlyphOffset(const GlyphOffset_var& e) {
    _obj_ = GlyphOffset::_duplicate(e._obj_);
}
inline GlyphOffset& GlyphOffset::operator =(const GlyphOffset_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphOffset::_duplicate(e._obj_);
    return *this;
}
inline GlyphOffset::~GlyphOffset() {
    _BaseObject__release(_obj_);
}
inline GlyphOffset_tmp GlyphOffset::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline GlyphOffset_tmp GlyphOffset::_duplicate(const GlyphOffset& r) {
    return _duplicate(r._obj_);
}
inline GlyphOffset::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline GlyphOffset_tmp::GlyphOffset_tmp(const GlyphOffset& r) {
    _obj_ = GlyphOffset::_duplicate(r._obj_);
}
inline GlyphOffset_tmp::GlyphOffset_tmp(const GlyphOffset_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphOffset_tmp*)&r)->_obj_ = 0;
}
inline GlyphOffset_tmp::~GlyphOffset_tmp() { }

inline GlyphTraversalRef GlyphTraversal::_duplicate(GlyphTraversalRef obj) {
    return (GlyphTraversalRef)_BaseObject__duplicate(obj, &GlyphTraversalStub::_create);
}
inline GlyphTraversal& GlyphTraversal::operator =(GlyphTraversalRef p) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphTraversal::_duplicate(p);
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const GlyphTraversal& r) {
    _obj_ = GlyphTraversal::_duplicate(r._obj_);
}
inline GlyphTraversal& GlyphTraversal::operator =(const GlyphTraversal& r) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphTraversal::_duplicate(r._obj_);
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const GlyphTraversal_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphTraversal_tmp*)&r)->_obj_ = 0;
}
inline GlyphTraversal& GlyphTraversal::operator =(const GlyphTraversal_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((GlyphTraversal_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline GlyphTraversal::GlyphTraversal(const GlyphTraversal_var& e) {
    _obj_ = GlyphTraversal::_duplicate(e._obj_);
}
inline GlyphTraversal& GlyphTraversal::operator =(const GlyphTraversal_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = GlyphTraversal::_duplicate(e._obj_);
    return *this;
}
inline GlyphTraversal::~GlyphTraversal() {
    _BaseObject__release(_obj_);
}
inline GlyphTraversal_tmp GlyphTraversal::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline GlyphTraversal_tmp GlyphTraversal::_duplicate(const GlyphTraversal& r) {
    return _duplicate(r._obj_);
}
inline GlyphTraversal::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline GlyphTraversal_tmp::GlyphTraversal_tmp(const GlyphTraversal& r) {
    _obj_ = GlyphTraversal::_duplicate(r._obj_);
}
inline GlyphTraversal_tmp::GlyphTraversal_tmp(const GlyphTraversal_tmp& r) {
    _obj_ = r._obj_;
    ((GlyphTraversal_tmp*)&r)->_obj_ = 0;
}
inline GlyphTraversal_tmp::~GlyphTraversal_tmp() { }

#endif
