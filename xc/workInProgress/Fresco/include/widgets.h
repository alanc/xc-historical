/* DO NOT EDIT -- Automatically generated from Interfaces/widgets.idl */

#ifndef Interfaces_widgets_h
#define Interfaces_widgets_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/viewer.h>

class AdjustmentType;
typedef AdjustmentType* AdjustmentRef;
typedef AdjustmentRef Adjustment_in;
class Adjustment;
class Adjustment_tmp;
class Adjustment_var;

class Adjustment {
public:
    AdjustmentRef _obj_;

    Adjustment() { _obj_ = 0; }
    Adjustment(AdjustmentRef p) { _obj_ = p; }
    Adjustment& operator =(AdjustmentRef p);
    Adjustment(const Adjustment&);
    Adjustment& operator =(const Adjustment& r);
    Adjustment(const Adjustment_tmp&);
    Adjustment& operator =(const Adjustment_tmp&);
    Adjustment(const Adjustment_var&);
    Adjustment& operator =(const Adjustment_var&);
    ~Adjustment();

    AdjustmentRef operator ->() { return _obj_; }

    operator Adjustment_in() const { return _obj_; }
    operator FrescoObject() const;
    static AdjustmentRef _narrow(BaseObjectRef p);
    static Adjustment_tmp _narrow(const BaseObject& r);

    static AdjustmentRef _duplicate(AdjustmentRef obj);
    static Adjustment_tmp _duplicate(const Adjustment& r);
    struct Settings {
        Coord lower, upper, length;
        Coord cur_lower, cur_upper, cur_length;
    };
};

class Adjustment_tmp : public Adjustment {
public:
    Adjustment_tmp(AdjustmentRef p) { _obj_ = p; }
    Adjustment_tmp(const Adjustment& r);
    Adjustment_tmp(const Adjustment_tmp& r);
    ~Adjustment_tmp();
};

class Adjustment_var {
public:
    AdjustmentRef _obj_;

    Adjustment_var(AdjustmentRef p) { _obj_ = p; }
    operator AdjustmentRef() const { return _obj_; }
    AdjustmentRef operator ->() { return _obj_; }
};

