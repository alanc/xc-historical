#include "naming.h"
#include <X11/Fresco/Ox/schema.h>
#include <X11/Fresco/Ox/stub.h>

TypeObjId _NamingContext_tid_ = 1000;

//+ NamingContext::%init,type,client
NamingContext::Name& NamingContext::Name::operator =(const Name& _s) {
    delete [] _buffer;
    _maximum = _s._maximum;
    _length = _s._length;
    _buffer = _maximum == 0 ? 0 : new NameComponent[_maximum];
    for (int i = 0; i < _length; i++) {
        _buffer[i] = _s._buffer[i];
    }
    return *this;
}
NamingContext::BindingInfoList& NamingContext::BindingInfoList::operator =(const BindingInfoList& _s) {
    delete [] _buffer;
    _maximum = _s._maximum;
    _length = _s._length;
    _buffer = _maximum == 0 ? 0 : new BindingInfo[_maximum];
    for (int i = 0; i < _length; i++) {
        _buffer[i] = _s._buffer[i];
    }
    return *this;
}
extern TypeObjId _NamingContext_tid_;
NamingContext::NotFound::NotFound() {
    _major_ = _index;
    _hash_ = _code;
    _interface_ = _NamingContext_tid_;
}
NamingContext::NotFound* NamingContext::NotFound::_cast(const Exception* e) {
    if (e->_major() == _index && e->_interface() == _NamingContext_tid_) {
        return (NotFound*)e;
    }
    return 0;
}
extern TypeObjId _NamingContext_tid_;
NamingContext::NotContext::NotContext() {
    _major_ = _index;
    _hash_ = _code;
    _interface_ = _NamingContext_tid_;
}
NamingContext::NotContext* NamingContext::NotContext::_cast(const Exception* e) {
    if (e->_major() == _index && e->_interface() == _NamingContext_tid_) {
        return (NotContext*)e;
    }
    return 0;
}
extern TypeObjId _NamingContext_tid_;
NamingContext::PermissionDenied::PermissionDenied() {
    _major_ = _index;
    _hash_ = _code;
    _interface_ = _NamingContext_tid_;
}
NamingContext::PermissionDenied* NamingContext::PermissionDenied::_cast(const Exception* e) {
    if (e->_major() == _index && e->_interface() == _NamingContext_tid_) {
        return (PermissionDenied*)e;
    }
    return 0;
}

NamingContextType::NamingContextType() { }
NamingContextType::~NamingContextType() { }

TypeObj_Descriptor* _NamingContext_parents_[] = { nil };
extern TypeObjId _NamingContext_tid_;
extern TypeObj_UnmarshalException _NamingContext_excepts[];
TypeObj_Descriptor _NamingContext_type_ = {
    /* type */ 0,
    /* id */ &_NamingContext_tid_,
    "NamingContext",
    _NamingContext_parents_, _NamingContext_excepts,
    /* methods */ nil,
    /* params */ nil,
    /* receive */ nil
};

NamingContextRef NamingContext::_narrow(BaseObjectRef o) {
    return (NamingContextRef)_BaseObject_tnarrow_(
        o, _NamingContext_tid_, &NamingContextStub::_create
    );
}
TypeObjId NamingContextType::_tid() { return _NamingContext_tid_; }

extern void _NamingContext_Name_put(
    MarshalBuffer&, const NamingContext::Name&
);
extern void _NamingContext_Name_get(
    MarshalBuffer&, NamingContext::Name&
);
extern void _NamingContext_BindingInfo_put(
    MarshalBuffer&, const NamingContext::BindingInfo&
);
extern void _NamingContext_BindingInfo_get(
    MarshalBuffer&, NamingContext::BindingInfo&
);
extern void _NamingContext_BindingInfoList_put(
    MarshalBuffer&, const NamingContext::BindingInfoList&
);
extern void _NamingContext_BindingInfoList_get(
    MarshalBuffer&, NamingContext::BindingInfoList&
);
extern void _NamingContext_Name_put(
    MarshalBuffer&, const NamingContext::Name&
);
extern void _NamingContext_Name_get(
    MarshalBuffer&, NamingContext::Name&
);
extern void _NamingContext_BindingInfo_put(
    MarshalBuffer&, const NamingContext::BindingInfo&
);
extern void _NamingContext_BindingInfo_get(
    MarshalBuffer&, NamingContext::BindingInfo&
);
extern void _NamingContext_BindingInfoList_put(
    MarshalBuffer&, const NamingContext::BindingInfoList&
);
extern void _NamingContext_BindingInfoList_get(
    MarshalBuffer&, NamingContext::BindingInfoList&
);
extern void _NamingContext_Name_put(
    MarshalBuffer&, const NamingContext::Name&
);
extern void _NamingContext_Name_get(
    MarshalBuffer&, NamingContext::Name&
);

