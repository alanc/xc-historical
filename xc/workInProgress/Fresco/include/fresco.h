/*
 * Copyright (c) 1993 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#ifndef Fresco_fresco_h
#define Fresco_fresco_h

#include <X11/Fresco/types.h>

/*
 * This include files assumes that object references are pointers,
 * so we can avoid including the interface definitions.
 */

class DisplayObj;
class DisplayObjType;
class DrawingKit;
class DrawingKitType;
class FigureKit;
class FigureKitType;
class GlyphType;
class LayoutKit;
class LayoutKitType;
class ThreadKit;
class ThreadKitType;
class ViewerType;
class WidgetKit;
class WidgetKitType;

//- Option
struct Option {
    //. An option describes a possible command-line argument in terms
    //. of a name that matches the argument string, a path that
    //. defines the root-style attribute to set, a format, and
    //. an optional implicit value.  If the format is "implicit" then
    //. the attribute will be set to the implicit value.
    //. If the format is "value" then the attribute will be set
    //. to the next argument on the command-line.  If the format
    //. is "path_value" then the next argument is a string containing
    //. the attribute path and value separated by a colon (":").

    enum Format { path_value, value, implicit };

    const char* name;
    const char* path;
    Option::Format format;
    const char* implicit_value;
};

//- Fresco
class Fresco : public BaseObjectType {
    //. Fresco is a C++ class that defines operations for accessing
    //. other Fresco objects.  It is defined as a class
    //. rather than an interface to provide support for
    //. the C++ "char*" data type and static memory management
    //. operations.  The Fresco class also provides access
    //. to non-standard kits.
    //.
    //. Typically, there will only be one Fresco object per
    //. address space that is created when the address space
    //. is initialized.  The Fresco object will hold references
    //. to kits, which in turn may hold other references.
    //. Therefore, to avoid a reference count circularity
    //. objects should not hold a reference to the Fresco object.
    //. Instead, objects can assume that the Fresco object will
    //. live at least as long as any object.
protected:
    Fresco();
    virtual ~Fresco();
public:
    //- class_name
    CharString class_name();
	//. Return the class name associated with a Fresco object.
	//. This operation is normally only needed when opening
	//. a display and initializing the display's style
	//. information.
    virtual CharStringRef _c_class_name() = 0;

    //- argc, argv
    virtual long argc() = 0;
    virtual char** argv() = 0;
	//. Return the argument list used when Fresco object
	//. was opened.  This information is normally only
	//. needed for registration with a session manager.

    //- open_display, open_default_display
    DisplayObj open_display(CharStringRef name);
    DisplayObj open_default_display();
	//. Open a display by name or open the default display.
	//. The name of the default display is platform-specific;
	//. on POSIX it can be set by the DISPLAY environment variable.
	//.
	//. Display names are of the form N:D.S, where N is
	//. an optional network host name.  If omitted, then
	//. the display is assumed to be on the local host.
	//. D is a display number on the host, and S is
	//. the number of the screen to use on the display
	//. by default.
    virtual DisplayObjType* _c_open_display(CharStringRef name) = 0;
    virtual DisplayObjType* _c_open_default_display() = 0;

    //- style
    StyleObj style();
	//. Return the root Fresco style.  This style will contain
	//. default and command-line attributes, but not display-specific
	//. or user-specific attributes.
    virtual StyleObjRef _c_style() = 0;

    //- create_request*
    RequestObj create_request(BaseObjectRef);
	//. Return a new request for dynamically invoking an operation
	//. on the given object.
    virtual RequestObjRef _c_create_request(BaseObjectRef) = 0;

    //- main*
    virtual void main(ViewerType* v, GlyphType* g) = 0;
	//. Start and run a "main loop" in the common case
	//. with a single viewer on a single display.
	//. This operation creates and maps a main viewer
	//. on the default screen of the main Fresco display
	//. (see Fresco_open or Fresco_open_display).  The main viewer
	//. uses the given viewer for input (unless the viewer is nil,
	//. in which case input is ignored) and the given glyph
	//. for output (unless the glyph is nil, in which case the
	//. viewer is used for output).  After mapping the viewer,
	//. main calls DisplayObj::run and waits for it to return.

