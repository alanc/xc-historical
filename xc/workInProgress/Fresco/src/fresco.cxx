/*
 * $XConsortium$
 */

/*
 * Copyright (c) 1992-93 Silicon Graphics, Inc.
 * Copyright (c) 1993 Fujitsu, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics and Fujitsu may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics and Fujitsu.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL SILICON GRAPHICS OR FUJITSU BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/glyph.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/charstr.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/fresco-impl.h>
#include <X11/Fresco/Impls/styles.h>
#include <X11/Fresco/Impls/viewers.h>
#include <X11/Fresco/OS/list.h>
#include <X11/Fresco/OS/threads.h>
#include <X11/Fresco/Ox/request.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(sgi)
#include <malloc.h>
#endif

/*
 * Standard command-line options.
 */

static Option standard_options[] = {
    { "-background", "*background", Option::value },
    { "-bg", "*background", Option::value },
    { "-dbuf", "*double_buffered", Option::implicit, "on" },
    { "-display", "*display", Option::value },
    { "-dpi", "*dpi", Option::value },
    { "-fg", "*foreground", Option::value },
    { "-flat", "*flat", Option::value },
    { "-fn", "*font", Option::value },
    { "-font", "*font", Option::value },
    { "-foreground", "*foreground", Option::value },
    { "-geometry", "*geometry", Option::value },
    { "-iconic", "*iconic", Option::implicit, "on" },
    { "-monochrome", "*gui", Option::implicit, "monochrome" },
    { "-motif", "*gui", Option::implicit, "Motif" },
    { "-mthread", "*single_threaded", Option::implicit, "off" },
    { "-name", "*name", Option::value },
    { "-nodbuf", "*double_buffered", Option::implicit, "off" },
    { "-sthread", "*single_threaded", Option::implicit, "on" },
    { "-smotif", "*gui", Option::implicit, "SGIMotif" },
    { "-synchronous", "*synchronous", Option::implicit, "on" },
    { "+synchronous", "*synchronous", Option::implicit, "off" },
    { "-title", "*title", Option::value },
    { "-visual", "*visual", Option::value },
    { "-visual_id", "*visual_id", Option::value },
    { "-xrm", nil, Option::path_value },
#ifdef sgi
    { "-malloc", "*malloc_debug", Option::implicit, "on" },
#endif
    { nil }
};

/*
 * Default root style attributes.
 */

static char* standard_attributes[] = {
    "foreground", "black",
    "background", "white",
    "font", "fixed",
    "double_buffered", "on",
    nil,
};

/*
 * Global functions for bootstrapping.
 */

Fresco* Fresco_open(
    const char* name, int& argc, char** argv, Option* options
) {
    FrescoImpl* f = new FrescoImpl(name, argc, argv, options);
    f->connect(f->_c_open_default_display());
    return f;
}

Fresco* Fresco_open_display(
    const char* display_name,
    const char* name, int& argc, char** argv, Option* options
) {
    FrescoImpl* f = new FrescoImpl(name, argc, argv, options);
    f->connect(f->_c_open_display(Fresco::string_ref(display_name)));
    return f;
}

Fresco* Fresco_init(
    const char* name, int& argc, char** argv, Option* options
) {
    return new FrescoImpl(name, argc, argv, options);
}

/* class SharedFrescoObjectImpl */

struct ObservableInfo {
    long tag;
    FrescoObjectRef observer;
};

declareList(ObserverList,ObservableInfo)
implementList(ObserverList,ObservableInfo)

SharedFrescoObjectImpl::SharedFrescoObjectImpl() {
    observers_ = nil;
}

SharedFrescoObjectImpl::~SharedFrescoObjectImpl() {
    ObserverList* list = observers_;
    if (list != nil) {
	ListItr(ObserverList) i(*list);
	/*
	 * First element is just for tag counter
	 */
	i.next();
	for (; i.more(); i.next()) {
	    i.cur().observer->disconnect();
	}
	delete list;
    }
}

/*
 * Observable operations.
 *
 * Note that the first "observer" on the list is really a dummy
 * to hold the counter for the object.  This way we pay an extra
 * word for observables, but one less word for objects that are
 * never observed.
 */

