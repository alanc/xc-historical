/*
 * Copyright (c) 1992-1993 Silicon Graphics, Inc.
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
 * Generate header file
 */

#include "generator.h"

Boolean ExprImpl::generate(Generator*) { return false; }

/*
 * Generate a list of expressions using the given function
 * on each expression.  Return true if any of the functions
 * return true.
 */

Boolean ExprImpl::generate_list(
    ExprList* list, Boolean (Expr::*func)(Generator*), Generator* g,
    const char* sep, const char* trail
) {
    Boolean b = false;
    if (list != nil) {
	long n = g->counter();
	g->counter(0);
	Boolean need_sep = false;
	Expr* e;
	for (ListItr(ExprList) i(*list); i.more(); i.next()) {
	    if (need_sep && sep != nil) {
		g->emit(sep);
	    }
	    e = i.cur();
	    need_sep = (e->*func)(g);
	    b |= need_sep;
	    g->count(+1);
	}
	if (need_sep && trail != nil) {
	    g->emit(trail);
	}
    }
    return b;
}

Boolean RootExpr::generate(Generator* g) {
    SourcePosition* p = g->handler()->position();
    String* s = nil;
    if (p != nil) {
	s = p->filename();
    }
    g->emit_edit_warning(s);
    if (g->begin_file(g->stubfile())) {
	g->emit_edit_warning(s);
	g->emit_stub_includes();
	g->end_file();
    }
    if (g->begin_file(g->serverfile())) {
	g->emit_edit_warning(s);
	g->emit_server_includes();
	g->end_file();
    }
    if (defs_ != nil) {
	/*
	 * The separator semantics are a bit different here
	 * than in ExprList::generate_list because we must check
	 * each expression individually to see if it needs
	 * a trailing semi-colon.  The reason is that some
	 * declarations (e.g., structs) need semi-colons because
	 * they can also be used as types for members, while
	 * interfaces don't need trailing semi-colons.
	 */
	g->need_sep(false);
	g->indirect(true);
	for (ListItr(ExprList) i(*defs_); i.more(); i.next()) {
	    if (g->need_sep(false)) {
		g->emit("\n");
	    }
	    if (i.cur()->generate(g)) {
		g->emit(";\n");
		g->need_sep(true);
	    }
	}
    }
    g->flush();
    return false;
}

Boolean InterfaceDef::generate(Generator* g) {
    if (g->is_source()) {
	String* s = ident_->string();
	g->indirect(false);
	if (!info_->generated_decl) {
	    put_forward_decl(g);
	    if (!forward_) {
		g->emit("\n");
	    }
	    info_->generated_decl = true;
	}
	if (!forward_) {
	    put_hdr(g);
	    if (g->stubclass() != nil) {
		put_stub_hdr(g);
	    }
	    put_hdr_inlines(g);
	    if (g->begin_file(g->stubfile())) {
		put_init(g);
		generate_extern_stubs(g);
		if (g->request() != nil) {
		    put_type_dii(g);
		} else {
		    put_type(g);
		}
		generate_stub(g);
		generate_types(g);
		g->end_file();
	    }
	    if (g->begin_file(g->serverfile())) {
		generate_extern_stubs(g);
		put_receive(g);
		g->end_file();
	    }
	}
	g->need_sep(/* newline separator */ true);
	g->indirect(true);
    }
    return /* no semicolon */ false;
}

void InterfaceDef::put_forward_decl(Generator* g) {
    String* s = ident_->string();
    Boolean has_super = g->superclass() != nil;
    if (g->cdecls()) {
	if (has_super) {
	    g->emit("typedef %S%p %I, %I%p;\n", s);
	} else {
	    g->emit("typedef void* %I, %I%p;\n", s);
	}
    } else {
	g->emit("class %I%t;\ntypedef %I%t* %I%p;\n", s);
	g->emit("class %I%r;\nclass _%I%rExpr;\nclass _%I%rElem;\n", s);
    }
}

