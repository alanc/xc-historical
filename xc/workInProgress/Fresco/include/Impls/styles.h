/*
 * Copyright (c) 1987-91 Stanford University
 * Copyright (c) 1991-93 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#ifndef Fresco_lib_style_h
#define Fresco_lib_style_h

#include <X11/Fresco/Impls/fresco-impl.h>
#include <X11/Fresco/OS/list.h>
#include <X11/Fresco/OS/thread.h>

class StyleValueImplList;
class StyleValueTable;
class StyleValueTableEntry;
class Fresco;
class StyleList;

declarePtrList(StringList,CharStringType)

typedef StringList PathName;

class StyleValueImpl : public StyleValueType {
public:
    StyleValueImpl(Fresco*, CharStringRef name, PathName* path);
    ~StyleValueImpl();

    //+ StyleValue::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* StyleValue */
    CharStringRef _c_name();
    Boolean uninitialized();
    Long priority();
    void priority(Long _p);
    Boolean is_on();
    Boolean read_boolean(Boolean& b);
    void write_boolean(Boolean b);
    Boolean read_coord(Coord& c);
    void write_coord(Coord c);
    Boolean read_integer(Long& i);
    void write_integer(Long i);
    Boolean read_real(Double& d);
    void write_real(Double d);
    Boolean _c_read_string(CharStringRef& s);
    void write_string(CharStringRef s);
    Boolean _c_read_value(FrescoObjectRef& s);
    void write_value(FrescoObjectRef s);
    void lock();
    void unlock();
    //+

    PathName* path() { return path_; }
    CharStringRef parse_value(CharStringRef);

    static void delete_path(PathName*);
protected:
    LockedFrescoObjectImpl object_;
    CharStringRef name_;
    PathName* path_;
    Boolean uninitialized_;
    CharStringRef value_;
    FrescoObjectRef any_value_;
    long priority_;

    void modified();
};

/*
 * SharedStyleImpl provides an implementation of the Style operations
 * suitable for using with Style implementations (e.g., Viewer).
 */

class SharedStyleImpl {
public:
    SharedStyleImpl(Fresco*);
    ~SharedStyleImpl();

    //+ StyleObj::=
    StyleObjRef _c_new_style();
    StyleObjRef _c_parent_style();
    void link_parent(StyleObjRef parent);
    void unlink_parent();
    Tag link_child(StyleObjRef child);
    void unlink_child(Tag link_tag);
    void merge(StyleObjRef s);
    CharStringRef _c_name();
    void _c_name(CharStringRef _p);
    void alias(CharStringRef s);
    Boolean is_on(CharStringRef name);
    StyleValueRef _c_bind(CharStringRef name);
    void unbind(CharStringRef name);
    StyleValueRef _c_resolve(CharStringRef name);
    StyleValueRef _c_resolve_wildcard(CharStringRef name, StyleObjRef start);
    Long match(CharStringRef name);
    void visit_aliases(StyleVisitorRef v);
    void visit_attributes(StyleVisitorRef v);
    void visit_styles(StyleVisitorRef v);
    void lock();
    void unlock();
    //+

    struct Info {
	StyleObjRef child;
	Tag tag;
    };

    Fresco* fresco_;
    StyleObjRef style_;
    LockObj lock_;

    void load_file(const char* filename, Long priority);
    void load_list(const char* str, Long length, Long priority);
    void load_property(const char* property, Long length, Long priority);

    StyleValueRef wildcard_match(StyleValueTableEntry* e, StyleObjRef start);
    StyleValueRef wildcard_match_name(
	CharStringRef name, StyleValueTableEntry*, StyleObjRef cur
    );
protected:
    CharStringRef name_;
    PathName* aliases_;
    StyleObjRef parent_;
    Tag links_;
    Tag unlink_;
    StyleValueTable* table_;
    StyleList* children_;
    Boolean modified_;
    long match_priority_;

    PathName* parse_name(CharStringRef name);
    Long find_separator(const char* start, const char* end);
    Boolean same_path(PathName* list1, PathName* list2);

    void strip(const char*& prop, Long& length);
    const char* index(const char* s, Long length, char c);
    void missing_colon(const char* s, Long length);
    void bad_property_name(const char* s, Long length);
    void bad_property_value(const char* s, Long length);

    StyleValueTableEntry* find_tail_entry(PathName*, Boolean force);
    StyleValueTableEntry* find_entry(CharStringRef, Boolean force);
    long finish_wildcard_match(StyleObjRef, PathName*, long p_index);
};

/*
 * StyleImpl is an implementation of Style that can be constructed directly.
 */

class StyleImpl : public StyleObjType {
public:
    StyleImpl(Fresco*);
    ~StyleImpl();

    //+ StyleObj::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* StyleObj */
    StyleObjRef _c_new_style();
    StyleObjRef _c_parent_style();
    void link_parent(StyleObjRef parent);
    void unlink_parent();
    Tag link_child(StyleObjRef child);
    void unlink_child(Tag link_tag);
    void merge(StyleObjRef s);
    CharStringRef _c_name();
    void _c_name(CharStringRef _p);
    void alias(CharStringRef s);
    Boolean is_on(CharStringRef name);
    StyleValueRef _c_bind(CharStringRef name);
    void unbind(CharStringRef name);
    StyleValueRef _c_resolve(CharStringRef name);
    StyleValueRef _c_resolve_wildcard(CharStringRef name, StyleObjRef start);
    Long match(CharStringRef name);
    void visit_aliases(StyleVisitorRef v);
    void visit_attributes(StyleVisitorRef v);
    void visit_styles(StyleVisitorRef v);
    void lock();
    void unlock();
    //+
protected:
    LockedFrescoObjectImpl object_;
    SharedStyleImpl impl_;
};

/*
 * Default StyleVisitor implementation cancels iteration immediately.
 *
 * Other StyleVisitor implementations can derive from StyleVisitorImpl
 * and just redefine the desired iteration method or methods.
 */

class StyleVisitorImpl : public StyleVisitorType {
public:
    StyleVisitorImpl();
    ~StyleVisitorImpl();

    //+ StyleVisitor::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* StyleVisitor */
    Boolean visit_alias(CharStringRef name);
    Boolean visit_attribute(StyleValueRef a);
    Boolean visit_style(StyleObjRef s);
    //+
protected:
    SharedFrescoObjectImpl object_;
};

#endif
