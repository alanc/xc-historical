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

/*
 * This is a hack to define a "global" database for ids
 * for object exchange.  We need ids for both the exchange
 * protocol (objref marshal/unmarshal) and for types.
 */

#include <X11/Fresco/Ox/schema.h>

/* exchanges */
/* Keep 0 free to catch/avoid uninitialized ids */
ExchangeId _nilref_xid_ = 1;
ExchangeId _RemoteExchange_xid_ = 2;
ExchangeId _LibraryExchange_xid_ = 3;
ExchangeId _maximum_known_xid_ = 3;

/* Types */
/* Keep 0 free to catch/avoid uninitialized ids */
TypeObjId TypeIdVar(XfBoolean) = 1;
TypeObjId TypeIdVar(XfChar) = 2;
TypeObjId TypeIdVar(XfDouble) = 3;
TypeObjId TypeIdVar(XfFloat) = 4;
TypeObjId TypeIdVar(XfLong) = 5;
TypeObjId TypeIdVar(XfShort) = 6;
TypeObjId TypeIdVar(XfULong) = 7;
TypeObjId TypeIdVar(XfUShort) = 8;
TypeObjId TypeIdVar(Xfstring) = 9;
TypeObjId TypeIdVar(Xfvoid) = 10;

TypeObjId TypeIdVar(BaseObject) = 20;
TypeObjId TypeIdVar(TypeObj) = 21;
TypeObjId TypeIdVar(RequestObj) = 22;

extern TypeObj_Descriptor
    TypeVar(XfBoolean), TypeVar(XfChar),
    TypeVar(XfDouble), TypeVar(XfFloat),
    TypeVar(XfLong), TypeVar(XfShort),
    TypeVar(XfULong), TypeVar(XfUShort),
    TypeVar(Xfstring), TypeVar(Xfvoid),
    TypeVar(BaseObject), TypeVar(TypeObj), TypeVar(RequestObj);

/* Builtin schema */
TypeObj_Descriptor* _ox_initial_schema[] = {
    &TypeVar(XfBoolean),
    &TypeVar(XfChar),
    &TypeVar(XfDouble),
    &TypeVar(XfFloat),
    &TypeVar(XfLong),
    &TypeVar(XfShort),
    &TypeVar(XfULong),
    &TypeVar(XfUShort),
    &TypeVar(Xfstring),
    &TypeVar(Xfvoid),
    &TypeVar(BaseObject),
    &TypeVar(TypeObj),
    &TypeVar(RequestObj),
    nil
};