void InterfaceDef::put_hdr(Generator* g) {
    String* s = ident_->string();
    const char* super = g->superclass();
    Boolean is_superclass = super != nil && *s == super;
    Boolean cdecls = g->cdecls();
    if (!cdecls) {
	put_indirect(g);
	g->emit("class %I%t", s);
	if (supertypes_ != nil) {
	    g->emit(" : public ");
	    Boolean save = g->interface_is_ref(false);
	    generate_list(
		supertypes_, &Expr::generate, g, "%t, public ", "%t"
	    );
	    g->interface_is_ref(save);
	} else if (super != nil && !is_superclass) {
	    g->emit(" : public %S%t");
	}
	if (is_superclass) {
	    g->emit(" {\npublic");
	} else {
	    g->emit(" {\nprotected");
	}
	g->emit(":\n%i%I%t();\n", s);
	g->emit("virtual ~%I%t();\n%u", s);
	g->emit("public:\n%i");
    }
    g->enter_scope(info_->block);
    const char* trail = cdecls ? nil : ";\n";
    generate_list(defs_, &Expr::generate, g, ";\n", trail);
    g->leave_scope();
    g->emit_transcriptions(true);
    if (super != nil && !cdecls) {
	g->emit("\n_%I%rExpr _ref();\n", s);
	if (g->metaclass() != nil) {
	    g->emit("virtual %MId _tid();\n");
	}
	if (is_superclass) {
	    g->emit("%uprivate:\n%i%I%t(const %I&);\n", s);
	    g->emit("void operator =(const %I&);\n", s);
	}
    }
    if (!cdecls) {
	g->emit("%u}");
    }
    g->emit(";\n");
}

void InterfaceDef::put_hdr_inlines(Generator* g) {
    if (g->superclass() != nil && g->metaclass() != nil && !g->cdecls()) {
	String* s = ident_->string();
	g->emit("\ninline %:%I%p %Q::_duplicate(%I%p obj) {\n%i", s);
	g->emit("return (%I%p)", s);
	put_cast_down(g);
	g->emit("_%S__duplicate(", s);
	put_cast_up(g);
	g->emit("obj, %c);\n%u}\n", s);
	put_indirect_inlines(g);
	g->emit("inline %:_%I%rExpr %Q%t::_ref() { return this; }\n", s);
    }
}

/*
 * Generate a cast up/down the current interface's ancestors.
 * The cast does NOT include this interface, as the exact format of
 * that may vary depending on the current scope.
 */

void InterfaceDef::put_cast_up(Generator* g) {
    if (supertypes_ != nil) {
	InterfaceDef* p = supertypes_->item(0)->symbol()->interface();
	if (p != nil) {
	    p->put_cast_up(g);
	    if (supertypes_->count() > 1) {
		g->emit("(%F%p)", nil, p);
	    }
	}
    }
}

void InterfaceDef::put_cast_down(Generator* g) {
    InterfaceDef* p;
    for (InterfaceDef* i = this; i->supertypes_ != nil; i = p) {
	p = i->supertypes_->item(0)->symbol()->interface();
	if (p != nil && i->supertypes_->count() > 1) {
	    g->emit("(%F%p)", nil, p);
	}
    }
}

/*
 * Generate a call to release an object reference.  Need to cast the
 * pointer up to the base type.
 */

void InterfaceDef::put_release(Generator* g) {
    g->emit("_%S__release(");
    put_cast_up(g);
    g->emit("_obj);");
}

void InterfaceDef::put_indirect(Generator* g) {
    String* s = ident_->string();
    g->indirect(true);

    /* indirect object ref */
    g->emit("class %I%r {\npublic:\n%i", s);
    g->emit("%I%p _obj;\n\n", s);

    g->emit("%I%r() { _obj = 0; }\n", s);
    g->emit("%I%r(%I%p p) { _obj = p; }\n", s);
    g->emit("%I%r& operator =(%I%p p);\n", s);
    g->emit("%I%r(const %I%r&);\n", s);
    g->emit("%I%r& operator =(const %I%r& r);\n", s);
    g->emit("%I%r(const _%I%rExpr&);\n", s);
    g->emit("%I%r& operator =(const _%I%rExpr&);\n", s);
    g->emit("%I%r(const _%I%rElem&);\n", s);
    g->emit("%I%r& operator =(const _%I%rElem&);\n", s);
    g->emit("~%I%r();\n\n", s);
    g->emit("operator %I%p() const { return _obj; }\n", s);
    g->emit("%I%p operator ->() { return _obj; }\n\n", s);

    put_ancestors(g, s, false);

    g->emit("static %I%p _narrow(%S%p p);\n", s);
    g->emit("static _%I%rExpr _narrow(const %S%r& r);\n\n", s);
    g->emit("static %I%p _duplicate(%I%p obj);\n", s);
    g->emit("static _%I%rExpr _duplicate(const %I%r& r);\n", s);

    g->enter_scope(info_->block);
    generate_list(defs_, &Expr::generate, g, ";\n", ";\n");
    g->leave_scope();

    g->emit("%u};\n\n");

    /* indirect ref for temporary expressions */
    g->emit("class _%I%rExpr : public %I%r {\npublic:\n%i", s);
    g->emit("_%I%rExpr(%I%p p) { _obj = p; }\n", s);
    g->emit("_%I%rExpr(const %I%r& r) { _obj = r._obj; }\n", s);
    g->emit("_%I%rExpr(const _%I%rExpr& r) { _obj = r._obj; }\n", s);
    g->emit("~_%I%rExpr();\n", s);
    g->emit("%u};\n\n");

    /* indirect refs that do not release when deallocated */
    g->emit("class _%I%rElem {\npublic:\n%i", s);
    g->emit("%I%p _obj;\n\n", s);
    g->emit("_%I%rElem(%I%p p) { _obj = p; }\n", s);
    g->emit("operator %I%p() const { return _obj; }\n", s);
    g->emit("%I%p operator ->() { return _obj; }\n", s);
    g->emit("%u};\n\n");

    g->indirect(false);
}

