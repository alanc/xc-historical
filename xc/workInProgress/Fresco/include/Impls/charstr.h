/*
 * Copyright (c) 1993-94 Silicon Graphics, Inc.
 * Copyright (c) 1993-94 Fujitsu, Ltd.
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

#ifndef Fresco_Impls_charstr_h
#define Fresco_Impls_charstr_h

#include <X11/Fresco/types.h>
#include <X11/Fresco/Impls/fobjects.h>

class CharStringImpl : public CharStringType {
public:
    CharStringImpl();
    CharStringImpl(const char*);
    CharStringImpl(const char*, Long length);
    CharStringImpl(CharStringRef);
    CharStringImpl(const CharStringImpl&);
    ~CharStringImpl();

    //+ CharString::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* CharString */
    CharString::HashValue hash();
    Long count();
    void copy(CharStringRef s);
    Boolean equal(CharStringRef s);
    Boolean case_insensitive_equal(CharStringRef s);
    void get_data(CharString::Data& d);
    void get_char_data(CharString::CharData& d);
    void put_data(const CharString::Data& d);
    void put_char_data(const CharString::CharData& d);
    //+

    static const char* index(const char* s, Long length, char c);
protected:
    SharedFrescoObjectImpl object_;
    char* data_;
    Long length_;

    void set(const char*, Long);
};

class CharStringBuffer {
public:
    CharStringBuffer(CharString s) { init(s); }
    CharStringBuffer(CharStringRef s) { init(s); }
    ~CharStringBuffer();

    char* string() { return buf_._buffer; }
    Long length() { return buf_._length; }

    Boolean operator ==(const char*);
    Boolean operator ==(const CharStringBuffer&);
    Boolean operator !=(const char*);
    Boolean operator !=(const CharStringBuffer&);
protected:
    char fixed_[100];
    CharString::CharData buf_;

    void init(CharStringRef);
};

static inline unsigned long key_to_hash(CharStringRef s) { return s->hash(); }
static inline key_equal(CharStringRef s1, CharStringRef s2) {
    return s1->equal(s2);
}

#endif
