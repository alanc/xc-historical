/* DO NOT EDIT -- Automatically generated from Interfaces/widgets.idl */

#ifndef Interfaces_widgets_h
#define Interfaces_widgets_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/viewer.h>

class AdjustmentType;
typedef AdjustmentType* AdjustmentRef;
class Adjustment;
class _AdjustmentExpr;
class _AdjustmentElem;

class Adjustment {
public:
    AdjustmentRef _obj;

    Adjustment() { _obj = 0; }
    Adjustment(AdjustmentRef p) { _obj = p; }
    Adjustment& operator =(AdjustmentRef p);
    Adjustment(const Adjustment&);
    Adjustment& operator =(const Adjustment& r);
    Adjustment(const _AdjustmentExpr&);
    Adjustment& operator =(const _AdjustmentExpr&);
    Adjustment(const _AdjustmentElem&);
    Adjustment& operator =(const _AdjustmentElem&);
    ~Adjustment();

    operator AdjustmentRef() const { return _obj; }
    AdjustmentRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static AdjustmentRef _narrow(BaseObjectRef p);
    static _AdjustmentExpr _narrow(const BaseObject& r);

    static AdjustmentRef _duplicate(AdjustmentRef obj);
    static _AdjustmentExpr _duplicate(const Adjustment& r);
    struct Settings {
        Coord lower, upper, length;
        Coord cur_lower, cur_upper, cur_length;
    };
};

class _AdjustmentExpr : public Adjustment {
public:
    _AdjustmentExpr(AdjustmentRef p) { _obj = p; }
    _AdjustmentExpr(const Adjustment& r) { _obj = r._obj; }
    _AdjustmentExpr(const _AdjustmentExpr& r) { _obj = r._obj; }
    ~_AdjustmentExpr();
};

class _AdjustmentElem {
public:
    AdjustmentRef _obj;

    _AdjustmentElem(AdjustmentRef p) { _obj = p; }
    operator AdjustmentRef() const { return _obj; }
    AdjustmentRef operator ->() { return _obj; }
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

    _AdjustmentExpr _ref();
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

inline AdjustmentRef Adjustment::_duplicate(AdjustmentRef obj) {
    return (AdjustmentRef)_BaseObject__duplicate(obj, &AdjustmentStub::_create);
}
inline Adjustment& Adjustment::operator =(AdjustmentRef p) {
    _BaseObject__release(_obj);
    _obj = Adjustment::_duplicate(p);
    return *this;
}
inline Adjustment::Adjustment(const Adjustment& r) {
    _obj = Adjustment::_duplicate(r._obj);
}
inline Adjustment& Adjustment::operator =(const Adjustment& r) {
    _BaseObject__release(_obj);
    _obj = Adjustment::_duplicate(r._obj);
    return *this;
}
inline Adjustment::Adjustment(const _AdjustmentExpr& r) {
    _obj = r._obj;
    ((_AdjustmentExpr*)&r)->_obj = 0;
}
inline Adjustment& Adjustment::operator =(const _AdjustmentExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_AdjustmentExpr*)&r)->_obj = 0;
    return *this;
}
inline Adjustment::Adjustment(const _AdjustmentElem& e) {
    _obj = Adjustment::_duplicate(e._obj);
}
inline Adjustment& Adjustment::operator =(const _AdjustmentElem& e) {
    _BaseObject__release(_obj);
    _obj = Adjustment::_duplicate(e._obj);
    return *this;
}
inline Adjustment::~Adjustment() {
    _BaseObject__release(_obj);
}
inline _AdjustmentExpr Adjustment::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _AdjustmentExpr Adjustment::_duplicate(const Adjustment& r) {
    return _duplicate(r._obj);
}
inline Adjustment::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _AdjustmentExpr::~_AdjustmentExpr() { }
inline _AdjustmentExpr AdjustmentType::_ref() { return this; }

class TelltaleType;
typedef TelltaleType* TelltaleRef;
class Telltale;
class _TelltaleExpr;
class _TelltaleElem;

class ButtonType;
typedef ButtonType* ButtonRef;
class Button;
class _ButtonExpr;
class _ButtonElem;