void InterfaceDef::put_ancestors(Generator* g, String* name, Boolean body) {
    if (supertypes_ != nil) {
	ExprList path, visited;
	for (ListItr(ExprList) e(*supertypes_); e.more(); e.next()) {
	    InterfaceDef* i = e.cur()->symbol()->interface();
	    path.remove_all();
	    i->put_base(g, name, body, path, visited);
	}
    }
}

void InterfaceDef::put_base(
    Generator* g, String* name, Boolean body,
    ExprList& path, ExprList& visited
) {
    for (ListItr(ExprList) i(visited); i.more(); i.next()) {
	if (i.cur() == this) {
	    return;
	}
    }
    visited.prepend(this);
    String* s = ident_->string();
    if (body) {
	g->emit("inline %:%I::", name);
	g->emit("operator %I() const {\n%i", s);
	g->emit("return _%IExpr((%I%p)_%S__duplicate(", s);
	for (ListItr(ExprList) j(path); j.more(); j.next()) {
	    g->emit("(%F%p)", nil, j.cur());
	}
	g->emit("_obj, %c));\n%u}\n", s);
    } else {
	g->emit("operator %I() const;\n", s);
    }
    path.append(this);
    if (supertypes_ != nil) {
	for (ListItr(ExprList) k(*supertypes_); k.more(); k.next()) {
	    InterfaceDef* i = k.cur()->symbol()->interface();
	    i->put_base(g, name, body, path, visited);
	}
    }
}

void InterfaceDef::put_indirect_inlines(Generator* g) {
    String* s = ident_->string();
    g->indirect(true);
    g->emit("inline %:%I%r& %:%I%r::operator =(%I%p p) {\n%i", s);
    put_release(g);
    g->emit("\n_obj = %I%r::_duplicate(p);\nreturn *this;\n%u}\n", s);

    g->emit("inline %:%I%r::%I%r(const %I%r& r) {\n%i", s);
    g->emit("_obj = %I%r::_duplicate(r._obj);\n%u}\n", s);

    g->emit("inline %:%I%r& %:%I%r::operator =(const %I%r& r) {\n%i", s);
    put_release(g);
    g->emit("\n_obj = %I%r::_duplicate(r._obj);\nreturn *this;\n%u}\n", s);

    g->emit("inline %:%I%r::%I%r(const _%I%rExpr& r) {\n%i", s);
    g->emit("_obj = r._obj;\n");
    g->emit("((_%I%rExpr*)&r)->_obj = 0;\n%u}\n", s);

    g->emit("inline %:%I%r& %:%I%r::operator =(const _%I%rExpr& r) {\n%i", s);
    put_release(g);
    g->emit("\n_obj = r._obj;\n");
    g->emit("((_%I%rExpr*)&r)->_obj = 0;\nreturn *this;\n%u}\n", s);

    g->emit("inline %:%I%r::%I%r(const _%I%rElem& e) {\n%i", s);
    g->emit("_obj = %I%r::_duplicate(e._obj);\n%u}\n", s);

    g->emit("inline %:%I%r& %:%I%r::operator =(const _%I%rElem& e) {\n%i", s);
    put_release(g);
    g->emit("\n_obj = %I%r::_duplicate(e._obj);\nreturn *this;\n%u}\n", s);

    g->emit("inline %:%I%r::~%I%r() {\n%i", s);
    put_release(g);
    g->emit("\n%u}\n");

    g->emit("inline %:_%I%rExpr %:%I%r::_narrow(const %S%r& r)", s);
    g->emit(" {\n%ireturn _narrow(r._obj);\n%u}\n", s);

    g->emit("inline %:_%I%rExpr %:%I%r::_duplicate(const %I%r& r)", s);
    g->emit(" {\n%ireturn _duplicate(r._obj);\n%u}\n", s);

    put_ancestors(g, s, true);

    g->emit("inline %:_%I%rExpr::~_%I%rExpr() { }\n", s);

    g->indirect(false);
}

