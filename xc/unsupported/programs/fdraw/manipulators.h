/*
 * manipulator.h
 */

#ifndef _manipulator_h
#define _manipulator_h

#include <X11/Fresco/Impls/figures.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/OS/list.h>
#include <X11/Fresco/types.h>

class FigViewer;
class Manipulator;

declarePtrList(ManipList, Manipulator);

class ButtonType;
typedef ButtonType* ButtonRef;

class Command;
class SelectInfo;
class Tool;

enum ToolType {
    create, magnify, move, alter, rotate, scale, choose, stretch, undefined
};

//- Transformer*
//+ Transformer : MonoGlyph
class Transformer : public MonoGlyph {
public:
    ~Transformer();
    TypeObjId _tid();
    static Transformer* _narrow(BaseObjectRef);
//+
public:
    Transformer(TransformObjRef);
    Transformer();
    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, Region_in r); //+ Glyph::extension
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
    TransformObjRef _c_transform(); //+ Glyph::transform

    void child_allocate(Glyph::AllocationInfo& a);
protected:
    TransformImpl tx_;
};

class ManipInfo {
public:
    ManipInfo(SelectInfo*);
    virtual ~ManipInfo();
public:
    ToolType tooltype_;
    Vertex center_;  // traversal independent center vertex
    Vertex pt_;  // traversal independent event position
    Boolean obj_manip_;  // manipulate the actual object
    Coord ax_, ay_; // initial anchor position for multiple selections
    Coord tool_ax_, tool_ay_; // tool anchor position
    TransformImpl t_; // traversal transform inverted;

    SelectInfo* sinfo_;
};

//- Manipulator*
//+ Manipulator : Transformer
class Manipulator : public Transformer {
public:
    ~Manipulator();
    TypeObjId _tid();
    static Manipulator* _narrow(BaseObjectRef);
//+
public:
    virtual Boolean grasp(Tool&, FigViewer&, SelectInfo*, EventRef);
    virtual Boolean manipulate(EventRef);
    virtual Command* effect(EventRef);

    virtual void execute(Command*);
    virtual void unexecute(Command*);

    virtual Manipulator* shallow_copy() = 0;
    virtual Manipulator* deep_copy() = 0;

    virtual void draw_handles(GlyphTraversalRef);

    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
protected:
    Manipulator(Manipulator* = nil);
    void info(Manipulator*, ManipInfo*);
    ManipInfo* info(Manipulator*);
protected:
    Boolean selected_ : 1;
protected: 
    ManipInfo* info_;  // cache information for manipulation
};

//- FigureManip*
//+ FigureManip : Manipulator
class FigureManip : public Manipulator {
public:
    ~FigureManip();
    TypeObjId _tid();
    static FigureManip* _narrow(BaseObjectRef);
//+
public:
    virtual Boolean grasp(Tool&, FigViewer&, SelectInfo*, EventRef);
    virtual Manipulator* shallow_copy() = 0;
    virtual Manipulator* deep_copy() = 0;
protected:
    FigureManip(FigureManip* m = nil);
    void figure_body(Figure*);
protected:
    Figure* figure_;
};

//- VertexManip*
//+ VertexManip : FigureManip
class VertexManip : public FigureManip {
public:
    ~VertexManip();
    TypeObjId _tid();
    static VertexManip* _narrow(BaseObjectRef);
//+
protected:
    VertexManip(VertexManip* m = nil);

    virtual Boolean grasp(Tool&, FigViewer&, SelectInfo*, EventRef);
    virtual Boolean manipulate(EventRef);

    virtual Manipulator* shallow_copy() = 0;
    virtual Manipulator* deep_copy() = 0;
    virtual void recompute_shape();

    virtual void draw_handles(GlyphTraversalRef);
protected:
    VertexList* vlist_;
};

//- LineManip*
//+ LineManip : VertexManip
class LineManip : public VertexManip {
public:
    ~LineManip();
    TypeObjId _tid();
    static LineManip* _narrow(BaseObjectRef);
//+
public:
    LineManip(LineManip* m = nil);

    virtual Boolean manipulate(EventRef);
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- RectManip*
//+ RectManip : VertexManip
class RectManip : public VertexManip {
public:
    ~RectManip();
    TypeObjId _tid();
    static RectManip* _narrow(BaseObjectRef);
//+
public:
    RectManip(RectManip* m = nil);

    virtual Boolean manipulate(EventRef);
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- EllipseManip*
//+ EllipseManip : VertexManip
class EllipseManip : public VertexManip {
public:
    ~EllipseManip();
    TypeObjId _tid();
    static EllipseManip* _narrow(BaseObjectRef);
//+
public:
    EllipseManip(EllipseManip* m = nil);