class Button {
public:
    ButtonRef _obj;

    Button() { _obj = 0; }
    Button(ButtonRef p) { _obj = p; }
    Button& operator =(ButtonRef p);
    Button(const Button&);
    Button& operator =(const Button& r);
    Button(const _ButtonExpr&);
    Button& operator =(const _ButtonExpr&);
    Button(const _ButtonElem&);
    Button& operator =(const _ButtonElem&);
    ~Button();

    operator ButtonRef() const { return _obj; }
    ButtonRef operator ->() { return _obj; }

    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static ButtonRef _narrow(BaseObjectRef p);
    static _ButtonExpr _narrow(const BaseObject& r);

    static ButtonRef _duplicate(ButtonRef obj);
    static _ButtonExpr _duplicate(const Button& r);
};

class _ButtonExpr : public Button {
public:
    _ButtonExpr(ButtonRef p) { _obj = p; }
    _ButtonExpr(const Button& r) { _obj = r._obj; }
    _ButtonExpr(const _ButtonExpr& r) { _obj = r._obj; }
    ~_ButtonExpr();
};

class _ButtonElem {
public:
    ButtonRef _obj;

    _ButtonElem(ButtonRef p) { _obj = p; }
    operator ButtonRef() const { return _obj; }
    ButtonRef operator ->() { return _obj; }
};

class ButtonType : public ViewerType {
protected:
    ButtonType();
    virtual ~ButtonType();
public:
    _TelltaleExpr state();
    virtual TelltaleRef _c_state();
    void state(TelltaleRef _p);
    virtual void _c_state(TelltaleRef _p);
    _ActionExpr click_action() {
        return _c_click_action();
    }
    virtual ActionRef _c_click_action();
    void click_action(ActionRef _p) {
        _c_click_action(_p);
    }
    virtual void _c_click_action(ActionRef _p);

    _ButtonExpr _ref();
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

inline ButtonRef Button::_duplicate(ButtonRef obj) {
    return (ButtonRef)_BaseObject__duplicate(obj, &ButtonStub::_create);
}
inline Button& Button::operator =(ButtonRef p) {
    _BaseObject__release(_obj);
    _obj = Button::_duplicate(p);
    return *this;
}
inline Button::Button(const Button& r) {
    _obj = Button::_duplicate(r._obj);
}
inline Button& Button::operator =(const Button& r) {
    _BaseObject__release(_obj);
    _obj = Button::_duplicate(r._obj);
    return *this;
}
inline Button::Button(const _ButtonExpr& r) {
    _obj = r._obj;
    ((_ButtonExpr*)&r)->_obj = 0;
}
inline Button& Button::operator =(const _ButtonExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ButtonExpr*)&r)->_obj = 0;
    return *this;
}
inline Button::Button(const _ButtonElem& e) {
    _obj = Button::_duplicate(e._obj);
}
inline Button& Button::operator =(const _ButtonElem& e) {
    _BaseObject__release(_obj);
    _obj = Button::_duplicate(e._obj);
    return *this;
}
inline Button::~Button() {
    _BaseObject__release(_obj);
}
inline _ButtonExpr Button::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ButtonExpr Button::_duplicate(const Button& r) {
    return _duplicate(r._obj);
}
inline Button::operator Viewer() const {
    return _ViewerExpr((ViewerRef)_BaseObject__duplicate(_obj, &ViewerStub::_create));
}
inline Button::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate((ViewerRef)_obj, &GlyphStub::_create));
}
inline Button::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _ButtonExpr::~_ButtonExpr() { }
inline _ButtonExpr ButtonType::_ref() { return this; }

class Telltale {
public:
    TelltaleRef _obj;

    Telltale() { _obj = 0; }
    Telltale(TelltaleRef p) { _obj = p; }
    Telltale& operator =(TelltaleRef p);
    Telltale(const Telltale&);
    Telltale& operator =(const Telltale& r);
    Telltale(const _TelltaleExpr&);
    Telltale& operator =(const _TelltaleExpr&);
    Telltale(const _TelltaleElem&);
    Telltale& operator =(const _TelltaleElem&);
    ~Telltale();