Boolean Module::generate(Generator* g) {
    return generate_list(defs_, &Expr::generate, g, ";\n");
}

Boolean Accessor::generate(Generator* g) {
    Boolean b = g->interface_is_ref(false);
    if (g->qualify()) {
	g->emit("%E%?", nil, qualifier_);
    }
    g->emit("%I", string_);
    g->interface_is_ref(b);
    return true;
}

Boolean Constant::generate(Generator* g) {
    String* s = ident_->string();
    if (g->cdecls()) {
	g->emit("#define %_%I %E\n", s, value_);
	return /* no trailing semi-colon */ false;
    } else if (g->indirect()) {
	g->emit("static const %F %I", s, type_);
	Scope* b = symbol_->scope();
	if (b == nil || b->name == nil) {
	    g->emit(" = %E", nil, value_);
	}
    }
    return true;
}

Boolean Unary::generate(Generator* g) {
    g->emit_op(op_);
    g->emit("(");
    expr_->generate(g);
    g->emit(")");
    return true;
}

Boolean Binary::generate(Generator* g) {
    g->emit("(");
    left_->generate(g);
    g->emit(")");
    g->emit_op(op_);
    g->emit("(");
    right_->generate(g);
    g->emit(")");
    return true;
}

Boolean TypeName::generate(Generator* g) {
    if (g->is_source() && g->indirect()) {
	if (seq_) {
	    return type_->generate(g);
	}
	if (g->varying()) {
	    switch (type_->symbol()->tag()) {
	    case Symbol::sym_interface:
	    case Symbol::sym_typedef:
	    case Symbol::sym_string:
		g->emit("typedef %E ", nil, type_);
		break;
	    default:
		g->emit("%E;\ntypedef %N ", nil, type_);
		break;
	    }
	} else {
	    g->emit("typedef %E ", nil, type_);
	}
	generate_list(declarators_, &Expr::generate, g, ", ");
	return true;
    }
    return false;
}

Boolean UnsignedType::generate(Generator* g) {
    if (g->is_source()) {
	Symbol* s = symbol_;
	if (s->tag() == Symbol::sym_typedef) {
	    TypeName* t = s->typename();
	    if (t->type() == nil) {
		/* builtin */
		g->emit("%I", t->mapping());
		return true;
	    }
	}
    }
    return false;
}

Boolean Declarator::generate(Generator* g) {
    g->emit_declarator_ident(ident_);
    if (subscripts_ != nil && g->is_array_decl()) {
	g->emit("[");
	generate_list(subscripts_, &Expr::generate, g, "][");
	g->emit("]");
    }
    return true;
}

Boolean StructDecl::generate(Generator* g) {
    if (g->is_source() && g->indirect()) {
	String* s = ident_->string();
	g->emit("struct %I {", s);
	if (members_ != nil) {
	    g->emit("\n%i");
	    generate_list(members_, &Expr::generate, g, ";\n");
	    g->emit(";\n%u");
	} else {
	    g->emit(" ");
	}
	g->emit("}");
	return true;
    }
    return false;
}

Boolean StructMember::generate(Generator* g) {
    g->emit("%E ", nil, type_);
    generate_list(declarators_, &Expr::generate, g, ", ");
    return true;
}