    virtual Boolean manipulate(EventRef);
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
    virtual void draw_handles(GlyphTraversalRef);
    virtual void recompute_shape();
};

//- Open_BSplineManip*
//+ Open_BSplineManip : VertexManip
class Open_BSplineManip : public VertexManip {
public:
    ~Open_BSplineManip();
    TypeObjId _tid();
    static Open_BSplineManip* _narrow(BaseObjectRef);
//+
public:
    Open_BSplineManip(Open_BSplineManip* m = nil);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
    virtual void recompute_shape();
};

//- Closed_BSplineManip*
//+ Closed_BSplineManip : VertexManip
class Closed_BSplineManip : public VertexManip {
public:
    ~Closed_BSplineManip();
    TypeObjId _tid();
    static Closed_BSplineManip* _narrow(BaseObjectRef);
//+
public:
    Closed_BSplineManip(Closed_BSplineManip* m = nil);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
    virtual void recompute_shape();
};

//- MultiLineManip*
//+ MultiLineManip : VertexManip
class MultiLineManip : public VertexManip {
public:
    ~MultiLineManip();
    TypeObjId _tid();
    static MultiLineManip* _narrow(BaseObjectRef);
//+
public:
    MultiLineManip(MultiLineManip* m = nil);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- PolygonManip*
//+ PolygonManip : VertexManip
class PolygonManip : public VertexManip {
public:
    ~PolygonManip();
    TypeObjId _tid();
    static PolygonManip* _narrow(BaseObjectRef);
//+
public:
    PolygonManip(PolygonManip* m = nil);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- PolyManip*
//+ PolyManip : Manipulator
class PolyManip : public Manipulator {
public:
    ~PolyManip();
    TypeObjId _tid();
    static PolyManip* _narrow(BaseObjectRef);
//+
public:
    PolyManip(PolyFigure*, PolyManip* m = nil);

    virtual void execute(Command*);
    virtual void unexecute(Command*);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();

protected:
    PolyFigure* polyfigure_;
};

//- ButtonManip*
//+ ButtonManip : Manipulator
class ButtonManip : public Manipulator {
public:
    ~ButtonManip();
    TypeObjId _tid();
    static ButtonManip* _narrow(BaseObjectRef);
//+
public:
    ButtonManip(
        ViewerRef, Boolean*, Transformer* t = nil, ButtonManip* m = nil
    );

    virtual void execute(Command*);
    virtual void unexecute(Command*);
        
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
protected:
    Transformer* transformer_;
    ViewerRef button_;
    Boolean* editing_;
};

//- BoxManip*
//+ BoxManip : Manipulator
class BoxManip : public Manipulator {
public:
    ~BoxManip();
    TypeObjId _tid();
    static BoxManip* _narrow(BaseObjectRef);
//+
public:
    virtual void execute(Command*);
    virtual void unexecute(Command*);
    virtual Manipulator* shallow_copy() = 0;
    virtual Manipulator* deep_copy() = 0;
protected:
    BoxManip(Transformer*, BoxManip* m = nil);
protected:
    Transformer* transformer_;
};

//- HBoxManip*
//+ HBoxManip : BoxManip
class HBoxManip : public BoxManip {
public:
    ~HBoxManip();
    TypeObjId _tid();
    static HBoxManip* _narrow(BaseObjectRef);
//+
public:
    HBoxManip(Transformer* t = nil, HBoxManip* m = nil);
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- VBoxManip*
//+ VBoxManip : BoxManip
class VBoxManip : public BoxManip {
public:
    ~VBoxManip();
    TypeObjId _tid();
    static VBoxManip* _narrow(BaseObjectRef);
//+
public:
    VBoxManip(Transformer* t = nil, VBoxManip* m = nil);
    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();
};

//- MacroManip*
//+ MacroManip : Manipulator
class MacroManip : public Manipulator {
public:
    ~MacroManip();
    TypeObjId _tid();
    static MacroManip* _narrow(BaseObjectRef);
//+
public:
    MacroManip(MacroManip* m = nil);

    virtual Boolean grasp(Tool&, FigViewer& fv, SelectInfo*, EventRef);
    virtual Boolean manipulate(EventRef);
    virtual Command* effect(EventRef);

    virtual void execute(Command*);
    virtual void unexecute(Command*);

    virtual Manipulator* shallow_copy();
    virtual Manipulator* deep_copy();

    virtual void add(Manipulator*);
    virtual void insert(long index, Manipulator*);
    virtual void remove(long index);
    virtual void remove_all();
    virtual long count();

    Manipulator* manipulator(long);
protected:
    ManipList* maniplist_;
};


#endif