    //- drawing_kit*
    void drawing_kit(DrawingKitType*);
    DrawingKit drawing_kit();
	//. Set or get the kit creating drawing objects such
	//. as colors and fonts.
    virtual void _c_drawing_kit(DrawingKitType*) = 0;
    virtual DrawingKitType* _c_drawing_kit() = 0;

    //- figure_kit*
    void figure_kit(FigureKitType*);
    FigureKit figure_kit();
	//. Set or get the kit for creating simple graphics figures.
    virtual void _c_figure_kit(FigureKitType*) = 0;
    virtual FigureKitType* _c_figure_kit() = 0;

    //- layout_kit*
    void layout_kit(LayoutKitType*);
    LayoutKit layout_kit();
	//. Set or get the kit for creating simple layout manager objects.
    virtual void _c_layout_kit(LayoutKitType*) = 0;
    virtual LayoutKitType* _c_layout_kit() = 0;

    //- thread_kit*
    void thread_kit(ThreadKitType*);
    ThreadKit thread_kit();
	//. Set or get the kit for creating threads and
	//. synchronization objects.
    virtual void _c_thread_kit(ThreadKitType*) = 0;
    virtual ThreadKitType* _c_thread_kit() = 0;

    //- widget_kit*
    void widget_kit(WidgetKitType*);
    WidgetKit widget_kit();
	//. Set or get the kit for creating simple widgets
	//. such as buttons, menus, and scrollbars.
    virtual void _c_widget_kit(WidgetKitType*) = 0;
    virtual WidgetKitType* _c_widget_kit() = 0;

    //- string_ref, string_copy
    static CharStringRef string_ref(const char*);
    static CharStringRef string_copy(const char*);
	//. Create a string from a null-terminated array
	//. of characters.  The string_ref operation simply
	//. references the data; the string_copy operation
	//. copies the data into its own storage that is freed
	//. when the string is deallocated.

    //- ref, unref
    static void ref(BaseObjectRef);
    static void unref(BaseObjectRef);
	//. The ref and unref operations respectively
	//. increment and decrement the reference count
	//. of a Fresco object.  If the count is not positive
	//. after an unref, then the object may be destroyed.
	//. These operations are static rather than operations
	//. on BaseObject to allow for nil references,
	//. which are ignored.
private:
    Fresco(const Fresco&);
    void operator =(const Fresco&);
};

//- Fresco_init
extern Fresco* Fresco_init(
    const char* name, int& argc, char** argv, Option* options = nil
);
    //. This function returns a Fresco object with which
    //. one can access other objects.  The name parameter
    //. specifies the name of the root style.  The argc and argv
    //. parameters should be the command-line parameters,
    //. the same as one would pass the function "main" in C.
    //. Unlike Fresco_open, this function does not open
    //. a display and does not create an initial drawing kit.
//-

//- Fresco_open, Fresco_open_display
extern Fresco* Fresco_open(
    const char* name, int& argc, char** argv, Option* options = nil
);
extern Fresco* Fresco_open_display(
    const char* display_name,
    const char* name, int& argc, char** argv, Option* options = nil
);
    //. These functions returns a Fresco object with which
    //. one can access other objects.  The name parameter
    //. specifies the name of the root style.  The argc and argv
    //. parameters should be the command-line parameters,
    //. the same as one would pass the function "main" in C.
    //.
    //. Both functions open a display and create an initial
    //. drawing kit from the display.  Fresco_open uses
    //. the default display name; Fresco_open_display uses
    //. the given display name.  Both functions return nil
    //. if they cannot successfully open the display.
//-

inline CharString Fresco::class_name() { return _c_class_name(); }
inline StyleObj Fresco::style() { return _c_style(); }
inline RequestObj Fresco::create_request(BaseObjectRef obj) {
    return _c_create_request(obj);
}

inline void Fresco::drawing_kit(DrawingKitType* k) { _c_drawing_kit(k); }
inline void Fresco::figure_kit(FigureKitType* k) { _c_figure_kit(k); }
inline void Fresco::layout_kit(LayoutKitType* k) { _c_layout_kit(k); }
inline void Fresco::thread_kit(ThreadKitType* k) { _c_thread_kit(k); }
inline void Fresco::widget_kit(WidgetKitType* k) { _c_widget_kit(k); }

#endif
