/*
 * Copyright (c) 1992-1993 Silicon Graphics, Inc.
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

#ifndef Fresco_Impls_fresco_impl_h
#define Fresco_Impls_fresco_impl_h

#include <X11/Fresco/fresco.h>
#include <X11/Fresco/Impls/fobjects.h>

class FrescoImpl : public Fresco {
public:
    FrescoImpl(const char* name, int& argc, char** argv, Option*);
    ~FrescoImpl();

    //+ FrescoObject::=
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    //+

    CharStringRef _c_class_name();
    long argc();
    char** argv();

    DisplayObjType* _c_open_display(CharStringRef name);
    DisplayObjType* _c_open_default_display();

    StyleObjRef _c_style();
    RequestObjRef _c_create_request(BaseObjectRef);

    void main(ViewerType*, GlyphType*);

    void _c_drawing_kit(DrawingKitType*);
    DrawingKitType* _c_drawing_kit();

    void _c_figure_kit(FigureKitType*);
    FigureKitType* _c_figure_kit();

    void _c_layout_kit(LayoutKitType*);
    LayoutKitType* _c_layout_kit();

    void _c_thread_kit(ThreadKitType*);
    ThreadKitType* _c_thread_kit();

    void _c_widget_kit(WidgetKitType*);
    WidgetKitType* _c_widget_kit();

    void connect(DisplayObjType*);
protected:
    SharedFrescoObjectImpl object_;
    CharStringRef class_name_;
    long argc_;
    char** argv_;
    DisplayObjType* display_;
    StyleObjRef style_;
    DrawingKitType* drawing_kit_;
    FigureKitType* figure_kit_;
    LayoutKitType* layout_kit_;
    ThreadKitType* thread_kit_;
    WidgetKitType* widget_kit_;

    void create_root_style(
	const char* name, int& argc, char** argv, Option* options
    );
    void find_name(int argc, char** argv);
    void parse_args(int& argc, char** argv, Option* options);
    void extract(Option*, int& i, int argc, char** argv);
    const char* next_arg(int& i, int argc, char** argv, const char* message);
    void bad_arg(const char* message);

    FigureKitType* create_figure_kit();
    LayoutKitType* create_layout_kit();
    ThreadKitType* create_thread_kit();
    WidgetKitType* create_widget_kit();
    void init_types();
};

#endif