Boolean UnionDecl::generate(Generator* g) {
    if (g->is_source() && g->indirect()) {
	String* s = ident_->string();
	if (g->varying()) {
	    g->emit("class %I {\npublic:\n%i", s);
	} else {
	    g->emit("struct %I {\n%i", s);
	}
	g->emit("%E _d", nil, type_);
	if (!g->cdecls()) {
	    g->emit("_");
	}
	g->emit(";\nunion _U {\n%i");
	for (ListItr(CaseList) c(*cases_); c.more(); c.next()) {
	    c.cur()->generate(g);
	    g->emit(";\n");
	}
	Boolean v = varying();
	if (v) {
	    g->emit("void* __init_;\n");
	}
	g->emit("%u} _u;\n");
	if (g->varying()) {
	    g->emit("\n%I();\n", s);
	    g->emit("%I(const %I& _u) { *this = _u; }\n", s);
	    g->emit("~%I() { ", s);
	    if (v) {
		g->emit("_free(); ");
	    }
	    g->emit("}\n");
	    g->emit("%I& operator =(const %I&);\n\n", s);
	    g->emit("const %F _d() const { return _d_; }\n", nil, type_);
	    g->emit("void _d(%F);\n", nil, type_);
	    g->emit("%F& _d() { return _d_; }\n", nil, type_);
	    for (ListItr(CaseList) i(*cases_); i.more(); i.next()) {
		g->emit("\n");
		generate_access_hdr(g, i.cur());
	    }
	    if (v) {
		g->emit("\nvoid _free();\n");
	    }
	}
	g->emit("%u}");
	return true;
    }
    return false;
}

void UnionDecl::generate_access_hdr(Generator* g, CaseElement* c) {
    UnionMember* u = c->element();
    Expr* t = u->type();
    Expr* e = u->declarators()->item(0);
    Boolean v = t->varying();
    Boolean addr = g->addr_type(t);
    if (addr) {
	g->emit("const ");
    }
    g->emit("%F", nil, t);
    if (addr) {
	g->emit("&");
    }
    g->emit(" %E() const { return ", nil, e);
    if (v) {
	g->emit("*");
    }
    g->emit("_u.%E; }\n", nil, e);

    g->emit("void %E(", nil, e);
    if (addr) {
	g->emit("const ");
    }
    g->emit("%F", nil, t);
    if (addr) {
	g->emit("&");
    }
    g->emit(");\n");

    g->emit("%F& ", nil, t);
    g->emit("%E();\n", nil, e);
}

Boolean CaseElement::generate(Generator* g) {
    return element_->generate(g);
}

Boolean UnionMember::generate(Generator* g) {
    Expr* e = declarators_->item(0);
    g->emit("%E", nil, type_);
    if (g->varying() && type_->varying()) {
	g->emit("* _");
    } else {
	g->emit(" ");
    }
    g->emit("%E", nil, e);
    return true;
}

Boolean CaseLabel::generate(Generator* g) {
    return value_->generate(g);
}

Boolean DefaultLabel::generate(Generator*) {
    /* do not output anything */
    return true;
}

Boolean EnumDecl::generate(Generator* g) {
    if (g->is_source() && g->indirect()) {
	g->emit("enum %I {\n%i", ident_->string());
	generate_list(members_, &Expr::generate, g, ", %b");
	g->emit("%u\n}");
	return true;
    }
    return false;
}

Boolean Enumerator::generate(Generator* g) {
    return ident_->generate(g);
}

Boolean SequenceDecl::generate(Generator* g) {
    if (g->varying()) {
	g->emit("class %N {\npublic:\n%ilong _maximum, _length; ", nil, this);
	g->emit("%E* _buffer;\n\n", nil, type_);
	g->emit("%N() { _maximum = _length = 0; _buffer = 0; }\n", nil, this);
	g->emit("%N(long m, long n, ", nil, this);
	g->emit("%E* e) {\n%i", nil, type_);
	g->emit("_maximum = m; _length = n; _buffer = e;\n%u}\n");
	g->emit("%N(const %N& _s) { _buffer = 0; *this = _s; }\n", nil, this);
	g->emit("%N& operator =(const %N&);\n", nil, this);
	g->emit("~%N() { delete [] _buffer; }\n", nil, this);
	g->emit("%u}");
    } else {
	g->emit("struct %N {\n%ilong _maximum, _length; ", nil, this);
	g->emit("%E* _buffer;\n%u}", nil, type_);
    }
    return true;
}

Boolean StringDecl::generate(Generator* g) {
    if (g->varying()) {
	g->emit("string");
    } else {
	g->emit("char*");
    }
    return true;
}