    operator TelltaleRef() const { return _obj; }
    TelltaleRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static TelltaleRef _narrow(BaseObjectRef p);
    static _TelltaleExpr _narrow(const BaseObject& r);

    static TelltaleRef _duplicate(TelltaleRef obj);
    static _TelltaleExpr _duplicate(const Telltale& r);
    enum Flag {
        enabled, visible, active, chosen, running, stepping, choosable, 
        toggle
    };
};

class _TelltaleExpr : public Telltale {
public:
    _TelltaleExpr(TelltaleRef p) { _obj = p; }
    _TelltaleExpr(const Telltale& r) { _obj = r._obj; }
    _TelltaleExpr(const _TelltaleExpr& r) { _obj = r._obj; }
    ~_TelltaleExpr();
};

class _TelltaleElem {
public:
    TelltaleRef _obj;

    _TelltaleElem(TelltaleRef p) { _obj = p; }
    operator TelltaleRef() const { return _obj; }
    TelltaleRef operator ->() { return _obj; }
};

class TelltaleType : public FrescoObjectType {
protected:
    TelltaleType();
    virtual ~TelltaleType();
public:
    virtual void set(Telltale::Flag f);
    virtual void clear(Telltale::Flag f);
    virtual Boolean test(Telltale::Flag f);
    _TelltaleExpr current() {
        return _c_current();
    }
    virtual TelltaleRef _c_current();
    void current(TelltaleRef _p) {
        _c_current(_p);
    }
    virtual void _c_current(TelltaleRef _p);

    _TelltaleExpr _ref();
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

inline TelltaleRef Telltale::_duplicate(TelltaleRef obj) {
    return (TelltaleRef)_BaseObject__duplicate(obj, &TelltaleStub::_create);
}
inline Telltale& Telltale::operator =(TelltaleRef p) {
    _BaseObject__release(_obj);
    _obj = Telltale::_duplicate(p);
    return *this;
}
inline Telltale::Telltale(const Telltale& r) {
    _obj = Telltale::_duplicate(r._obj);
}
inline Telltale& Telltale::operator =(const Telltale& r) {
    _BaseObject__release(_obj);
    _obj = Telltale::_duplicate(r._obj);
    return *this;
}
inline Telltale::Telltale(const _TelltaleExpr& r) {
    _obj = r._obj;
    ((_TelltaleExpr*)&r)->_obj = 0;
}
inline Telltale& Telltale::operator =(const _TelltaleExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_TelltaleExpr*)&r)->_obj = 0;
    return *this;
}
inline Telltale::Telltale(const _TelltaleElem& e) {
    _obj = Telltale::_duplicate(e._obj);
}
inline Telltale& Telltale::operator =(const _TelltaleElem& e) {
    _BaseObject__release(_obj);
    _obj = Telltale::_duplicate(e._obj);
    return *this;
}
inline Telltale::~Telltale() {
    _BaseObject__release(_obj);
}
inline _TelltaleExpr Telltale::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _TelltaleExpr Telltale::_duplicate(const Telltale& r) {
    return _duplicate(r._obj);
}
inline Telltale::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _TelltaleExpr::~_TelltaleExpr() { }
inline _TelltaleExpr TelltaleType::_ref() { return this; }

class MenuType;
typedef MenuType* MenuRef;
class Menu;
class _MenuExpr;
class _MenuElem;

class MenuItemType;
typedef MenuItemType* MenuItemRef;
class MenuItem;
class _MenuItemExpr;
class _MenuItemElem;

class MenuItem {
public:
    MenuItemRef _obj;

    MenuItem() { _obj = 0; }
    MenuItem(MenuItemRef p) { _obj = p; }
    MenuItem& operator =(MenuItemRef p);
    MenuItem(const MenuItem&);
    MenuItem& operator =(const MenuItem& r);
    MenuItem(const _MenuItemExpr&);
    MenuItem& operator =(const _MenuItemExpr&);
    MenuItem(const _MenuItemElem&);
    MenuItem& operator =(const _MenuItemElem&);
    ~MenuItem();

    operator MenuItemRef() const { return _obj; }
    MenuItemRef operator ->() { return _obj; }