//+ SharedFrescoObjectImpl(FrescoObject::attach)
Tag SharedFrescoObjectImpl::attach(FrescoObjectRef observer) {
    ObservableInfo i;
    if (observers_ == nil) {
	observers_ = new ObserverList;
	i.tag = 0;
	i.observer = nil;
	observers_->append(i);
    }
    ObservableInfo& hdr = observers_->item_ref(0);
    hdr.tag += 1;
    i.tag = hdr.tag;
    i.observer = observer;
    observers_->append(i);
    return i.tag;
}

/*
 * Detach the observer corresponding to the given tag.
 * Probably should raise an exception or otherwise
 * return an error if the tag is not found, but for now
 * we just silently ignore.
 */

//+ SharedFrescoObjectImpl(FrescoObject::detach)
void SharedFrescoObjectImpl::detach(Tag attach_tag) {
    if (observers_ != nil) {
	ListUpdater(ObserverList) i(*observers_);
	for (i.next(); i.more(); i.next()) {
	    if (i.cur().tag == attach_tag) {
		i.remove_cur();
		break;
	    }
	}
    }
}

//+ SharedFrescoObjectImpl(FrescoObject::notify_observers)
void SharedFrescoObjectImpl::notify_observers() {
    if (observers_ != nil) {
	ListUpdater(ObserverList) i(*observers_);
	for (i.next(); i.more(); i.next()) {
	    i.cur().observer->update();
	}
    }
}

/* class LockedFrescoObjectImpl */

//+ LockedFrescoObjectImpl(FrescoObject::ref__)
Long LockedFrescoObjectImpl::ref__(Long references) {
    lock_->acquire();
    long n = SharedFrescoObjectImpl::ref__(references);
    lock_->release();
    return n;
}

//+ LockedFrescoObjectImpl(FrescoObject::attach)
Tag LockedFrescoObjectImpl::attach(FrescoObjectRef observer) {
    lock_->acquire();
    Tag t = SharedFrescoObjectImpl::attach(observer);
    lock_->release();
    return t;
}

//+ LockedFrescoObjectImpl(FrescoObject::detach)
void LockedFrescoObjectImpl::detach(Tag attach_tag) {
    lock_->acquire();
    SharedFrescoObjectImpl::detach(attach_tag);
    lock_->release();
}

//+ LockedFrescoObjectImpl(FrescoObject::notify_observers)
void LockedFrescoObjectImpl::notify_observers() {
    lock_->acquire();
    SharedFrescoObjectImpl::notify_observers();
    lock_->release();
}

/* class Fresco */

Fresco::Fresco() { }
Fresco::~Fresco() { }

CharStringRef Fresco::string_ref(const char* ptr) {
    return new CharStringImpl(ptr);
}

CharStringRef Fresco::string_copy(const char* ptr) {
    return new CharStringImpl(ptr);
}

void Fresco::ref(BaseObjectRef r) {
    if (is_not_nil(r)) {
	r->ref__(+1);
    }
}

void Fresco::unref(BaseObjectRef r) {
    if (is_not_nil(r) && r->ref__(-1) <= 0) {
	delete r;
    }
}

/* class FrescoImpl */

FrescoImpl::FrescoImpl(
    const char* name, int& argc, char** argv, Option* options
) {
    class_name_ = new CharStringImpl(name);
    argc_ = argc;
    argv_ = argv;
    display_ = nil;
    style_ = nil;
    drawing_kit_ = nil;
    figure_kit_ = nil;
    layout_kit_ = nil;
    thread_kit_ = nil;
    widget_kit_ = nil;

    /*
     * Make sure the root style is created after the kit references
     * are initialized to nil because creating the root style might
     * (probably will) cause a kit to be created.
     */
    create_root_style(name, argc, argv, options);

    init_types();
#if defined(sgi)
    if (style_->is_on(Fresco::string_ref("malloc_debug"))) {
	mallopt(M_DEBUG, 1);
    }
#endif
}

