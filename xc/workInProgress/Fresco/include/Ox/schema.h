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

#ifndef ox_schema_h
#define ox_schema_h

#include "obj-impl.h"

/*
 * This function handles narrowing of a nil object reference.
 * It should be a static member function in BaseObject, but
 * cannot be to workaround a C++ compiler bug.
 */
void* _BaseObject_tnarrow_(BaseObjectRef, TypeObjId, StubCreator);

typedef ULong ExchangeId;

class Exception;
class Exchange;
class MarshalBuffer;
class TypeImpl;

typedef void (*TypeObj_CallFunc)(BaseObjectRef, ULong, MarshalBuffer&);
typedef Exception* (*TypeObj_UnmarshalException)(MarshalBuffer&);

struct TypeObj_Descriptor;

struct TypeObj_OpData {
    const char* name;
    TypeObj_Descriptor* result;
    Long nparams;
};

struct TypeObj_ParamData {
    const char* name;
    Long mode;
    TypeObj_Descriptor* type;
};

struct TypeObj_Descriptor {
    TypeImpl* type;
    TypeObjId* id;
    char* name;
    TypeObj_Descriptor** parents;
    TypeObj_UnmarshalException* excepts;
    TypeObj_OpData* methods;
    TypeObj_ParamData* params;
    TypeObj_CallFunc receive;
};

class TypeImpl : public TypeObjType {
public:
    TypeImpl(TypeObj::KindOf, TypeObj_Descriptor*);
    ~TypeImpl();

    Long ref__(Long);

    //+ TypeObj::=
    string name();
    Boolean op_info(TypeObj::OpInfo& op, Long index);
    TypeObj::KindOf kind();
    Long enum_info();
    void _c_array_info(TypeObjRef& type, Long& size);
    Long members();
    TypeObjRef _c_member_info(Long index);
    void _c_sequence_info(TypeObjRef& type, Long& size);
    TypeObjRef _c_typedef_info();
    //+

    static TypeImpl* make_type(TypeObj_Descriptor*);
protected:
    SharedBaseObjectImpl object_;
    TypeObj::KindOf kind_;
    TypeObj_Descriptor* descriptor_;
    Long opcount_;
};

class TypeSchemaImpl : public TypeSchema {
protected:
    TypeSchemaImpl();
    ~TypeSchemaImpl();
public:
    static TypeSchemaImpl* schema();

    void load(TypeObj_Descriptor*);
    void load_list(TypeObj_Descriptor**);
    TypeObjRef map(TypeObjId);
    Boolean compatible(TypeObjId ancestor, TypeObjId descendant);
    Boolean match(TypeObj_Descriptor*, TypeObjId);
    void receiver(TypeObjId, TypeObj_CallFunc);
    TypeObj_Descriptor* descriptor(TypeObjId);
    void save();
    void deactivate();
protected:
    TypeObj_Descriptor** list_;
    Long size_;
    Long used_;
    TypeObj_Descriptor* cache_[8];

    static TypeSchemaImpl* schema_;
};

/*
 * Handy macros for referring to type variables.
 * The id (TypeIdVar) is _<type>_tid_ and the descriptor (TypeVar)
 * is _<type>_type_.
 */

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __TypeIdVar(T) _##T##_##tid##_
#define TypeIdVar(T) __TypeIdVar(T)
#define __TypeVar(T) _##T##_##type##_
#define TypeVar(T) __TypeVar(T)
#else
#define __TypeIdVar(T) _/**/T/**/_/**/tid/**/_
#define TypeIdVar(T) __TypeIdVar(T)
#define __TypeVar(T) _/**/T/**/_/**/type/**/_
#define TypeVar(T) __TypeVar(T)
#endif

/*
 * Short-hand for simple type descriptors.
 */

#define DefineType(Name,SizeName,StringName) \
TypeObj_Descriptor TypeVar(Name) = { \
    0, &TypeIdVar(Name), StringName, 0, 0, 0, 0, 0 \
};

#endif