class AdjustmentType : public FrescoObjectType {
protected:
    AdjustmentType();
    virtual ~AdjustmentType();
public:
    virtual Coord small_scroll();
    virtual void small_scroll(Coord _p);
    virtual Coord large_scroll();
    virtual void large_scroll(Coord _p);
    virtual void get_settings(Adjustment::Settings& s);
    virtual void begin();
    virtual void commit();
    virtual void cancel();
    virtual void scroll_forward();
    virtual void scroll_backward();
    virtual void page_forward();
    virtual void page_backward();
    virtual void scroll_to(Coord lower);
    virtual void scroll_by(Coord delta);
    virtual void scale_to(Coord length);
    virtual void constrain(Coord& c);
    AdjustmentRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class AdjustmentStub : public AdjustmentType {
public:
    AdjustmentStub(Exchange*);
    ~AdjustmentStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class TelltaleType;
typedef TelltaleType* TelltaleRef;
typedef TelltaleRef Telltale_in;
class Telltale;
class Telltale_tmp;
class Telltale_var;

class ButtonType;
typedef ButtonType* ButtonRef;
typedef ButtonRef Button_in;
class Button;
class Button_tmp;
class Button_var;

class Button {
public:
    ButtonRef _obj_;

    Button() { _obj_ = 0; }
    Button(ButtonRef p) { _obj_ = p; }
    Button& operator =(ButtonRef p);
    Button(const Button&);
    Button& operator =(const Button& r);
    Button(const Button_tmp&);
    Button& operator =(const Button_tmp&);
    Button(const Button_var&);
    Button& operator =(const Button_var&);
    ~Button();

    ButtonRef operator ->() { return _obj_; }

    operator Button_in() const { return _obj_; }
    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static ButtonRef _narrow(BaseObjectRef p);
    static Button_tmp _narrow(const BaseObject& r);

    static ButtonRef _duplicate(ButtonRef obj);
    static Button_tmp _duplicate(const Button& r);
};

class Button_tmp : public Button {
public:
    Button_tmp(ButtonRef p) { _obj_ = p; }
    Button_tmp(const Button& r);
    Button_tmp(const Button_tmp& r);
    ~Button_tmp();
};

class Button_var {
public:
    ButtonRef _obj_;

    Button_var(ButtonRef p) { _obj_ = p; }
    operator ButtonRef() const { return _obj_; }
    ButtonRef operator ->() { return _obj_; }
};

class ButtonType : public ViewerType {
protected:
    ButtonType();
    virtual ~ButtonType();
public:
    Telltale_tmp state();
    virtual TelltaleRef _c_state();
    void state(Telltale_in _p);
    virtual void _c_state(Telltale_in _p);
    Action_tmp click_action() {
        return _c_click_action();
    }
    virtual ActionRef _c_click_action();
    void click_action(Action_in _p) {
        _c_click_action(_p);
    }
    virtual void _c_click_action(Action_in _p);
    ButtonRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ButtonStub : public ButtonType {
public:
    ButtonStub(Exchange*);
    ~ButtonStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class Telltale {
public:
    TelltaleRef _obj_;

    Telltale() { _obj_ = 0; }
    Telltale(TelltaleRef p) { _obj_ = p; }
    Telltale& operator =(TelltaleRef p);
    Telltale(const Telltale&);
    Telltale& operator =(const Telltale& r);
    Telltale(const Telltale_tmp&);
    Telltale& operator =(const Telltale_tmp&);
    Telltale(const Telltale_var&);
    Telltale& operator =(const Telltale_var&);
    ~Telltale();

    TelltaleRef operator ->() { return _obj_; }

    operator Telltale_in() const { return _obj_; }
    operator FrescoObject() const;
    static TelltaleRef _narrow(BaseObjectRef p);
    static Telltale_tmp _narrow(const BaseObject& r);

    static TelltaleRef _duplicate(TelltaleRef obj);
    static Telltale_tmp _duplicate(const Telltale& r);
    enum Flag {
        enabled, visible, active, chosen, running, stepping, choosable, 
        toggle
    };
};

class Telltale_tmp : public Telltale {
public:
    Telltale_tmp(TelltaleRef p) { _obj_ = p; }
    Telltale_tmp(const Telltale& r);
    Telltale_tmp(const Telltale_tmp& r);
    ~Telltale_tmp();
};

class Telltale_var {
public:
    TelltaleRef _obj_;

    Telltale_var(TelltaleRef p) { _obj_ = p; }
    operator TelltaleRef() const { return _obj_; }
    TelltaleRef operator ->() { return _obj_; }
};

class TelltaleType : public FrescoObjectType {
protected:
    TelltaleType();
    virtual ~TelltaleType();
public:
    virtual void set(Telltale::Flag f);
    virtual void clear(Telltale::Flag f);
    virtual Boolean test(Telltale::Flag f);
    Telltale_tmp current() {
        return _c_current();
    }
    virtual TelltaleRef _c_current();
    void current(Telltale_in _p) {
        _c_current(_p);
    }
    virtual void _c_current(Telltale_in _p);
    TelltaleRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class TelltaleStub : public TelltaleType {
public:
    TelltaleStub(Exchange*);
    ~TelltaleStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class MenuType;
typedef MenuType* MenuRef;
typedef MenuRef Menu_in;
class Menu;
class Menu_tmp;
class Menu_var;

class MenuItemType;
typedef MenuItemType* MenuItemRef;
typedef MenuItemRef MenuItem_in;
class MenuItem;
class MenuItem_tmp;
class MenuItem_var;

class MenuItem {
public:
    MenuItemRef _obj_;

    MenuItem() { _obj_ = 0; }
    MenuItem(MenuItemRef p) { _obj_ = p; }
    MenuItem& operator =(MenuItemRef p);
    MenuItem(const MenuItem&);
    MenuItem& operator =(const MenuItem& r);
    MenuItem(const MenuItem_tmp&);
    MenuItem& operator =(const MenuItem_tmp&);
    MenuItem(const MenuItem_var&);
    MenuItem& operator =(const MenuItem_var&);
    ~MenuItem();

    MenuItemRef operator ->() { return _obj_; }

    operator MenuItem_in() const { return _obj_; }
    operator Button() const;
    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static MenuItemRef _narrow(BaseObjectRef p);
    static MenuItem_tmp _narrow(const BaseObject& r);

    static MenuItemRef _duplicate(MenuItemRef obj);
    static MenuItem_tmp _duplicate(const MenuItem& r);
};

class MenuItem_tmp : public MenuItem {
public:
    MenuItem_tmp(MenuItemRef p) { _obj_ = p; }
    MenuItem_tmp(const MenuItem& r);
    MenuItem_tmp(const MenuItem_tmp& r);
    ~MenuItem_tmp();
};

class MenuItem_var {
public:
    MenuItemRef _obj_;

    MenuItem_var(MenuItemRef p) { _obj_ = p; }
    operator MenuItemRef() const { return _obj_; }
    MenuItemRef operator ->() { return _obj_; }
};

class MenuItemType : public ButtonType {
protected:
    MenuItemType();
    virtual ~MenuItemType();
public:
    Menu_tmp submenu();
    virtual MenuRef _c_submenu();
    void submenu(Menu_in _p);
    virtual void _c_submenu(Menu_in _p);
    MenuItem_tmp next_menu() {
        return _c_next_menu();
    }
    virtual MenuItemRef _c_next_menu();
    MenuItem_tmp prev_menu() {
        return _c_prev_menu();
    }
    virtual MenuItemRef _c_prev_menu();
    virtual void insert_menu_before(MenuItem_in i);
    virtual void insert_menu_after(MenuItem_in i);
    virtual void replace_menu(MenuItem_in i);
    virtual void remove_menu();
    MenuItemRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class MenuItemStub : public MenuItemType {
public:
    MenuItemStub(Exchange*);
    ~MenuItemStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class Menu {
public:
    MenuRef _obj_;

    Menu() { _obj_ = 0; }
    Menu(MenuRef p) { _obj_ = p; }
    Menu& operator =(MenuRef p);
    Menu(const Menu&);
    Menu& operator =(const Menu& r);
    Menu(const Menu_tmp&);
    Menu& operator =(const Menu_tmp&);
    Menu(const Menu_var&);
    Menu& operator =(const Menu_var&);
    ~Menu();

    MenuRef operator ->() { return _obj_; }

    operator Menu_in() const { return _obj_; }
    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static MenuRef _narrow(BaseObjectRef p);
    static Menu_tmp _narrow(const BaseObject& r);

    static MenuRef _duplicate(MenuRef obj);
    static Menu_tmp _duplicate(const Menu& r);
};

class Menu_tmp : public Menu {
public:
    Menu_tmp(MenuRef p) { _obj_ = p; }
    Menu_tmp(const Menu& r);
    Menu_tmp(const Menu_tmp& r);
    ~Menu_tmp();
};

class Menu_var {
public:
    MenuRef _obj_;

    Menu_var(MenuRef p) { _obj_ = p; }
    operator MenuRef() const { return _obj_; }
    MenuRef operator ->() { return _obj_; }
};

class MenuType : public ViewerType {
protected:
    MenuType();
    virtual ~MenuType();
public:
    virtual void append_menu_item(MenuItem_in i);
    virtual void prepend_menu_item(MenuItem_in i);
    MenuItem_tmp first_menu_item() {
        return _c_first_menu_item();
    }
    virtual MenuItemRef _c_first_menu_item();
    MenuItem_tmp last_menu_item() {
        return _c_last_menu_item();
    }
    virtual MenuItemRef _c_last_menu_item();
    MenuItem_tmp selected() {
        return _c_selected();
    }
    virtual MenuItemRef _c_selected();
    void selected(MenuItem_in _p) {
        _c_selected(_p);
    }
    virtual void _c_selected(MenuItem_in _p);
    MenuRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class MenuStub : public MenuType {
public:
    MenuStub(Exchange*);
    ~MenuStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class WidgetKitType;
typedef WidgetKitType* WidgetKitRef;
typedef WidgetKitRef WidgetKit_in;
class WidgetKit;
class WidgetKit_tmp;
class WidgetKit_var;

class WidgetKit {
public:
    WidgetKitRef _obj_;

    WidgetKit() { _obj_ = 0; }
    WidgetKit(WidgetKitRef p) { _obj_ = p; }
    WidgetKit& operator =(WidgetKitRef p);
    WidgetKit(const WidgetKit&);
    WidgetKit& operator =(const WidgetKit& r);
    WidgetKit(const WidgetKit_tmp&);
    WidgetKit& operator =(const WidgetKit_tmp&);
    WidgetKit(const WidgetKit_var&);
    WidgetKit& operator =(const WidgetKit_var&);
    ~WidgetKit();

    WidgetKitRef operator ->() { return _obj_; }

    operator WidgetKit_in() const { return _obj_; }
    operator FrescoObject() const;
    static WidgetKitRef _narrow(BaseObjectRef p);
    static WidgetKit_tmp _narrow(const BaseObject& r);

    static WidgetKitRef _duplicate(WidgetKitRef obj);
    static WidgetKit_tmp _duplicate(const WidgetKit& r);
};

class WidgetKit_tmp : public WidgetKit {
public:
    WidgetKit_tmp(WidgetKitRef p) { _obj_ = p; }
    WidgetKit_tmp(const WidgetKit& r);
    WidgetKit_tmp(const WidgetKit_tmp& r);
    ~WidgetKit_tmp();
};

class WidgetKit_var {
public:
    WidgetKitRef _obj_;

    WidgetKit_var(WidgetKitRef p) { _obj_ = p; }
    operator WidgetKitRef() const { return _obj_; }
    WidgetKitRef operator ->() { return _obj_; }
};

class WidgetKitType : public FrescoObjectType {
protected:
    WidgetKitType();
    virtual ~WidgetKitType();
public:
    Glyph_tmp inset_frame(Glyph_in g) {
        return _c_inset_frame(g);
    }
    virtual GlyphRef _c_inset_frame(Glyph_in g);
    Glyph_tmp outset_frame(Glyph_in g) {
        return _c_outset_frame(g);
    }
    virtual GlyphRef _c_outset_frame(Glyph_in g);
    Glyph_tmp bright_inset_frame(Glyph_in g) {
        return _c_bright_inset_frame(g);
    }
    virtual GlyphRef _c_bright_inset_frame(Glyph_in g);
    Viewer_tmp label(CharString_in s) {
        return _c_label(s);
    }
    virtual ViewerRef _c_label(CharString_in s);
    Menu_tmp menubar() {
        return _c_menubar();
    }
    virtual MenuRef _c_menubar();
    Menu_tmp pulldown() {
        return _c_pulldown();
    }
    virtual MenuRef _c_pulldown();
    Menu_tmp pullright() {
        return _c_pullright();
    }
    virtual MenuRef _c_pullright();
    MenuItem_tmp menubar_item(Glyph_in g) {
        return _c_menubar_item(g);
    }
    virtual MenuItemRef _c_menubar_item(Glyph_in g);
    MenuItem_tmp menu_item(Glyph_in g) {
        return _c_menu_item(g);
    }
    virtual MenuItemRef _c_menu_item(Glyph_in g);
    MenuItem_tmp check_menu_item(Glyph_in g) {
        return _c_check_menu_item(g);
    }
    virtual MenuItemRef _c_check_menu_item(Glyph_in g);
    MenuItem_tmp radio_menu_item(Glyph_in g, Telltale_in group) {
        return _c_radio_menu_item(g, group);
    }
    virtual MenuItemRef _c_radio_menu_item(Glyph_in g, Telltale_in group);
    MenuItem_tmp menu_item_separator() {
        return _c_menu_item_separator();
    }
    virtual MenuItemRef _c_menu_item_separator();
    Telltale_tmp telltale_group() {
        return _c_telltale_group();
    }
    virtual TelltaleRef _c_telltale_group();
    Button_tmp push_button(Glyph_in g, Action_in a) {
        return _c_push_button(g, a);
    }
    virtual ButtonRef _c_push_button(Glyph_in g, Action_in a);
    Button_tmp default_button(Glyph_in g, Action_in a) {
        return _c_default_button(g, a);
    }
    virtual ButtonRef _c_default_button(Glyph_in g, Action_in a);
    Button_tmp palette_button(Glyph_in g, Action_in a) {
        return _c_palette_button(g, a);
    }
    virtual ButtonRef _c_palette_button(Glyph_in g, Action_in a);
    Button_tmp check_box(Glyph_in g, Action_in a) {
        return _c_check_box(g, a);
    }
    virtual ButtonRef _c_check_box(Glyph_in g, Action_in a);
    Button_tmp radio_button(Glyph_in g, Action_in a, Telltale_in group) {
        return _c_radio_button(g, a, group);
    }
    virtual ButtonRef _c_radio_button(Glyph_in g, Action_in a, Telltale_in group);
    Viewer_tmp slider(Axis a, Adjustment_in adj) {
        return _c_slider(a, adj);
    }
    virtual ViewerRef _c_slider(Axis a, Adjustment_in adj);
    Viewer_tmp scroll_bar(Axis a, Adjustment_in adj) {
        return _c_scroll_bar(a, adj);
    }
    virtual ViewerRef _c_scroll_bar(Axis a, Adjustment_in adj);
    Viewer_tmp panner(Adjustment_in x, Adjustment_in y) {
        return _c_panner(x, y);
    }
    virtual ViewerRef _c_panner(Adjustment_in x, Adjustment_in y);
    Button_tmp zoomer(Coord scale, Adjustment_in x, Adjustment_in y, Adjustment_in z) {
        return _c_zoomer(scale, x, y, z);
    }
    virtual ButtonRef _c_zoomer(Coord scale, Adjustment_in x, Adjustment_in y, Adjustment_in z);
    Button_tmp up_mover(Adjustment_in a) {
        return _c_up_mover(a);
    }
    virtual ButtonRef _c_up_mover(Adjustment_in a);
    Button_tmp down_mover(Adjustment_in a) {
        return _c_down_mover(a);
    }
    virtual ButtonRef _c_down_mover(Adjustment_in a);
    Button_tmp left_mover(Adjustment_in a) {
        return _c_left_mover(a);
    }
    virtual ButtonRef _c_left_mover(Adjustment_in a);
    Button_tmp right_mover(Adjustment_in a) {
        return _c_right_mover(a);
    }
    virtual ButtonRef _c_right_mover(Adjustment_in a);
    WidgetKitRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class WidgetKitStub : public WidgetKitType {
public:
    WidgetKitStub(Exchange*);
    ~WidgetKitStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline AdjustmentRef Adjustment::_duplicate(AdjustmentRef obj) {
    return (AdjustmentRef)_BaseObject__duplicate(obj, &AdjustmentStub::_create);
}
inline Adjustment& Adjustment::operator =(AdjustmentRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Adjustment::_duplicate(p);
    return *this;
}
inline Adjustment::Adjustment(const Adjustment& r) {
    _obj_ = Adjustment::_duplicate(r._obj_);
}
inline Adjustment& Adjustment::operator =(const Adjustment& r) {
    _BaseObject__release(_obj_);
    _obj_ = Adjustment::_duplicate(r._obj_);
    return *this;
}
inline Adjustment::Adjustment(const Adjustment_tmp& r) {
    _obj_ = r._obj_;
    ((Adjustment_tmp*)&r)->_obj_ = 0;
}
inline Adjustment& Adjustment::operator =(const Adjustment_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Adjustment_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Adjustment::Adjustment(const Adjustment_var& e) {
    _obj_ = Adjustment::_duplicate(e._obj_);
}
inline Adjustment& Adjustment::operator =(const Adjustment_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Adjustment::_duplicate(e._obj_);
    return *this;
}
inline Adjustment::~Adjustment() {
    _BaseObject__release(_obj_);
}
inline Adjustment_tmp Adjustment::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Adjustment_tmp Adjustment::_duplicate(const Adjustment& r) {
    return _duplicate(r._obj_);
}
inline Adjustment::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Adjustment_tmp::Adjustment_tmp(const Adjustment& r) {
    _obj_ = Adjustment::_duplicate(r._obj_);
}
inline Adjustment_tmp::Adjustment_tmp(const Adjustment_tmp& r) {
    _obj_ = r._obj_;
    ((Adjustment_tmp*)&r)->_obj_ = 0;
}
inline Adjustment_tmp::~Adjustment_tmp() { }

inline ButtonRef Button::_duplicate(ButtonRef obj) {
    return (ButtonRef)_BaseObject__duplicate(obj, &ButtonStub::_create);
}
inline Button& Button::operator =(ButtonRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Button::_duplicate(p);
    return *this;
}
inline Button::Button(const Button& r) {
    _obj_ = Button::_duplicate(r._obj_);
}
inline Button& Button::operator =(const Button& r) {
    _BaseObject__release(_obj_);
    _obj_ = Button::_duplicate(r._obj_);
    return *this;
}
inline Button::Button(const Button_tmp& r) {
    _obj_ = r._obj_;
    ((Button_tmp*)&r)->_obj_ = 0;
}
inline Button& Button::operator =(const Button_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Button_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Button::Button(const Button_var& e) {
    _obj_ = Button::_duplicate(e._obj_);
}
inline Button& Button::operator =(const Button_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Button::_duplicate(e._obj_);
    return *this;
}
inline Button::~Button() {
    _BaseObject__release(_obj_);
}
inline Button_tmp Button::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Button_tmp Button::_duplicate(const Button& r) {
    return _duplicate(r._obj_);
}
inline Button::operator Viewer() const {
    return Viewer_tmp((ViewerRef)_BaseObject__duplicate((ViewerRef)_obj_, &ViewerStub::_create));
}
inline Button::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)_obj_, &GlyphStub::_create));
}
inline Button::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Button_tmp::Button_tmp(const Button& r) {
    _obj_ = Button::_duplicate(r._obj_);
}
inline Button_tmp::Button_tmp(const Button_tmp& r) {
    _obj_ = r._obj_;
    ((Button_tmp*)&r)->_obj_ = 0;
}
inline Button_tmp::~Button_tmp() { }

inline TelltaleRef Telltale::_duplicate(TelltaleRef obj) {
    return (TelltaleRef)_BaseObject__duplicate(obj, &TelltaleStub::_create);
}
inline Telltale& Telltale::operator =(TelltaleRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Telltale::_duplicate(p);
    return *this;
}
inline Telltale::Telltale(const Telltale& r) {
    _obj_ = Telltale::_duplicate(r._obj_);
}
inline Telltale& Telltale::operator =(const Telltale& r) {
    _BaseObject__release(_obj_);
    _obj_ = Telltale::_duplicate(r._obj_);
    return *this;
}
inline Telltale::Telltale(const Telltale_tmp& r) {
    _obj_ = r._obj_;
    ((Telltale_tmp*)&r)->_obj_ = 0;
}
inline Telltale& Telltale::operator =(const Telltale_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Telltale_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Telltale::Telltale(const Telltale_var& e) {
    _obj_ = Telltale::_duplicate(e._obj_);
}
inline Telltale& Telltale::operator =(const Telltale_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Telltale::_duplicate(e._obj_);
    return *this;
}
inline Telltale::~Telltale() {
    _BaseObject__release(_obj_);
}
inline Telltale_tmp Telltale::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Telltale_tmp Telltale::_duplicate(const Telltale& r) {
    return _duplicate(r._obj_);
}
inline Telltale::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Telltale_tmp::Telltale_tmp(const Telltale& r) {
    _obj_ = Telltale::_duplicate(r._obj_);
}
inline Telltale_tmp::Telltale_tmp(const Telltale_tmp& r) {
    _obj_ = r._obj_;
    ((Telltale_tmp*)&r)->_obj_ = 0;
}
inline Telltale_tmp::~Telltale_tmp() { }

inline MenuItemRef MenuItem::_duplicate(MenuItemRef obj) {
    return (MenuItemRef)_BaseObject__duplicate(obj, &MenuItemStub::_create);
}
inline MenuItem& MenuItem::operator =(MenuItemRef p) {
    _BaseObject__release(_obj_);
    _obj_ = MenuItem::_duplicate(p);
    return *this;
}
inline MenuItem::MenuItem(const MenuItem& r) {
    _obj_ = MenuItem::_duplicate(r._obj_);
}
inline MenuItem& MenuItem::operator =(const MenuItem& r) {
    _BaseObject__release(_obj_);
    _obj_ = MenuItem::_duplicate(r._obj_);
    return *this;
}
inline MenuItem::MenuItem(const MenuItem_tmp& r) {
    _obj_ = r._obj_;
    ((MenuItem_tmp*)&r)->_obj_ = 0;
}
inline MenuItem& MenuItem::operator =(const MenuItem_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((MenuItem_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline MenuItem::MenuItem(const MenuItem_var& e) {
    _obj_ = MenuItem::_duplicate(e._obj_);
}
inline MenuItem& MenuItem::operator =(const MenuItem_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = MenuItem::_duplicate(e._obj_);
    return *this;
}
inline MenuItem::~MenuItem() {
    _BaseObject__release(_obj_);
}
inline MenuItem_tmp MenuItem::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline MenuItem_tmp MenuItem::_duplicate(const MenuItem& r) {
    return _duplicate(r._obj_);
}
inline MenuItem::operator Button() const {
    return Button_tmp((ButtonRef)_BaseObject__duplicate((ButtonRef)_obj_, &ButtonStub::_create));
}
inline MenuItem::operator Viewer() const {
    return Viewer_tmp((ViewerRef)_BaseObject__duplicate((ButtonRef)(ViewerRef)_obj_, &ViewerStub::_create));
}
inline MenuItem::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((ButtonRef)(ViewerRef)(GlyphRef)_obj_, &GlyphStub::_create));
}
inline MenuItem::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((ButtonRef)(ViewerRef)(GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline MenuItem_tmp::MenuItem_tmp(const MenuItem& r) {
    _obj_ = MenuItem::_duplicate(r._obj_);
}
inline MenuItem_tmp::MenuItem_tmp(const MenuItem_tmp& r) {
    _obj_ = r._obj_;
    ((MenuItem_tmp*)&r)->_obj_ = 0;
}
inline MenuItem_tmp::~MenuItem_tmp() { }

inline MenuRef Menu::_duplicate(MenuRef obj) {
    return (MenuRef)_BaseObject__duplicate(obj, &MenuStub::_create);
}
inline Menu& Menu::operator =(MenuRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Menu::_duplicate(p);
    return *this;
}
inline Menu::Menu(const Menu& r) {
    _obj_ = Menu::_duplicate(r._obj_);
}
inline Menu& Menu::operator =(const Menu& r) {
    _BaseObject__release(_obj_);
    _obj_ = Menu::_duplicate(r._obj_);
    return *this;
}
inline Menu::Menu(const Menu_tmp& r) {
    _obj_ = r._obj_;
    ((Menu_tmp*)&r)->_obj_ = 0;
}
inline Menu& Menu::operator =(const Menu_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Menu_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Menu::Menu(const Menu_var& e) {
    _obj_ = Menu::_duplicate(e._obj_);
}
inline Menu& Menu::operator =(const Menu_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Menu::_duplicate(e._obj_);
    return *this;
}
inline Menu::~Menu() {
    _BaseObject__release(_obj_);
}
inline Menu_tmp Menu::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Menu_tmp Menu::_duplicate(const Menu& r) {
    return _duplicate(r._obj_);
}
inline Menu::operator Viewer() const {
    return Viewer_tmp((ViewerRef)_BaseObject__duplicate((ViewerRef)_obj_, &ViewerStub::_create));
}
inline Menu::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)_obj_, &GlyphStub::_create));
}
inline Menu::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Menu_tmp::Menu_tmp(const Menu& r) {
    _obj_ = Menu::_duplicate(r._obj_);
}
inline Menu_tmp::Menu_tmp(const Menu_tmp& r) {
    _obj_ = r._obj_;
    ((Menu_tmp*)&r)->_obj_ = 0;
}
inline Menu_tmp::~Menu_tmp() { }

inline WidgetKitRef WidgetKit::_duplicate(WidgetKitRef obj) {
    return (WidgetKitRef)_BaseObject__duplicate(obj, &WidgetKitStub::_create);
}
inline WidgetKit& WidgetKit::operator =(WidgetKitRef p) {
    _BaseObject__release(_obj_);
    _obj_ = WidgetKit::_duplicate(p);
    return *this;
}
inline WidgetKit::WidgetKit(const WidgetKit& r) {
    _obj_ = WidgetKit::_duplicate(r._obj_);
}
inline WidgetKit& WidgetKit::operator =(const WidgetKit& r) {
    _BaseObject__release(_obj_);
    _obj_ = WidgetKit::_duplicate(r._obj_);
    return *this;
}
inline WidgetKit::WidgetKit(const WidgetKit_tmp& r) {
    _obj_ = r._obj_;
    ((WidgetKit_tmp*)&r)->_obj_ = 0;
}
inline WidgetKit& WidgetKit::operator =(const WidgetKit_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((WidgetKit_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline WidgetKit::WidgetKit(const WidgetKit_var& e) {
    _obj_ = WidgetKit::_duplicate(e._obj_);
}
inline WidgetKit& WidgetKit::operator =(const WidgetKit_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = WidgetKit::_duplicate(e._obj_);
    return *this;
}
inline WidgetKit::~WidgetKit() {
    _BaseObject__release(_obj_);
}
inline WidgetKit_tmp WidgetKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline WidgetKit_tmp WidgetKit::_duplicate(const WidgetKit& r) {
    return _duplicate(r._obj_);
}
inline WidgetKit::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline WidgetKit_tmp::WidgetKit_tmp(const WidgetKit& r) {
    _obj_ = WidgetKit::_duplicate(r._obj_);
}
inline WidgetKit_tmp::WidgetKit_tmp(const WidgetKit_tmp& r) {
    _obj_ = r._obj_;
    ((WidgetKit_tmp*)&r)->_obj_ = 0;
}
inline WidgetKit_tmp::~WidgetKit_tmp() { }

#endif