FrescoImpl::~FrescoImpl() {
    Fresco::unref(class_name_);
    Fresco::unref(style_);
    Fresco::unref(drawing_kit_);
    Fresco::unref(figure_kit_);
    Fresco::unref(layout_kit_);
    Fresco::unref(thread_kit_);
    Fresco::unref(widget_kit_);
    Fresco::unref(display_);
}

//+ FrescoImpl(FrescoObject::=object_.)
Long FrescoImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag FrescoImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void FrescoImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void FrescoImpl::disconnect() {
    object_.disconnect();
}
void FrescoImpl::notify_observers() {
    object_.notify_observers();
}
void FrescoImpl::update() {
    object_.update();
}
//+

CharStringRef FrescoImpl::_c_class_name() {
    return CharString::_duplicate(class_name_);
}

long FrescoImpl::argc() { return argc_; }
char** FrescoImpl::argv() { return argv_; }

StyleObjRef FrescoImpl::_c_style() {
    return StyleObj::_duplicate(style_);
}

RequestObjRef FrescoImpl::_c_create_request(BaseObjectRef obj) {
    return new RequestObjImpl(obj);
}

void FrescoImpl::main(ViewerRef v, GlyphRef g) {
    ScreenObj s = display_->default_screen();
    ViewerRef nv;
    if (is_nil(g)) {
	nv = v;
    } else {
	nv = new ViewerImpl(this);
	nv->body(g);
	if (is_not_nil(v)) {
	    nv->append_viewer(v);
	}
    }
    Window w = s->application(nv);
    w->map();
    display_->run(true);
}

void FrescoImpl::_c_drawing_kit(DrawingKitRef k) {
    Fresco::unref(drawing_kit_);
    drawing_kit_ = DrawingKit::_duplicate(k);
}

DrawingKit Fresco::drawing_kit() { return _c_drawing_kit(); }

DrawingKitRef FrescoImpl::_c_drawing_kit() {
    return DrawingKit::_duplicate(drawing_kit_);
}

void FrescoImpl::_c_figure_kit(FigureKitRef k) {
    Fresco::unref(figure_kit_);
    figure_kit_ = FigureKit::_duplicate(k);
}

FigureKit Fresco::figure_kit() { return _c_figure_kit(); }

FigureKitRef FrescoImpl::_c_figure_kit() {
    if (figure_kit_ == nil) {
	figure_kit_ = create_figure_kit();
    }
    return FigureKit::_duplicate(figure_kit_);
}

void FrescoImpl::_c_layout_kit(LayoutKitRef k) {
    Fresco::unref(layout_kit_);
    layout_kit_ = LayoutKit::_duplicate(k);
}

LayoutKit Fresco::layout_kit() { return _c_layout_kit(); }

LayoutKitRef FrescoImpl::_c_layout_kit() {
    if (layout_kit_ == nil) {
	layout_kit_ = create_layout_kit();
    }
    return LayoutKit::_duplicate(layout_kit_);
}

void FrescoImpl::_c_thread_kit(ThreadKitRef k) {
    Fresco::unref(thread_kit_);
    thread_kit_ = ThreadKit::_duplicate(k);
}

ThreadKit Fresco::thread_kit() { return _c_thread_kit(); }

ThreadKitRef FrescoImpl::_c_thread_kit() {
    if (thread_kit_ == nil) {
	thread_kit_ = create_thread_kit();
    }
    return ThreadKit::_duplicate(thread_kit_);
}

void FrescoImpl::_c_widget_kit(WidgetKitRef k) {
    Fresco::unref(widget_kit_);
    widget_kit_ = WidgetKit::_duplicate(k);
}

WidgetKit Fresco::widget_kit() { return _c_widget_kit(); }

WidgetKitRef FrescoImpl::_c_widget_kit() {
    if (widget_kit_ == nil) {
	widget_kit_ = create_widget_kit();
    }
    return WidgetKit::_duplicate(widget_kit_);
}

void FrescoImpl::connect(DisplayObjRef d) {
    if (d == nil) {
	fprintf(stderr, "Can't open DISPLAY\n");
	exit(1);
    }
    display_ = d;
    drawing_kit_ = d->_c_drawing_kit();
}