Boolean AttrDecl::generate(Generator* g) {
    Expr* e;
    Boolean need_sep = false;
    const char* sep = ";\n\n";
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	e = i.cur();
	if (need_sep) {
	    g->emit(sep);
	}
	if (!readonly_) {
	    if (g->cdecls()) {
		g->emit("extern void %__set_%E(%~%p, ", nil, e);
	    } else {
		g->emit("virtual void %E(", nil, e);
	    }
	    if (g->envclass() != nil && g->envfirst()) {
		g->emit("%e, ");
	    }
	    g->emit("%E", nil, type_);
	    if (g->envclass() != nil && !g->envfirst()) {
		g->emit(", %e");
	    }
	    g->emit(")%=;\n", nil, type_);
	}
	if (g->cdecls()) {
	    g->emit("extern %E %__get_", nil, type_);
	    g->emit("%E(%~%p", nil, e);
	    if (g->envclass() != nil) {
		g->emit(", ");
	    }
	} else {
	    g->emit("virtual %E ", nil, type_);
	    g->emit("%E(", nil, e);
	}
	g->emit("%e)%=");
	need_sep = true;
    }
    return true;
}

Boolean ExceptDecl::generate(Generator* g) {
    if (g->is_source() && g->indirect()) {
	String* s = ident_->string();
	g->emit("class %I : public UserException {\npublic:\n%i", s);
	g->emit("enum { _index = ");
	g->emit_integer(index_);
	g->emit(", _code = ");
	long ihash = symbol_->scope()->name->hash();
	g->emit_integer(((ihash & 0xfffff) << 6) + index_);
	g->emit(" };\n");
	g->emit("%I();\n", s);
	g->emit("static %I* _cast(const %x*);\n\n", s);
	g->emit("void _put(%B&) const;\n");
	g->emit("static %x* _get(%B&);\n", s);
	if (members_ != nil) {
	    g->emit("\n");
	    generate_list(members_, &Expr::generate, g, ";\n");
	    g->emit(";\n");
	}
	g->emit("%u}");
	return true;
    }
    return false;
}

/*
 * Still need to handle attributes (oneway) and context.
 */

Boolean Operation::generate(Generator* g) {
    if (g->indirect()) {
	return false;
    }
    if (g->cdecls()) {
	g->emit("extern %Y ", nil, type_);
	g->emit("%_%N(%~%p, ", nil, this);
	g->emit_param_decls(params_, Generator::emit_env_formals);
	g->emit(")");
    } else {
	String* s = ident_->string();
	if (need_indirect_) {
	    g->indirect(true);
	    put_indirect_type(g);
	    g->emit(" %I(", s);
	    g->emit_param_decls(params_, Generator::emit_env_formals);
	    g->emit(")");
	    if (inline_indirect_) {
		put_indirect_body(g, !g->void_type(type_));
	    } else {
		g->emit(";\n");
	    }
	    g->indirect(false);
	}
	g->emit("virtual %F ", nil, type_);
	if (need_indirect_) {
	    g->emit("_c_");
	}
	g->emit("%I(", s, type_);
	g->emit_param_decls(params_, Generator::emit_env_formals);
	g->emit(")%=");
    }
    return true;
}

void Operation::put_indirect_type(Generator* g) {
    if (rtn_indirect_) {
	Boolean b = g->interface_is_ref(false);
	g->emit("%;_%E%rExpr", nil, type_);
	g->interface_is_ref(b);
    } else {
	g->emit("%F", nil, type_);
    }
}

void Operation::put_indirect_body(Generator* g, Boolean has_return_value) {
    g->emit(" {\n%i");
    if (has_return_value) {
	g->emit("return ");
    }
    g->emit("_c_%I", ident_->string());
    g->emit_param_list(params_, Generator::emit_env_actuals);
    g->emit(";\n%u}\n");
}

/*
 * Generate a parameter declaration.  The basic idea for "in" parameters
 * is that primitive types (char, short, long) and object pointers
 * are passed by value, while structured types (string, sequence, struct)
 * are passed by const reference.  For "inout" or "out" parameters,
 * everything is passed non-const reference.  Arrays are the strange case,
 * as they are passed by "value pointer" meaning the mode looks like
 * by-value but is really by-reference.
 *
 * The other unusual situation is when we are generating an actual parameter
 * (not the formal declaration) from an indirect operation, and the
 * parameter is an in or inout object.  In this case, we need to pass
 * a reference to the direct object pointer.
 */

