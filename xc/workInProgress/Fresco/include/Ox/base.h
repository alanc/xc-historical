/*
 * Copyright (c) 1993 Silicon Graphics, Inc.
 * Copyright (c) 1993 Fujitsu, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
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

#ifndef ox_base_h
#define ox_base_h

class Env;
class Exchange;
class MarshalBuffer;

class RequestObjType;
class TypeObjType;

#ifndef nil
#define nil 0
#endif

#ifndef ox_Boolean
#define ox_Boolean

typedef unsigned char Boolean;

static const unsigned char false = 0;
static const unsigned char true = 1;

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#endif

#ifndef ox_octet
#define ox_octet
typedef unsigned char octet;
#endif

#ifndef ox_string
#define ox_string
typedef char* string;
#endif

/*
 * Names for IDL types.
 */

typedef char Char;
typedef unsigned char Octet;
typedef short Short;
typedef unsigned short UShort;

/*
 * IDL longs are 32-bits, which is a potential problem
 * with respect to 32/64-bit machine ambiguities.
 */
#if (_MIPS_SZLONG == 64)
typedef __int32_t Long;
typedef __uint32_t ULong;
#else
typedef long Long;
typedef unsigned long ULong;
#endif

typedef float Float;
typedef double Double;

/* Anachronisms */
typedef Long long_int;
typedef ULong u_long_int;

class BaseObjectType;
typedef BaseObjectType* BaseObjectRef;
class BaseObject;
class _BaseObjectExpr;
class _BaseObjectElem;

#ifndef ox_is_nil
#define ox_is_nil

inline Boolean is_nil(BaseObjectRef p) { return p == 0; }
inline Boolean is_not_nil(BaseObjectRef p) { return p != 0; }

#endif

typedef ULong TypeObjId;
typedef BaseObjectRef (*StubCreator)(Exchange*);

/*
 * These functions are global instead of static member functions
 * to work around a compiler multiple inheritance bug.
 */
extern void* _BaseObject__duplicate(BaseObjectRef, StubCreator);
extern void _BaseObject__release(BaseObjectRef);

class BaseObject {
public:
    BaseObjectRef _obj;

    BaseObject() { _obj = 0; }
    BaseObject(BaseObjectRef p) { _obj = p; }
    BaseObject& operator =(BaseObjectRef p) {
	_BaseObject__release(_obj);
	_obj = p;
	return *this;
    }
    BaseObject(const BaseObject& r) {
	_obj = (BaseObjectRef)_BaseObject__duplicate(r._obj, 0);
    }
    BaseObject& operator =(const BaseObject& r) {
	_BaseObject__release(_obj);
	_obj = (BaseObjectRef)_BaseObject__duplicate(r._obj, 0);
	return *this;
    }
    BaseObject(const _BaseObjectExpr& r);
    BaseObject& operator =(const _BaseObjectExpr& r);
    BaseObject(const _BaseObjectElem& r);
    BaseObject& operator =(const _BaseObjectElem& r);
    ~BaseObject() { _BaseObject__release(_obj); }

    operator BaseObjectRef() const { return _obj; }
    BaseObjectRef operator ->() { return _obj; }

    static BaseObjectRef _duplicate(BaseObjectRef p) {
	return (BaseObjectRef)_BaseObject__duplicate(p, 0);
    }
    static _BaseObjectExpr _duplicate(const BaseObject& r);
};

class _BaseObjectExpr : public BaseObject {
public:
    _BaseObjectExpr(BaseObjectRef p) { _obj = p; }
    _BaseObjectExpr(const BaseObject& r) { _obj = r._obj; }
    _BaseObjectExpr(const _BaseObjectExpr& r) { _obj = r._obj; }
    ~_BaseObjectExpr() { }
};

inline BaseObject::BaseObject(const _BaseObjectExpr& r) {
    _obj = r._obj;
    ((_BaseObjectExpr*)&r)->_obj = 0;
}

inline BaseObject& BaseObject::operator =(const _BaseObjectExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_BaseObjectExpr*)&r)->_obj = 0;
    return *this;
}

inline _BaseObjectExpr BaseObject::_duplicate(const BaseObject& r) {
    return (BaseObjectRef)_BaseObject__duplicate(r._obj, 0);
}

class _BaseObjectElem {
public:
    BaseObjectRef _obj;

    _BaseObjectElem(BaseObjectRef p) { _obj = p; }
    operator BaseObjectRef() const { return _obj; }
    BaseObjectRef operator ->() { return _obj; }
};

inline BaseObject::BaseObject(const _BaseObjectElem& r) {
    _BaseObject__release(_obj);
    _obj = (BaseObjectRef)_BaseObject__duplicate(r._obj, 0);
}

inline BaseObject& BaseObject::operator =(const _BaseObjectElem& r) {
    _BaseObject__release(_obj);
    _obj = (BaseObjectRef)_BaseObject__duplicate(r._obj, 0);
    return *this;
}

//- BaseObjectType*
class BaseObjectType {
    //. All objects support BaseObjectType operations, which
    //. include access to an object's type and holding or
    //. releasing a reference to the object.
public:
    BaseObjectType();
    virtual ~BaseObjectType();

    //- _type
    virtual TypeObjType* _type();
	//. Return a reference to the object's type.

    //- _request
    virtual RequestObjType* _request();
	//. Create a request for performing dynamically-chosen operations
	//. on the object.

    virtual Long ref__(Long references);
    virtual Exchange* _exchange();
    virtual void* _tnarrow(TypeObjId, StubCreator);
    virtual TypeObjId _tid();
private:
    BaseObjectType(const BaseObjectType&);
    void operator =(const BaseObjectType&);
};

class BaseObjectStub : public BaseObjectType {
public:
    BaseObjectStub(Exchange*);
    ~BaseObjectStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class TypeSchema;
typedef TypeSchema* TypeSchemaRef;

class TypeSchema {
protected:
    TypeSchema();
    virtual ~TypeSchema();
public:
    virtual TypeObjType* map(TypeObjId t) = 0;
    virtual Boolean compatible(TypeObjId ancestor, TypeObjId descendant) = 0;
};

class Exception {
protected:
    Exception();
public:
    virtual ~Exception();

    TypeObjId _interface() const;
    Long _major() const;
    Long _hash() const;

    virtual void _put(MarshalBuffer&) const;
protected:
    TypeObjId _interface_;
    Long _major_;
    Long _hash_;
};

inline TypeObjId Exception::_interface() const { return _interface_; }
inline Long Exception::_major() const { return _major_; }
inline Long Exception::_hash() const { return _hash_; }

class SystemException : public Exception {
protected:
    SystemException();
public:
    ~SystemException();

    static SystemException* _cast(const Exception*);
};

class UserException : public Exception {
protected:
    UserException();
public:
    ~UserException();

    static UserException* _cast(const Exception*);
};

#endif