void FrescoImpl::create_root_style(
    const char* name, int& argc, char** argv, Option* options
) {
    style_ = new StyleImpl(this);

    style_->alias(Fresco::string_ref(name));
    find_name(argc, argv);

    for (char** attr = standard_attributes; *attr != nil; attr += 2) {
	StyleValue a = style_->bind(new CharStringImpl(attr[0]));
	a->write_string(new CharStringImpl(attr[1]));
	a->priority(-10);
    }
    if (options != nil) {
	parse_args(argc, argv, options);
    }
    parse_args(argc, argv, standard_options);
}

/*
 * Figure out the name for the root style.  If -name is specified
 * on the command line then we use it.  Otherwise, we have to check
 * for the RESOURCE_NAME environment variable, or finally try
 * to use argv[0].
 */

void FrescoImpl::find_name(int argc, char** argv) {
    const char* value = nil;
    for (int i = 1; i < argc - 1; i++) {
	if (strcmp(argv[i], "-name") == 0) {
	    value = argv[i + 1];
	    break;
	}
    }
    if (value == nil) {
	value = getenv("RESOURCE_NAME");
	if (value == nil) {
	    if (argc > 0) {
		value = argv[0];
		const char* p = value + strlen(value) - 1;
		for (; p >= value; p--) {
		    if (*p == '/') {
			value = p + 1;
			break;
		    }
		}
	    } else {
		value = "Anon-7";
	    }
	}
    }
    style_->name(Fresco::string_ref(value));
}

/*
 * Parse an argument list, setting any attributes that are specified
 * in the option list.  Matching arguments are removed (in-place)
 * from the argument list.
 */

void FrescoImpl::parse_args(int& argc, char** argv, Option* options) {
    int i;
    int newargc = 1;
    char* newargv[1024];
    newargv[0] = argv[0];
    for (i = 1; i < argc; i++) {
	Boolean matched = false;
	const char* s = argv[i];
	for (Option* o = options; o->name != nil; o++) {
	    if (strcmp(s, o->name) == 0) {
		extract(o, i, argc, argv);
		matched = true;
		break;
	    }
	}
	if (!matched) {
	    if (newargc != i) {
		argv[newargc] = argv[i];
	    }
	    ++newargc;
	}
    }
    if (newargc != argc) {
	argc = newargc;
	argv[argc] = nil;
    }
}

/*
 * Extract an attribute definition from an argument.
 */

void FrescoImpl::extract(Option* o, int& i, int argc, char** argv) {
    const char* name;
    Long name_length;
    const char* value;
    Long value_length;
    const char* colon;
    switch (o->format) {
    case Option::path_value:
	value  = next_arg(i, argc, argv, "Missing attribute definition");
	value_length = strlen(value);
	colon = CharStringImpl::index(value, value_length, ':');
	if (colon == nil) {
	    bad_arg("Missing ':' in attribute definition");
	} else {
	    name = value;
	    name_length = colon - value;
	    value = colon + 1;
	    value_length -= (name_length + 1);
	}
	break;
    case Option::value:
	name = o->path;
	name_length = strlen(name);
	value = next_arg(i, argc, argv, "Missing value");
	value_length = strlen(value);
	break;
    case Option::implicit:
	name = o->path;
	name_length = strlen(name);
	value = o->implicit_value;
	value_length = strlen(value);
	break;
    }
    StyleValue a = style_->bind(new CharStringImpl(name, name_length));
    a->write_string(new CharStringImpl(value, value_length));
    a->priority(0);
}

const char* FrescoImpl::next_arg(
    int& i, int argc, char** argv, const char* message
) {
    ++i;
    if (i == argc) {
	bad_arg(message);
    }
    return argv[i];
}

void FrescoImpl::bad_arg(const char* message) {
    fflush(stdout);
    fprintf(stderr, "%s\n", message);
    exit(1);
}

ThreadKitRef FrescoImpl::create_thread_kit() {
    return ThreadKit_implementation();
}