Boolean Parameter::generate(Generator* g) {
    Boolean f = g->formals();
    Boolean a = g->array_decl(f);
    Symbol::Tag t = g->actual_type(type_)->tag();
    Boolean in_param = (attr_ == ExprKit::in_param);
    Boolean indirect = g->indirect();
    if (f) {
	Boolean cdecls = g->cdecls();
	Boolean addr = g->addr_type(type_);
	if (addr && in_param && !cdecls) {
	    g->emit("const ");
	}
	Boolean ref = g->interface_is_ref(false);
	g->emit(cdecls ? "%Y" : "%X", nil, type_);
	if (t == Symbol::sym_interface) {
	    g->emit(indirect && !in_param ? "%r" : "%p");
	}
	g->interface_is_ref(ref);
	if (addr || (!in_param && t != Symbol::sym_array)) {
	    g->emit(cdecls ? "*" : "&");
	}
	g->emit("%o");
    }
    declarator_->generate(g);
    g->array_decl(a);
    if (indirect && !in_param && !f && t == Symbol::sym_interface) {
	g->emit("._obj");
    }
    return true;
}

Boolean IdentifierImpl::generate(Generator* g) {
    Symbol* s = symbol_;
    if (s != nil && s->tag() == Symbol::sym_typedef) {
	TypeName* t = s->typename();
	if (t->type() == nil) {
	    /* builtin type */
	    g->emit("%I", t->mapping());
	    return true;
	}
    }
    g->emit("%I", value_);
    if (symbol_ != nil && symbol_->tag() == Symbol::sym_interface) {
	g->emit("%P");
    }
    return true;
}

Boolean BooleanLiteral::generate(Generator* g) {
    g->emit_integer(value_ ? 1 : 0);
    return true;
}

Boolean IntegerLiteral::generate(Generator* g) {
    g->emit_integer(value_);
    return true;
}

Boolean FloatLiteral::generate(Generator* g) {
    g->emit_float(value_);
    return true;
}

Boolean StringLiteral::generate(Generator* g) {
    g->emit("\"%I\"", value_);
    return true;
}

Boolean CharLiteral::generate(Generator* g) {
    g->emit("\'");
    g->emit_char(value_);
    g->emit("\'");
    return true;
}

/*
 * Regardless of whether this function produces any output
 * it should return false to avoid a semi-colon being output
 * after it.
 */

Boolean SrcPos::generate(Generator* g) {
    SourcePosition* p = position();
    if (p != nil) {
	FileName name = p->filename();
	LineNumber n = p->lineno();
	if (g->set_source(p)) {
	    if (n == 1) {
		g->emit_ifndef(name);
		g->emit_includes();
	    } else {
		g->emit_endif(name);
	    }
	} else if (n == 1) {
	    g->emit_include(name);
	}
    }
    return false;
}

/*
 * Generate the name for an expression; this for type expressions.
 */

Boolean ExprImpl::generate_name(Generator*) { return false; }
Boolean IdentifierImpl::generate_name(Generator* g) { return generate(g); }

Boolean Accessor::generate_name(Generator* g) {
    SequenceDecl* s = g->actual_type(this)->sequence_type();
    if (s != nil) {
	return s->generate_name(g);
    }
    g->emit("%I", string_);
    return true;
}

Boolean InterfaceDef::generate_name(Generator* g) {
    g->emit("%I", ident_->string());
    return true;
}

Boolean Operation::generate_name(Generator* g) {
    g->emit("%I", ident_->string());
    return true;
}

Boolean AttrOp::generate_name(Generator* g) {
    const char* fmt = (params_ == nil) ? "_get_%I" : "_set_%I";
    g->emit(fmt, ident_->string());
    return true;
}

Boolean TypeName::generate_name(Generator* g) {
    return (type_ == nil) ? generate(g) : type_->generate_name(g);
}

Boolean UnsignedType::generate_name(Generator* g) { return generate(g); }

Boolean StructDecl::generate_name(Generator* g) {
    g->emit("%I", ident_->string());
    return true;
}

Boolean UnionDecl::generate_name(Generator* g) {
    g->emit("%I", ident_->string());
    return true;
}

Boolean SequenceDecl::generate_name(Generator* g) {
    if (name_ != nil) {
	name_->generate(g);
    } else {
	g->emit("_%NSeq", nil, type_);
	if (id_ > 1) {
	    g->emit_integer(id_);
	}
    }
    return true;
}

Boolean Declarator::generate_name(Generator* g) {
    g->emit("%I", ident_->string());
    return true;
}

Boolean StringDecl::generate_name(Generator* g) { return generate(g); }