NamingContextStub::NamingContextStub(Exchange* e) { exch_ = e; }
NamingContextStub::~NamingContextStub() { }
BaseObjectRef NamingContextStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new NamingContextStub(e);
}
Exchange* NamingContextStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _NamingContext_resolve_pdesc[3] = { 2, 48, 1 };
MarshalBuffer::ArgMarshal _NamingContext_resolve_pfunc[] = {
    &_NamingContext_Name_put, &_NamingContext_Name_get,
    &BaseObjectStub::_create
};
MarshalBuffer::ArgInfo _NamingContext_resolve_pinfo = {
    &_NamingContext_tid_, 0, _NamingContext_resolve_pdesc, _NamingContext_resolve_pfunc
};
_BaseObjectExpr NamingContextType::resolve(const NamingContext::Name& n, Env* _env) {
    return _c_resolve(n, _env);
}
BaseObjectRef NamingContextType::_c_resolve(const NamingContext::Name& n, Env* _env) {
    MarshalBuffer _b;
    extern TypeObjId _NamingContext_tid_;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &n;
    _b.invoke(this, _NamingContext_resolve_pinfo, _arg, _env);
    return (BaseObjectRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _NamingContext_list_pdesc[3] = { 2, 0, 1 };
MarshalBuffer::ArgMarshal _NamingContext_list_pfunc[] = {
    &_NamingContext_Name_put, &_NamingContext_Name_get,
    &_NamingContext_BindingInfoList_put, &_NamingContext_BindingInfoList_get
};
MarshalBuffer::ArgInfo _NamingContext_list_pinfo = {
    &_NamingContext_tid_, 1, _NamingContext_list_pdesc, _NamingContext_list_pfunc
};
NamingContext::BindingInfoList NamingContextType::list(const NamingContext::Name& n, Env* _env) {
    MarshalBuffer _b;
    extern TypeObjId _NamingContext_tid_;
    MarshalBuffer::ArgValue _arg[2];
    NamingContext::BindingInfoList _result;
    _arg[0].u_addr = &_result;
    _arg[1].u_addr = &n;
    _b.invoke(this, _NamingContext_list_pinfo, _arg, _env);
    return _result;
}
void _NamingContext_Name_put(MarshalBuffer& _b, const NamingContext::Name& _this) {
    long _i;
    _b.put_seq_hdr(&_this);
    for (_i = 0; _i < _this._length; _i++) {
        _b.put_string(_this._buffer[_i]);
    }
}
void _NamingContext_Name_get(MarshalBuffer& _b, NamingContext::Name& _this) {
    Long _i;
    _b.get_seq_hdr(&_this);
    _this._buffer = (_this._maximum == 0) ? 0 : new NamingContext::NameComponent[_this._maximum];
    for (_i = 0; _i < _this._length; _i++) {
        _this._buffer[_i] = _b.get_string();
    }
}
void _NamingContext_BindingInfo_put(MarshalBuffer& _b, const NamingContext::BindingInfo& _this) {
    _b.put_string(_this.comp);
    _b.put_long(_this.bt);
}
void _NamingContext_BindingInfo_get(MarshalBuffer& _b, NamingContext::BindingInfo& _this) {
    _this.comp = _b.get_string();
    _this.bt = NamingContext::BindingType(_b.get_long());
}
void _NamingContext_BindingInfoList_put(MarshalBuffer& _b, const NamingContext::BindingInfoList& _this) {
    long _i;
    _b.put_seq_hdr(&_this);
    for (_i = 0; _i < _this._length; _i++) {
        _NamingContext_BindingInfo_put(_b, _this._buffer[_i]);
    }
}
void _NamingContext_BindingInfoList_get(MarshalBuffer& _b, NamingContext::BindingInfoList& _this) {
    Long _i;
    _b.get_seq_hdr(&_this);
    _this._buffer = (_this._maximum == 0) ? 0 : new NamingContext::BindingInfo[_this._maximum];
    for (_i = 0; _i < _this._length; _i++) {
        _NamingContext_BindingInfo_get(_b, _this._buffer[_i]);
    }
}
void NamingContext::NotFound::_put(MarshalBuffer& _b) const {
    _b.put_long(_NamingContext_tid_);
    _b.put_long(_major_);
    const NamingContext::NotFound& _this = *this;
    _b.put_long(_this.mode);
}
Exception* NamingContext::NotFound::_get(MarshalBuffer& _b) {
    NamingContext::NotFound& _this = *(new NotFound);
    _this.mode = _b.get_long();
    return &_this;
}
void NamingContext::NotContext::_put(MarshalBuffer& _b) const {
    _b.put_long(_NamingContext_tid_);
    _b.put_long(_major_);
}
Exception* NamingContext::NotContext::_get(MarshalBuffer&) { return new NotContext; }
void NamingContext::PermissionDenied::_put(MarshalBuffer& _b) const {
    _b.put_long(_NamingContext_tid_);
    _b.put_long(_major_);
}
Exception* NamingContext::PermissionDenied::_get(MarshalBuffer&) { return new PermissionDenied; }
TypeObj_UnmarshalException _NamingContext_excepts[] = {
    &NamingContext::NotFound::_get,
    &NamingContext::NotContext::_get,
    &NamingContext::PermissionDenied::_get
};
//+
