/*
 * tools.h
 */

#ifndef _tools_h
#define _tools_h

#include <X11/Fresco/types.h>
#include <X11/Fresco/Impls/transform.h>

class EventType;
typedef EventType* EventRef;

class DamageObjType;
typedef DamageObjType* DamageObjRef;

class FigViewer;
class GlyphTraversalType;
class Manipulator;

typedef GlyphTraversalType* GlyphTraversalRef;

class SelectInfo {
public:
    SelectInfo();
    virtual ~SelectInfo();
    SelectInfo* copy();
public:
    TransformImpl t_; // traversal transform
    Manipulator* m_; // selected manipulator;
    long level_; // depth of a selected manipulator
    DamageObjRef d_;
};

//- Tool*
//+ Tool : FrescoObjectType
class Tool : public FrescoObjectType {
public:
    ~Tool();
    TypeObjId _tid();
    static Tool* _narrow(BaseObjectRef);
//+
public:
    virtual SelectInfo* create_manipulator(
        GlyphTraversalRef, EventRef, FigViewer*
    );
    void anchor(Coord&, Coord&);  // for multiple selections
protected:
    Tool();
protected:
    Coord ax_, ay_;
};

//- SelectTool*
//+ SelectTool : Tool
class SelectTool : public Tool {
public:
    ~SelectTool();
    TypeObjId _tid();
    static SelectTool* _narrow(BaseObjectRef);
//+
public:
    SelectTool();

    virtual SelectInfo* create_manipulator(
        GlyphTraversalRef, EventRef, FigViewer*
    );
};

//- CreateTool*
//+ CreateTool : Tool
class CreateTool : public Tool {
public:
    ~CreateTool();
    TypeObjId _tid();
    static CreateTool* _narrow(BaseObjectRef);
//+
public:
    CreateTool(Manipulator*);

    virtual SelectInfo* create_manipulator(
        GlyphTraversalRef, EventRef, FigViewer*
    );
protected:
    Manipulator* manip_;
};

//- MoveTool*
//+ MoveTool : Tool
class MoveTool : public Tool {
public:
    ~MoveTool();
    TypeObjId _tid();
    static MoveTool* _narrow(BaseObjectRef);
//+
public:
    MoveTool();
};

//- ScaleTool*
//+ ScaleTool : Tool
class ScaleTool : public Tool {
public:
    ~ScaleTool();
    TypeObjId _tid();
    static ScaleTool* _narrow(BaseObjectRef);
//+
public:
    ScaleTool();
};

//- RotateTool*
//+ RotateTool : Tool
class RotateTool : public Tool {
public:
    ~RotateTool();
    TypeObjId _tid();
    static RotateTool* _narrow(BaseObjectRef);
//+
public:
    RotateTool();
};

#endif