    operator Button() const;
    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static MenuItemRef _narrow(BaseObjectRef p);
    static _MenuItemExpr _narrow(const BaseObject& r);

    static MenuItemRef _duplicate(MenuItemRef obj);
    static _MenuItemExpr _duplicate(const MenuItem& r);
};

class _MenuItemExpr : public MenuItem {
public:
    _MenuItemExpr(MenuItemRef p) { _obj = p; }
    _MenuItemExpr(const MenuItem& r) { _obj = r._obj; }
    _MenuItemExpr(const _MenuItemExpr& r) { _obj = r._obj; }
    ~_MenuItemExpr();
};

class _MenuItemElem {
public:
    MenuItemRef _obj;

    _MenuItemElem(MenuItemRef p) { _obj = p; }
    operator MenuItemRef() const { return _obj; }
    MenuItemRef operator ->() { return _obj; }
};

class MenuItemType : public ButtonType {
protected:
    MenuItemType();
    virtual ~MenuItemType();
public:
    _MenuExpr submenu();
    virtual MenuRef _c_submenu();
    void submenu(MenuRef _p);
    virtual void _c_submenu(MenuRef _p);
    _MenuItemExpr next_menu() {
        return _c_next_menu();
    }
    virtual MenuItemRef _c_next_menu();
    _MenuItemExpr prev_menu() {
        return _c_prev_menu();
    }
    virtual MenuItemRef _c_prev_menu();
    virtual void insert_menu_before(MenuItemRef i);
    virtual void insert_menu_after(MenuItemRef i);
    virtual void replace_menu(MenuItemRef i);
    virtual void remove_menu();

