/*
 * visitors.h
 */

#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/OS/list.h>

class RegionImpl;
class TransformImpl;
class Command;
class ManipList;

declarePtrList(AllocationList, RegionImpl);
declarePtrList(TransformList, TransformImpl);

class Appender : public GlyphVisitorImpl {
public:
    Appender(GlyphRef);
    ~Appender();
    Boolean visit(GlyphRef, GlyphOffsetRef);
protected:
    GlyphRef glyph_;
};

class Remover : public GlyphVisitorImpl {
public:
    Remover();
    ~Remover();
    Boolean visit(GlyphRef, GlyphOffsetRef);
protected:
    GlyphOffsetList list_;
};

class Counter : public GlyphVisitorImpl {
public:
    Counter();
    Boolean visit(GlyphRef, GlyphOffsetRef);
    long count();
protected:
    long count_;
};

inline long Counter::count () { return count_; }

class CmdVisitor : public GlyphVisitorImpl {
public:
    CmdVisitor(Command* cmd, Boolean execute = true);
    ~CmdVisitor();

    Boolean visit(GlyphRef, GlyphOffsetRef);
protected:
    Command* cmd_;
    Boolean execute_;
};

class ManipCopier : public GlyphVisitorImpl {
public:
    ManipCopier(Boolean shallow = true);
    ~ManipCopier();

    Boolean visit(GlyphRef, GlyphOffsetRef);
    ManipList* manipulators();
private:
    ManipList* maniplist_;
    Boolean shallow_;
};

inline ManipList* ManipCopier::manipulators () { return maniplist_; }

class TAManipCopier : public ManipCopier {
public:
    TAManipCopier(RegionRef a, Boolean shallow = true);
    ~TAManipCopier();

    Boolean visit(GlyphRef, GlyphOffsetRef);

    AllocationList* allocations();
    TransformList* transforms();
private:
    AllocationList* alist_;
    TransformList* tlist_;
    RegionRef a_;
};

inline AllocationList* TAManipCopier::allocations () { return alist_; }
inline TransformList* TAManipCopier::transforms () { return tlist_; }

class OffsetVisitor : public GlyphVisitorImpl {
public:
    OffsetVisitor();
    ~OffsetVisitor();

    Boolean visit(GlyphRef, GlyphOffsetRef);
    GlyphOffsetRef offset(long);
protected:
    GlyphOffsetList* glist_;
};

