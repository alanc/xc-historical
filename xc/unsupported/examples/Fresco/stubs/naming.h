/* DO NOT EDIT -- Automatically generated from Interfaces/naming.idl */

#ifndef Interfaces_naming_h
#define Interfaces_naming_h

#include <X11/Fresco/Ox/base.h>

class NamingContextType;
typedef NamingContextType* NamingContextRef;
class NamingContext;
class _NamingContextExpr;
class _NamingContextElem;

class NamingContext {
public:
    NamingContextRef _obj;

    NamingContext() { _obj = 0; }
    NamingContext(NamingContextRef p) { _obj = p; }
    NamingContext& operator =(NamingContextRef p);
    NamingContext(const NamingContext&);
    NamingContext& operator =(const NamingContext& r);
    NamingContext(const _NamingContextExpr&);
    NamingContext& operator =(const _NamingContextExpr&);
    NamingContext(const _NamingContextElem&);
    NamingContext& operator =(const _NamingContextElem&);
    ~NamingContext();

    operator NamingContextRef() const { return _obj; }
    NamingContextRef operator ->() { return _obj; }

    static NamingContextRef _narrow(BaseObjectRef p);
    static _NamingContextExpr _narrow(const BaseObject& r);

    static NamingContextRef _duplicate(NamingContextRef obj);
    static _NamingContextExpr _duplicate(const NamingContext& r);
    typedef string NameComponent;
    class Name {
    public:
        long _maximum, _length; NameComponent* _buffer;

        Name() { _maximum = _length = 0; _buffer = 0; }
        Name(long m, long n, NameComponent* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        Name(const Name& _s) { _buffer = 0; *this = _s; }
        Name& operator =(const Name&);
        ~Name() { delete [] _buffer; }
    };
    enum BindingType {
        context_type, symbolic_link_type, ordered_merge_type, regular_type
    };
    struct BindingInfo {
        NameComponent comp;
        BindingType bt;
    };
    class BindingInfoList {
    public:
        long _maximum, _length; BindingInfo* _buffer;

        BindingInfoList() { _maximum = _length = 0; _buffer = 0; }
        BindingInfoList(long m, long n, BindingInfo* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        BindingInfoList(const BindingInfoList& _s) { _buffer = 0; *this = _s; }
        BindingInfoList& operator =(const BindingInfoList&);
        ~BindingInfoList() { delete [] _buffer; }
    };
    class NotFound : public UserException {
    public:
        enum { _index = 1, _code = 19984385 };
        NotFound();
        static NotFound* _cast(const Exception*);

        void _put(MarshalBuffer&) const;
        static Exception* _get(MarshalBuffer&);

        Long mode;
    };
    class NotContext : public UserException {
    public:
        enum { _index = 2, _code = 19984386 };
        NotContext();
        static NotContext* _cast(const Exception*);

        void _put(MarshalBuffer&) const;
        static Exception* _get(MarshalBuffer&);
    };
    class PermissionDenied : public UserException {
    public:
        enum { _index = 3, _code = 19984387 };
        PermissionDenied();
        static PermissionDenied* _cast(const Exception*);

        void _put(MarshalBuffer&) const;
        static Exception* _get(MarshalBuffer&);
    };
};

class _NamingContextExpr : public NamingContext {
public:
    _NamingContextExpr(NamingContextRef p) { _obj = p; }
    _NamingContextExpr(const NamingContext& r) { _obj = r._obj; }
    _NamingContextExpr(const _NamingContextExpr& r) { _obj = r._obj; }
    ~_NamingContextExpr();
};

class _NamingContextElem {
public:
    NamingContextRef _obj;

    _NamingContextElem(NamingContextRef p) { _obj = p; }
    operator NamingContextRef() const { return _obj; }
    NamingContextRef operator ->() { return _obj; }
};

class NamingContextType : public BaseObjectType {
protected:
    NamingContextType();
    virtual ~NamingContextType();
public:
    _BaseObjectExpr resolve(const NamingContext::Name& n, Env* _env = 0);
    virtual BaseObjectRef _c_resolve(const NamingContext::Name& n, Env* _env = 0);
    virtual NamingContext::BindingInfoList list(const NamingContext::Name& n, Env* _env = 0);

    _NamingContextExpr _ref();
    virtual TypeObjId _tid();
};

class NamingContextStub : public NamingContextType {
public:
    NamingContextStub(Exchange*);
    ~NamingContextStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline NamingContextRef NamingContext::_duplicate(NamingContextRef obj) {
    return (NamingContextRef)_BaseObject__duplicate(obj, &NamingContextStub::_create);
}
inline NamingContext& NamingContext::operator =(NamingContextRef p) {
    _BaseObject__release(_obj);
    _obj = NamingContext::_duplicate(p);
    return *this;
}
inline NamingContext::NamingContext(const NamingContext& r) {
    _obj = NamingContext::_duplicate(r._obj);
}
inline NamingContext& NamingContext::operator =(const NamingContext& r) {
    _BaseObject__release(_obj);
    _obj = NamingContext::_duplicate(r._obj);
    return *this;
}
inline NamingContext::NamingContext(const _NamingContextExpr& r) {
    _obj = r._obj;
    ((_NamingContextExpr*)&r)->_obj = 0;
}
inline NamingContext& NamingContext::operator =(const _NamingContextExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_NamingContextExpr*)&r)->_obj = 0;
    return *this;
}
inline NamingContext::NamingContext(const _NamingContextElem& e) {
    _obj = NamingContext::_duplicate(e._obj);
}
inline NamingContext& NamingContext::operator =(const _NamingContextElem& e) {
    _BaseObject__release(_obj);
    _obj = NamingContext::_duplicate(e._obj);
    return *this;
}
inline NamingContext::~NamingContext() {
    _BaseObject__release(_obj);
}
inline _NamingContextExpr NamingContext::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _NamingContextExpr NamingContext::_duplicate(const NamingContext& r) {
    return _duplicate(r._obj);
}
inline _NamingContextExpr::~_NamingContextExpr() { }
inline _NamingContextExpr NamingContextType::_ref() { return this; }

#endif