    _MenuItemExpr _ref();
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

inline MenuItemRef MenuItem::_duplicate(MenuItemRef obj) {
    return (MenuItemRef)_BaseObject__duplicate(obj, &MenuItemStub::_create);
}
inline MenuItem& MenuItem::operator =(MenuItemRef p) {
    _BaseObject__release(_obj);
    _obj = MenuItem::_duplicate(p);
    return *this;
}
inline MenuItem::MenuItem(const MenuItem& r) {
    _obj = MenuItem::_duplicate(r._obj);
}
inline MenuItem& MenuItem::operator =(const MenuItem& r) {
    _BaseObject__release(_obj);
    _obj = MenuItem::_duplicate(r._obj);
    return *this;
}
inline MenuItem::MenuItem(const _MenuItemExpr& r) {
    _obj = r._obj;
    ((_MenuItemExpr*)&r)->_obj = 0;
}
inline MenuItem& MenuItem::operator =(const _MenuItemExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_MenuItemExpr*)&r)->_obj = 0;
    return *this;
}
inline MenuItem::MenuItem(const _MenuItemElem& e) {
    _obj = MenuItem::_duplicate(e._obj);
}
inline MenuItem& MenuItem::operator =(const _MenuItemElem& e) {
    _BaseObject__release(_obj);
    _obj = MenuItem::_duplicate(e._obj);
    return *this;
}
inline MenuItem::~MenuItem() {
    _BaseObject__release(_obj);
}
inline _MenuItemExpr MenuItem::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _MenuItemExpr MenuItem::_duplicate(const MenuItem& r) {
    return _duplicate(r._obj);
}
inline MenuItem::operator Button() const {
    return _ButtonExpr((ButtonRef)_BaseObject__duplicate(_obj, &ButtonStub::_create));
}
inline MenuItem::operator Viewer() const {
    return _ViewerExpr((ViewerRef)_BaseObject__duplicate((ButtonRef)_obj, &ViewerStub::_create));
}
inline MenuItem::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate((ButtonRef)(ViewerRef)_obj, &GlyphStub::_create));
}
inline MenuItem::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((ButtonRef)(ViewerRef)(GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _MenuItemExpr::~_MenuItemExpr() { }
inline _MenuItemExpr MenuItemType::_ref() { return this; }

class Menu {
public:
    MenuRef _obj;

    Menu() { _obj = 0; }
    Menu(MenuRef p) { _obj = p; }
    Menu& operator =(MenuRef p);
    Menu(const Menu&);
    Menu& operator =(const Menu& r);
    Menu(const _MenuExpr&);
    Menu& operator =(const _MenuExpr&);
    Menu(const _MenuElem&);
    Menu& operator =(const _MenuElem&);
    ~Menu();

    operator MenuRef() const { return _obj; }
    MenuRef operator ->() { return _obj; }

    operator Viewer() const;
    operator Glyph() const;
    operator FrescoObject() const;
    static MenuRef _narrow(BaseObjectRef p);
    static _MenuExpr _narrow(const BaseObject& r);

    static MenuRef _duplicate(MenuRef obj);
    static _MenuExpr _duplicate(const Menu& r);
};

class _MenuExpr : public Menu {
public:
    _MenuExpr(MenuRef p) { _obj = p; }
    _MenuExpr(const Menu& r) { _obj = r._obj; }
    _MenuExpr(const _MenuExpr& r) { _obj = r._obj; }
    ~_MenuExpr();
};

class _MenuElem {
public:
    MenuRef _obj;

    _MenuElem(MenuRef p) { _obj = p; }
    operator MenuRef() const { return _obj; }
    MenuRef operator ->() { return _obj; }
};

class MenuType : public ViewerType {
protected:
    MenuType();
    virtual ~MenuType();
public:
    virtual void append_menu_item(MenuItemRef i);
    virtual void prepend_menu_item(MenuItemRef i);
    _MenuItemExpr first_menu_item() {
        return _c_first_menu_item();
    }
    virtual MenuItemRef _c_first_menu_item();
    _MenuItemExpr last_menu_item() {
        return _c_last_menu_item();
    }
    virtual MenuItemRef _c_last_menu_item();
    _MenuItemExpr selected() {
        return _c_selected();
    }
    virtual MenuItemRef _c_selected();
    void selected(MenuItemRef _p) {
        _c_selected(_p);
    }
    virtual void _c_selected(MenuItemRef _p);

    _MenuExpr _ref();
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

inline MenuRef Menu::_duplicate(MenuRef obj) {
    return (MenuRef)_BaseObject__duplicate(obj, &MenuStub::_create);
}
inline Menu& Menu::operator =(MenuRef p) {
    _BaseObject__release(_obj);
    _obj = Menu::_duplicate(p);
    return *this;
}
inline Menu::Menu(const Menu& r) {
    _obj = Menu::_duplicate(r._obj);
}
inline Menu& Menu::operator =(const Menu& r) {
    _BaseObject__release(_obj);
    _obj = Menu::_duplicate(r._obj);
    return *this;
}
inline Menu::Menu(const _MenuExpr& r) {
    _obj = r._obj;
    ((_MenuExpr*)&r)->_obj = 0;
}
inline Menu& Menu::operator =(const _MenuExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_MenuExpr*)&r)->_obj = 0;
    return *this;
}
inline Menu::Menu(const _MenuElem& e) {
    _obj = Menu::_duplicate(e._obj);
}
inline Menu& Menu::operator =(const _MenuElem& e) {
    _BaseObject__release(_obj);
    _obj = Menu::_duplicate(e._obj);
    return *this;
}
inline Menu::~Menu() {
    _BaseObject__release(_obj);
}
inline _MenuExpr Menu::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _MenuExpr Menu::_duplicate(const Menu& r) {
    return _duplicate(r._obj);
}
inline Menu::operator Viewer() const {
    return _ViewerExpr((ViewerRef)_BaseObject__duplicate(_obj, &ViewerStub::_create));
}
inline Menu::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate((ViewerRef)_obj, &GlyphStub::_create));
}
inline Menu::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((ViewerRef)(GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _MenuExpr::~_MenuExpr() { }
inline _MenuExpr MenuType::_ref() { return this; }

class WidgetKitType;
typedef WidgetKitType* WidgetKitRef;
class WidgetKit;
class _WidgetKitExpr;
class _WidgetKitElem;

class WidgetKit {
public:
    WidgetKitRef _obj;

    WidgetKit() { _obj = 0; }
    WidgetKit(WidgetKitRef p) { _obj = p; }
    WidgetKit& operator =(WidgetKitRef p);
    WidgetKit(const WidgetKit&);
    WidgetKit& operator =(const WidgetKit& r);
    WidgetKit(const _WidgetKitExpr&);
    WidgetKit& operator =(const _WidgetKitExpr&);
    WidgetKit(const _WidgetKitElem&);
    WidgetKit& operator =(const _WidgetKitElem&);
    ~WidgetKit();

    operator WidgetKitRef() const { return _obj; }
    WidgetKitRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static WidgetKitRef _narrow(BaseObjectRef p);
    static _WidgetKitExpr _narrow(const BaseObject& r);

    static WidgetKitRef _duplicate(WidgetKitRef obj);
    static _WidgetKitExpr _duplicate(const WidgetKit& r);
};

class _WidgetKitExpr : public WidgetKit {
public:
    _WidgetKitExpr(WidgetKitRef p) { _obj = p; }
    _WidgetKitExpr(const WidgetKit& r) { _obj = r._obj; }
    _WidgetKitExpr(const _WidgetKitExpr& r) { _obj = r._obj; }
    ~_WidgetKitExpr();
};

class _WidgetKitElem {
public:
    WidgetKitRef _obj;

    _WidgetKitElem(WidgetKitRef p) { _obj = p; }
    operator WidgetKitRef() const { return _obj; }
    WidgetKitRef operator ->() { return _obj; }
};

class WidgetKitType : public FrescoObjectType {
protected:
    WidgetKitType();
    virtual ~WidgetKitType();
public:
    _GlyphExpr inset_frame(GlyphRef g) {
        return _c_inset_frame(g);
    }
    virtual GlyphRef _c_inset_frame(GlyphRef g);
    _GlyphExpr outset_frame(GlyphRef g) {
        return _c_outset_frame(g);
    }
    virtual GlyphRef _c_outset_frame(GlyphRef g);
    _GlyphExpr bright_inset_frame(GlyphRef g) {
        return _c_bright_inset_frame(g);
    }
    virtual GlyphRef _c_bright_inset_frame(GlyphRef g);
    _ViewerExpr label(CharStringRef s) {
        return _c_label(s);
    }
    virtual ViewerRef _c_label(CharStringRef s);
    _MenuExpr menubar() {
        return _c_menubar();
    }
    virtual MenuRef _c_menubar();
    _MenuExpr pulldown() {
        return _c_pulldown();
    }
    virtual MenuRef _c_pulldown();
    _MenuExpr pullright() {
        return _c_pullright();
    }
    virtual MenuRef _c_pullright();
    _MenuItemExpr menubar_item(GlyphRef g) {
        return _c_menubar_item(g);
    }
    virtual MenuItemRef _c_menubar_item(GlyphRef g);
    _MenuItemExpr menu_item(GlyphRef g) {
        return _c_menu_item(g);
    }
    virtual MenuItemRef _c_menu_item(GlyphRef g);
    _MenuItemExpr check_menu_item(GlyphRef g) {
        return _c_check_menu_item(g);
    }
    virtual MenuItemRef _c_check_menu_item(GlyphRef g);
    _MenuItemExpr radio_menu_item(GlyphRef g, TelltaleRef group) {
        return _c_radio_menu_item(g, group);
    }
    virtual MenuItemRef _c_radio_menu_item(GlyphRef g, TelltaleRef group);
    _MenuItemExpr menu_item_separator() {
        return _c_menu_item_separator();
    }
    virtual MenuItemRef _c_menu_item_separator();
    _TelltaleExpr telltale_group() {
        return _c_telltale_group();
    }
    virtual TelltaleRef _c_telltale_group();
    _ButtonExpr push_button(GlyphRef g, ActionRef a) {
        return _c_push_button(g, a);
    }
    virtual ButtonRef _c_push_button(GlyphRef g, ActionRef a);
    _ButtonExpr default_button(GlyphRef g, ActionRef a) {
        return _c_default_button(g, a);
    }
    virtual ButtonRef _c_default_button(GlyphRef g, ActionRef a);
    _ButtonExpr palette_button(GlyphRef g, ActionRef a) {
        return _c_palette_button(g, a);
    }
    virtual ButtonRef _c_palette_button(GlyphRef g, ActionRef a);
    _ButtonExpr check_box(GlyphRef g, ActionRef a) {
        return _c_check_box(g, a);
    }
    virtual ButtonRef _c_check_box(GlyphRef g, ActionRef a);
    _ButtonExpr radio_button(GlyphRef g, ActionRef a, TelltaleRef group) {
        return _c_radio_button(g, a, group);
    }
    virtual ButtonRef _c_radio_button(GlyphRef g, ActionRef a, TelltaleRef group);
    _ViewerExpr slider(Axis a, AdjustmentRef adj) {
        return _c_slider(a, adj);
    }
    virtual ViewerRef _c_slider(Axis a, AdjustmentRef adj);
    _ViewerExpr scroll_bar(Axis a, AdjustmentRef adj) {
        return _c_scroll_bar(a, adj);
    }
    virtual ViewerRef _c_scroll_bar(Axis a, AdjustmentRef adj);
    _ViewerExpr panner(AdjustmentRef x, AdjustmentRef y) {
        return _c_panner(x, y);
    }
    virtual ViewerRef _c_panner(AdjustmentRef x, AdjustmentRef y);
    _ButtonExpr zoomer(Coord scale, AdjustmentRef x, AdjustmentRef y, AdjustmentRef z) {
        return _c_zoomer(scale, x, y, z);
    }
    virtual ButtonRef _c_zoomer(Coord scale, AdjustmentRef x, AdjustmentRef y, AdjustmentRef z);
    _ButtonExpr up_mover(AdjustmentRef a) {
        return _c_up_mover(a);
    }
    virtual ButtonRef _c_up_mover(AdjustmentRef a);
    _ButtonExpr down_mover(AdjustmentRef a) {
        return _c_down_mover(a);
    }
    virtual ButtonRef _c_down_mover(AdjustmentRef a);
    _ButtonExpr left_mover(AdjustmentRef a) {
        return _c_left_mover(a);
    }
    virtual ButtonRef _c_left_mover(AdjustmentRef a);
    _ButtonExpr right_mover(AdjustmentRef a) {
        return _c_right_mover(a);
    }
    virtual ButtonRef _c_right_mover(AdjustmentRef a);

    _WidgetKitExpr _ref();
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

inline WidgetKitRef WidgetKit::_duplicate(WidgetKitRef obj) {
    return (WidgetKitRef)_BaseObject__duplicate(obj, &WidgetKitStub::_create);
}
inline WidgetKit& WidgetKit::operator =(WidgetKitRef p) {
    _BaseObject__release(_obj);
    _obj = WidgetKit::_duplicate(p);
    return *this;
}
inline WidgetKit::WidgetKit(const WidgetKit& r) {
    _obj = WidgetKit::_duplicate(r._obj);
}
inline WidgetKit& WidgetKit::operator =(const WidgetKit& r) {
    _BaseObject__release(_obj);
    _obj = WidgetKit::_duplicate(r._obj);
    return *this;
}
inline WidgetKit::WidgetKit(const _WidgetKitExpr& r) {
    _obj = r._obj;
    ((_WidgetKitExpr*)&r)->_obj = 0;
}
inline WidgetKit& WidgetKit::operator =(const _WidgetKitExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_WidgetKitExpr*)&r)->_obj = 0;
    return *this;
}
inline WidgetKit::WidgetKit(const _WidgetKitElem& e) {
    _obj = WidgetKit::_duplicate(e._obj);
}
inline WidgetKit& WidgetKit::operator =(const _WidgetKitElem& e) {
    _BaseObject__release(_obj);
    _obj = WidgetKit::_duplicate(e._obj);
    return *this;
}
inline WidgetKit::~WidgetKit() {
    _BaseObject__release(_obj);
}
inline _WidgetKitExpr WidgetKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _WidgetKitExpr WidgetKit::_duplicate(const WidgetKit& r) {
    return _duplicate(r._obj);
}
inline WidgetKit::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _WidgetKitExpr::~_WidgetKitExpr() { }
inline _WidgetKitExpr WidgetKitType::_ref() { return this; }

#endif
