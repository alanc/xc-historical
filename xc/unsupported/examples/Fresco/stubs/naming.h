/* DO NOT EDIT -- Automatically generated from Interfaces/naming.idl */

#ifndef Interfaces_naming_h
#define Interfaces_naming_h

#include <X11/Fresco/Ox/base.h>

class NamingContextType;
typedef NamingContextType* NamingContextRef;
typedef NamingContextRef NamingContext_in;
class NamingContext;
class NamingContext_tmp;
class NamingContext_var;

class NamingContext {
public:
    NamingContextRef _obj_;

    NamingContext() { _obj_ = 0; }
    NamingContext(NamingContextRef p) { _obj_ = p; }
    NamingContext& operator =(NamingContextRef p);
    NamingContext(const NamingContext&);
    NamingContext& operator =(const NamingContext& r);
    NamingContext(const NamingContext_tmp&);
    NamingContext& operator =(const NamingContext_tmp&);
    NamingContext(const NamingContext_var&);
    NamingContext& operator =(const NamingContext_var&);
    ~NamingContext();

    NamingContextRef operator ->() { return _obj_; }

    operator NamingContext_in() const { return _obj_; }
    static NamingContextRef _narrow(BaseObjectRef p);
    static NamingContext_tmp _narrow(const BaseObject& r);

    static NamingContextRef _duplicate(NamingContextRef obj);
    static NamingContext_tmp _duplicate(const NamingContext& r);
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
        NotFound(Long mode);
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

class NamingContext_tmp : public NamingContext {
public:
    NamingContext_tmp(NamingContextRef p) { _obj_ = p; }
    NamingContext_tmp(const NamingContext& r);
    NamingContext_tmp(const NamingContext_tmp& r);
    ~NamingContext_tmp();
};

class NamingContext_var {
public:
    NamingContextRef _obj_;

    NamingContext_var(NamingContextRef p) { _obj_ = p; }
    operator NamingContextRef() const { return _obj_; }
    NamingContextRef operator ->() { return _obj_; }
};

class NamingContextType : public BaseObjectType {
protected:
    NamingContextType();
    virtual ~NamingContextType();
public:
    BaseObject_tmp resolve(const NamingContext::Name& n, Env* _env = 0);
    virtual BaseObjectRef _c_resolve(const NamingContext::Name& n, Env* _env = 0);
    virtual NamingContext::BindingInfoList list(const NamingContext::Name& n, Env* _env = 0);
    NamingContextRef _obj() { return this; }
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
    _BaseObject__release(_obj_);
    _obj_ = NamingContext::_duplicate(p);
    return *this;
}
inline NamingContext::NamingContext(const NamingContext& r) {
    _obj_ = NamingContext::_duplicate(r._obj_);
}
inline NamingContext& NamingContext::operator =(const NamingContext& r) {
    _BaseObject__release(_obj_);
    _obj_ = NamingContext::_duplicate(r._obj_);
    return *this;
}
inline NamingContext::NamingContext(const NamingContext_tmp& r) {
    _obj_ = r._obj_;
    ((NamingContext_tmp*)&r)->_obj_ = 0;
}
inline NamingContext& NamingContext::operator =(const NamingContext_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((NamingContext_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline NamingContext::NamingContext(const NamingContext_var& e) {
    _obj_ = NamingContext::_duplicate(e._obj_);
}
inline NamingContext& NamingContext::operator =(const NamingContext_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = NamingContext::_duplicate(e._obj_);
    return *this;
}
inline NamingContext::~NamingContext() {
    _BaseObject__release(_obj_);
}
inline NamingContext_tmp NamingContext::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline NamingContext_tmp NamingContext::_duplicate(const NamingContext& r) {
    return _duplicate(r._obj_);
}
inline NamingContext_tmp::NamingContext_tmp(const NamingContext& r) {
    _obj_ = NamingContext::_duplicate(r._obj_);
}
inline NamingContext_tmp::NamingContext_tmp(const NamingContext_tmp& r) {
    _obj_ = r._obj_;
    ((NamingContext_tmp*)&r)->_obj_ = 0;
}
inline NamingContext_tmp::~NamingContext_tmp() { }

#endif
