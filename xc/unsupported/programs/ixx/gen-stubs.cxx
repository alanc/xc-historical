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
 * Generate code
 */

#include "generator.h"

Boolean ExprImpl::generate_stub(Generator*) { return false; }
Boolean ExprImpl::generate_marshal(Generator*) { return false; }
Boolean ExprImpl::generate_unmarshal(Generator*) { return false; }

Boolean IdentifierImpl::generate_stub(Generator* g) { return generate(g); }
Boolean IdentifierImpl::generate_marshal(Generator* g) { return generate(g); }

Boolean IdentifierImpl::generate_unmarshal(Generator* g) {
    return generate(g);
}

void InterfaceDef::put_stub_hdr(Generator* g) {
    String* s = ident_->string();
    g->emit("\nclass %I%C : public %I%t {\n", s);
    g->emit("public:\n%i%I%C(%O*);\n~%I%C();\n\n", s);
    g->emit("static %S%p _create(%O*);\n");
    g->emit("%O* _exchange();\n");
    g->emit("%uprotected:\n%i");
    g->emit("%O* exch_;\n");
    g->emit("%u};\n");
}

Boolean InterfaceDef::generate_stub(Generator* g) {
    String* s = ident_->string();
    g->emit("%Q%C::%I%C(%O* e) { exch_ = e; }\n", s);
    g->emit("%Q%C::~%I%C() { }\n", s);
    g->emit("%S%p %Q%C::_create(%O* e) {%i\n", s);
    g->emit("return (%S%p)(void*)new %Q%C(e);\n%u}\n", s);
    g->emit("%O* %Q%C::_exchange() {%i\nreturn exch_;\n%u}\n", s);
    if (defs_ != nil) {
	g->enter_scope(info_->block);
	for (ListItr(ExprList) i(*defs_); i.more(); i.next()) {
	    Expr* e = i.cur();
	    Symbol* sym = e->symbol();
	    if (sym != nil) {
		switch (sym->tag()) {
		case Symbol::sym_operation:
		case Symbol::sym_attribute:
		    e->generate_stub(g);
		    break;
		default:
		    break;
		}
	    }
	}
	g->leave_scope();
    }
    return true;
}

Boolean Operation::generate_stub(Generator* g) {
    String* s = ident_->string();
    Boolean has_return_value = !g->void_type(type_);
    Boolean has_marshal = has_marshal_funcs(g);
    long nparams = (params_ == nil) ? 1 : params_->count() + 1;
    long rdesc = type_desc(g, type_);
    g->emit("%B::ArgDesc _%_%N_pdesc[", nil, this);
    g->emit_integer(nparams + 1);
    g->emit("] = { ");
    g->emit_integer(nparams);
    g->emit(", ");
    g->emit_integer(rdesc << 2);
    if (params_ != nil) {
	generate_param_desc(g);
    }
    g->emit(" };\n");
    if (has_marshal) {
	g->emit("%B::ArgMarshal _%_%N_pfunc[] = {\n%i", nil, this);
	g->need_sep(false);
	if (params_ != nil) {
	    generate_param_marshal(g);
	}
	if (g->need_sep(false)) {
	    g->emit(",\n");
	}
	g->need_sep(generate_marshal_func(g, type_));
	g->emit("\n%u};\n");
    }

    g->emit("%B::ArgInfo _%_%N_pinfo = {\n%i", nil, this);
    g->emit("&_%_tid_, ");
    g->emit_integer(index_);
    g->emit(", _%_%N_pdesc, ", nil, this);
    if (has_marshal) {
	g->emit("_%_%N_pfunc", nil, this);
    } else {
	g->emit("0");
    }
    g->emit("\n%u};\n");

    if (need_indirect_ && !inline_indirect_) {
	put_indirect_type(g);
	g->emit(" %~%t::%I", s);
	g->indirect(true);
	g->emit_param_list(params_, Generator::emit_env_formals_body);
	put_indirect_body(g, has_return_value);
	g->indirect(false);
    }

    g->emit("%F %~%t::", nil, type_);
    if (need_indirect_) {
	g->emit("_c_");
    }
    g->emit("%I", s);
    g->emit_param_list(params_, Generator::emit_env_formals_body);
    g->emit(" {\n%i%B _b;\n");
    g->emit("extern %MId _%_tid_;\n");
    g->emit("%B::ArgValue ");
    generate_arg(g, nparams, ";\n");
    if (rdesc == 0) {
	/* pass address of return value */
	g->emit("%F _result;\n", nil, type_);
	generate_arg(g, 0, ".u_addr = &_result;\n");
    }
    if (params_ != nil) {
	generate_param_value(g);
    }
    g->emit("_b.invoke(this, _%_%N_pinfo, _arg%,%a);\n", nil, this);
    if (has_return_value) {
	generate_return_value(g, type_);
    }
    g->emit("%u}\n");
    if (g->cstubs()) {
	g->emit("extern \"C\" %F ", nil, type_);
	g->emit("%_%N(%b%i", nil, this);
	g->emit("%F%p _this", nil, interface_->ident());
	if (params_ != nil) {
	    g->emit(", ");
	    g->emit_param_decls(params_, Generator::emit_env_formals_body);
	}
	g->emit("%b%u) {\n%i");
	if (has_return_value) {
	    g->emit("return ");
	}
	g->emit("_this->%I", s);
	g->emit_param_list(params_, Generator::emit_env_actuals);
	g->emit(";\n%u}\n");
    }
    return true;
}

long Operation::type_desc(Generator* g, Expr* e) {
    Symbol* s = g->actual_type(e);
    long rdesc;
    switch (s->tag()) {
    case Symbol::sym_enum:
	rdesc = g->symbol_table()->long_type()->typename()->kind();
	break;
    case Symbol::sym_string:
	rdesc = g->symbol_table()->string_type()->typename()->kind();
	break;
    case Symbol::sym_typedef:
	rdesc = s->typename()->kind();
	break;
    case Symbol::sym_interface:
	rdesc = s->interface()->kind();
	break;
    default:
	rdesc = 0;
	break;
    }
    return rdesc;
}

Boolean Operation::generate_marshal_func(Generator* g, Expr* e) {
    Boolean b = true;
    Symbol* s = g->actual_type(e);
    switch (s->tag()) {
    case Symbol::sym_string:
    case Symbol::sym_typedef:
    case Symbol::sym_enum:
	/* don't need marshal functions */
	b = false;
	break;
    case Symbol::sym_interface:
	b = g->interface_is_ref(false);
	g->emit("&%F%C::_create", nil, e);
	g->interface_is_ref(b);
	break;
    case Symbol::sym_sequence:
	g->emit("&_%Y_put, &_%Y_get", nil, s->sequence_type());
	break;
    default:
	g->emit("&_%Y_put, &_%Y_get", nil, e);
	break;
    }
    return b;
}

Boolean Operation::has_marshal(Generator* g, Expr* e) {
    switch (g->actual_type(e)->tag()) {
    case Symbol::sym_string:
    case Symbol::sym_typedef:
    case Symbol::sym_enum:
	return false;
    }
    return true;
}

Boolean Operation::has_marshal_funcs(Generator* g) {
    if (has_marshal(g, type_)) {
	return true;
    }
    if (params_ != nil) {
	for (ListItr(ExprList) e(*params_); e.more(); e.next()) {
	    Parameter* p = e.cur()->symbol()->parameter();
	    /* valid params_ => p != nil */
	    Declarator* d = p->declarator();
	    Expr* t = (d->subscripts() != nil) ? d : p->type();
	    if (has_marshal(g, t)) {
		return true;
	    }
	}
    }
    return false;
}

void Operation::generate_param_desc(Generator* g) {
    for (ListItr(ExprList) e(*params_); e.more(); e.next()) {
	g->emit(", ");
	Parameter* p = e.cur()->symbol()->parameter();
	/* valid params_ => p != nil */
	g->emit_integer(p->attr() | (type_desc(g, p) << 2));
    }
}

void Operation::generate_param_marshal(Generator* g) {
    for (ListItr(ExprList) e(*params_); e.more(); e.next()) {
	if (g->need_sep(false)) {
	    g->emit(",\n");
	}
	Parameter* p = e.cur()->symbol()->parameter();
	/* valid params_ => p != nil */
	Declarator* d = p->declarator();
	Expr* t = (d->subscripts() != nil) ? d : p->type();
	g->need_sep(generate_marshal_func(g, t));
    }
}

void Operation::generate_param_value(Generator* g) {
    int n = 1;
    for (ListItr(ExprList) e(*params_); e.more(); e.next(), n++) {
	Parameter* p = e.cur()->symbol()->parameter();
	/* valid params_ => p != nil */
	Boolean in_param = (p->attr() == ExprKit::in_param);
	Declarator* d = p->declarator();
	Identifier* name = d->ident();
	Symbol* s = g->actual_type(d);
	switch (s->tag()) {
	case Symbol::sym_string:
	case Symbol::sym_typedef:
	    if (in_param) {
		generate_arg(g, n, ".u_");
		g->emit("%I = %E;\n", s->typename()->str(), name);
	    }
	    break;
	case Symbol::sym_enum:
	    if (in_param) {
		generate_arg(g, n, ".u_long = ");
		g->emit("%E;\n", nil, name);
	    }
	    break;
	case Symbol::sym_interface:
	    if (in_param) {
		generate_arg(g, n, ".u_objref = ");
		g->emit("%E;\n", nil, name);
	    }
	    break;
	default:
	    in_param = false;
	    break;
	}
	if (!in_param) {
	    generate_arg(g, n, ".u_addr = ");
	    if (s->tag() != Symbol::sym_array) {
		g->emit("&");
	    }
	    g->emit("%E;\n", nil, name);
	}
    }
}

void Operation::generate_return_value(Generator* g, Expr* t) {
    Symbol* s = g->actual_type(t);
    g->emit("return ");
    switch (s->tag()) {
    case Symbol::sym_enum:
	g->emit("(%F)_arg[0].u_long", nil, t);
	break;
    case Symbol::sym_interface:
	g->emit("(%F)_arg[0].u_objref", nil, t);
	break;
    case Symbol::sym_string:
	g->emit("_arg[0].u_string");
	break;
    case Symbol::sym_typedef:
	g->emit("_arg[0].u_%I", s->typename()->str());
	break;
    default:
	g->emit("_result");
	break;
    }
    g->emit(";\n");
}

void Operation::generate_arg(Generator* g, long n, const char* str) {
    g->emit("_arg[");
    g->emit_integer(n);
    g->emit("]");
    g->emit(str);
}

Boolean Parameter::generate_stub(Generator* g) {
    if (attr_ != ExprKit::out_param) {
	g->emit_put(type_, "%E", declarator_);
	return true;
    }
    return false;
}

Boolean AttrDecl::generate_stub(Generator* g) {
    Expr* e;
    long rdesc = Operation::type_desc(g, type_);
    long d = rdesc << 2;
    g->emit("%B::ArgDesc ");
    emit_index_name(g, "set_pdesc[3] = { 2, 4, ");
    g->emit_integer(d + ExprKit::in_param);
    g->emit(" };\n");
    g->emit("%B::ArgDesc ");
    emit_index_name(g, "get_pdesc[2] = { 1, ");
    g->emit_integer(d);
    g->emit(" };\n");
    Boolean has_marshal = Operation::has_marshal(g, type_);
    if (has_marshal) {
	g->emit("%B::ArgMarshal ");
	emit_index_name(g, "_pfunc[] = {\n%i");
	Operation::generate_marshal_func(g, type_);
	g->emit("\n%u};\n");
    }
    Boolean need_sep = false;
    const char* sep = ";\n\n";
    long index = index_;
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	e = i.cur();
	if (need_sep) {
	    g->emit(sep);
	}
	if (!readonly_) {
	    emit_param_info(g, "set", e, index, has_marshal);
	    put_stub(g, e);
	    ++index;
	}
	emit_param_info(g, "get", e, index, has_marshal);
	get_stub(g, e, rdesc);
	++index;
	g->emit("\n%u}\n");
    }
    return true;
}

void AttrDecl::put_stub(Generator* g, Expr* e) {
    g->emit("void %:%E", nil, e);
    g->emit("(%F", nil, type_);
    g->emit(" %A%,%f) {\n%i", nil, e);
    g->emit("%B _b;\n");
    g->emit("%B::ArgValue ");
    Operation::generate_arg(g, 2, ";\n");
    Operation::generate_arg(g, 1, ".u_");
    Symbol* s = g->actual_type(type_);
    switch (s->tag()) {
    case Symbol::sym_enum:
	g->emit("long = ");
	break;
    case Symbol::sym_interface:
	g->emit("objref = ");
	break;
    case Symbol::sym_string:
	g->emit("string = ");
	break;
    case Symbol::sym_typedef:
	g->emit("%I = ", s->typename()->str());
	break;
    case Symbol::sym_array:
	g->emit("addr = ");
	break;
    default:
	g->emit("addr = &");
	break;
    }
    g->emit("%A;\n", nil, e);
    g->emit("_b.invoke(this, _%_set_%E_pinfo, _arg%,%a);\n", nil, e);
    g->emit("%u}\n");
}

void AttrDecl::get_stub(Generator* g, Expr* e, long rdesc) {
    g->emit("%F ", nil, type_);
    g->emit("%:%E(%f)", nil, e);
    g->emit(" {\n%i");
    g->emit("%B _b;\n");
    g->emit("extern %MId _%_tid_;\n");
    g->emit("%B::ArgValue ");
    Operation::generate_arg(g, 1, ";\n");
    if (rdesc == 0) {
	/* pass address of return value */
	g->emit("%F _result;\n", nil, type_);
	Operation::generate_arg(g, 0, ".u_addr = &_result;\n");
    }
    g->emit("_b.invoke(this, _%_get_%E_pinfo, _arg%,%a);\n", nil, e);
    Operation::generate_return_value(g, type_);
}

void AttrDecl::emit_param_info(
    Generator* g, const char* sep, Expr* e, long index, Boolean marshal_func
) {
    g->emit("%B::ArgInfo _%_");
    g->emit(sep);
    g->emit("_%E_pinfo = {\n%i&_%_tid_, ", nil, e);
    g->emit_integer(index);
    g->emit(", ");
    emit_index_name(g, sep);
    g->emit("_pdesc, ");
    if (marshal_func) {
	emit_index_name(g, "_pfunc");
    } else {
	g->emit("0");
    }
    g->emit("\n%u};\n");
}

/*
 * Emit a name suitable for the parameter description information
 * for all the attributes in this declaration.  We could use
 * the index, but for now we use the name of the first attribute.
 */

void AttrDecl::emit_index_name(Generator* g, const char* after) {
    g->emit("_%_");
    declarators_->item(0)->generate(g);
    g->emit("_");
    g->emit(after);
}

/*
 * Generate extern decls for stubs for data types.
 */

Boolean ExprImpl::generate_extern_stubs(Generator*) { return false; }
Boolean IdentifierImpl::generate_extern_stubs(Generator*) { return false; }

Boolean InterfaceDef::generate_extern_stubs(Generator* g) {
    Boolean b = false;
    if (defs_ != nil) {
	g->enter_scope(info_->block);
	if (generate_list(defs_, &Expr::generate_extern_stubs, g)) {
	    g->emit("\n\n");
	    b = true;
	}
	g->leave_scope();
    }
    return b;
}

Boolean TypeName::generate_extern_stubs(Generator* g) {
    return (
	type_ != nil && (
	    g->emit_extern_stubs(type_) |
	    generate_list(declarators_, &Expr::generate_extern_stubs, g)
	)
    );
}

Boolean StructDecl::generate_extern_stubs(Generator* g) {
    return generate_list(members_, &Expr::generate_extern_stubs, g);
}

Boolean StructMember::generate_extern_stubs(Generator* g) {
    return (
	g->emit_extern_stubs(type_) |
	generate_list(declarators_, &Expr::generate_extern_stubs, g)
    );
}

Boolean UnionDecl::generate_extern_stubs(Generator* g) {
    Boolean b = g->emit_extern_stubs(type_);
    for (ListItr(CaseList) i(*cases_); i.more(); i.next()) {
	b |= i.cur()->element()->generate_extern_stubs(g);
    }
    return b;
}

Boolean SequenceDecl::generate_extern_stubs(Generator* g) {
    return g->emit_extern_stubs(type_);
}

Boolean Operation::generate_extern_stubs(Generator* g) {
    return (
	g->emit_extern_stubs(type_) |
	generate_list(params_, &Expr::generate_extern_stubs, g)
    );
}

Boolean Parameter::generate_extern_stubs(Generator* g) {
    return (
	g->emit_extern_stubs(type_) | declarator_->generate_extern_stubs(g)
    );
}

Boolean AttrDecl::generate_extern_stubs(Generator* g) {
    return (
	g->emit_extern_stubs(type_) |
	generate_list(declarators_, &Expr::generate_extern_stubs, g)
    );
}

Boolean Declarator::generate_extern_stubs(Generator* g) {
    long f = g->file_mask();
    if (subscripts_ != nil && !symbol_->declared_stub(f)) {
	g->emit("\nextern void _%Y_put(\n%i%B&, %b", nil, this);
	g->emit("const %F ", nil, element_type_);
	g->emit("%E\n%u);\n", nil, this);
	g->emit("extern void _%Y_get(\n%i%B&, %b", nil, this);
	g->emit("%F ", nil, element_type_);
	g->emit("%E\n%u);", nil, this);
	symbol_->declare_stub(f);
	return true;
    }
    return false;
}

/*
 * Generate stubs for types.  These are separated out because,
 * unlike the method/attribute stubs, these should be shared between
 * client and server code.
 */

Boolean ExprImpl::generate_types(Generator*) { return false; }
Boolean IdentifierImpl::generate_types(Generator*) { return false; }

Boolean InterfaceDef::generate_types(Generator* g) {
    Boolean b = false;
    if (defs_ != nil) {
	g->enter_scope(info_->block);
	b = generate_list(defs_, &Expr::generate_types, g);
	if (info_->block->except_index != 0) {
	    b = put_excepts(g);
	}
	g->leave_scope();
    }
    return b;
}

Boolean InterfaceDef::put_excepts(Generator* g) {
    g->emit("%M_UnmarshalException _%_excepts[] = {\n%i");
    g->need_sep(false);
    for (ListItr(ExprList) i(*defs_); i.more(); i.next()) {
	Symbol* s = i.cur()->symbol();
	if (s != nil && s->tag() == Symbol::sym_exception) {
	    if (g->need_sep(true)) {
		g->emit(",\n");
	    }
	    ExceptDecl* e = s->except_type();
	    g->emit("&%:%I::_get", e->ident()->string());
	}
    }
    g->emit("\n%u};\n");
    return true;
}

Boolean TypeName::generate_types(Generator* g) {
    return (
	type_->generate_types(g) ||
	generate_list(declarators_, &Expr::generate_types, g)
    );
}

Boolean StructDecl::generate_types(Generator* g) {
    g->enter_scope(block_);
    generate_list(members_, &Expr::generate_types, g);
    g->leave_scope();

    String* s = ident_->string();
    g->emit("void _%_%I_put(%B& _b, const %F& _this) {\n%i", s, this);
    generate_list(members_, &Expr::generate_marshal, g);
    g->emit("%u}\n");

    g->emit("void _%_%I_get(%B& _b, %F& _this) {\n%i", s, this);
    generate_list(members_, &Expr::generate_unmarshal, g);
    g->emit("%u}\n");
    return true;
}

Boolean StructMember::generate_types(Generator* g) {
    return (
	type_->generate_types(g) ||
	generate_list(declarators_, &Expr::generate_types, g)
    );
}

Boolean StructMember::generate_marshal(Generator* g) {
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	g->emit_put(type_, "_this.%E", i.cur());
    }
    return true;
}

Boolean StructMember::generate_unmarshal(Generator* g) {
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	g->emit_get(type_, "_this.%E", i.cur());
    }
    return true;
}

Boolean UnionDecl::generate_types(Generator* g) {
    g->enter_scope(block_);
    for (ListItr(CaseList) c(*cases_); c.more(); c.next()) {
	c.cur()->element()->generate_types(g);
    }
    /*
     * Need two leave scopes for extra union scope.
     */
    g->leave_scope();
    g->leave_scope();

    String* s = ident_->string();
    g->emit("void _%_%I_put(%B& _b, const %F& _this) {\n%i", s, this);
    g->emit_put(type_, "_this._d()", type_);
    g->emit("switch (_this._d()) {\n");
    for (ListItr(CaseList) i(*cases_); i.more(); i.next()) {
	i.cur()->generate_copy(&Expr::generate_marshal, g);
    }
    g->emit("}\n%u}\n");

    g->emit("void _%_%I_get(%B& _b, %F& _this) {\n%i", s, this);
    g->emit_get(type_, "_this._d()", type_);
    g->emit("switch (_this._d()) {\n");
    for (ListItr(CaseList) j(*cases_); j.more(); j.next()) {
	j.cur()->generate_copy(&Expr::generate_unmarshal, g);
    }
    g->emit("}\n%u}\n");
    return true;
}

Boolean CaseElement::generate_copy(
    Boolean (Expr::*func)(Generator*), Generator* g
) {
    generate_list(labels_, func, g);
    g->emit("%i");
    (element_->*func)(g);
    g->emit("break;\n%u");
    return true;
}

Boolean UnionMember::generate_marshal(Generator* g) {
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	g->emit_put(type_, "_this.%E()", i.cur());
    }
    return true;
}

Boolean UnionMember::generate_unmarshal(Generator* g) {
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	g->emit_get(type_, "_this.%E()", i.cur());
    }
    return true;
}

Boolean CaseLabel::label(Generator* g) {
    g->emit("case %X:\n", nil, value_);
    return true;
}

Boolean DefaultLabel::generate_marshal(Generator* g) { return label(g); }
Boolean DefaultLabel::generate_unmarshal(Generator* g) { return label(g); }

Boolean DefaultLabel::label(Generator* g) {
    g->emit("default:\n");
    return true;
}

Boolean SequenceDecl::generate_types(Generator* g) {
    g->emit("void _%Y_put(%B& _b, const %F& _this) {\n%i", nil, this);
    generate_marshal(g);
    g->emit("%u}\n");

    g->emit("void _%Y_get(%B& _b, %F& _this) {\n%i", nil, this);
    generate_unmarshal(g);
    g->emit("%u}\n");
    return true;
}

Boolean SequenceDecl::generate_marshal(Generator* g) {
    Symbol* s = g->actual_type(type_);
    if (s->tag() == Symbol::sym_typedef &&
	s != g->symbol_table()->string_type()
    ) {
	/* builtin type */
	g->emit("_b.put_seq(&_this, ");
	g->emit("sizeof(%F));\n", nil, type_);
    } else {
	g->emit("long _i;\n");
	g->emit("_b.put_seq_hdr(&_this);\n");
	g->emit("for (_i = 0; _i < _this._length; _i++) {\n%i");
	g->emit_put(type_, "_this._buffer[_i]", type_);
	g->emit("%u}\n");
    }
    return true;
}

Boolean SequenceDecl::generate_unmarshal(Generator* g) {
    Symbol* s = g->actual_type(type_);
    if (s->tag() == Symbol::sym_typedef &&
	s != g->symbol_table()->string_type()
    ) {
	/* builtin type */
	g->emit("_b.get_seq(&_this, sizeof(%F));\n", nil, type_);
    } else {
	g->emit("Long _i;\n");
	g->emit("_b.get_seq_hdr(&_this);\n_this._buffer = ");
	g->emit(
	    "(_this._maximum == 0) ? 0 : new %F[_this._maximum];\n",
	    nil, type_
	);
	g->emit("for (_i = 0; _i < _this._length; _i++) {\n%i");
	g->emit_get(type_, "_this._buffer[_i]", type_);
	g->emit("%u}\n");
    }
    return true;
}

Boolean ExceptDecl::generate_types(Generator* g) {
    g->enter_scope(block_);
    generate_list(members_, &Expr::generate_types, g);
    g->leave_scope();

    String* s = ident_->string();
    g->emit("void %Q::_put(%B& _b) const {\n%i", s);
    g->emit("_b.put_long(_%_tid_);\n", s);
    g->emit("_b.put_long(_major_);\n");
    if (members_ != nil) {
	g->emit("const %F& _this = *this;\n", nil, this);
	generate_list(members_, &Expr::generate_marshal, g);
    }
    g->emit("%u}\n");

    /*
     * The output code is a little strange because it uses
     * a reference temporary and then returns its address.
     * This usage is necessary because the member unmarshal code
     * assumes _this is an object rather than a pointer.
     */
    g->emit("%x* %Q::_get(%B&", s);
    if (members_ == nil) {
	g->emit(") { return new %I; }\n", s);
    } else {
	g->emit(" _b) {\n%i%F& _this = *(new %I);\n", s, this);
	generate_list(members_, &Expr::generate_unmarshal, g);
	g->emit("return &_this;\n%u}\n");
    }
    return true;
}

Boolean Operation::generate_types(Generator* g) {
    return (
	type_->generate_types(g) ||
	generate_list(params_, &Expr::generate_types, g)
    );
}

Boolean Parameter::generate_types(Generator* g) {
    return type_->generate_types(g) || declarator_->generate_types(g);
}

Boolean AttrDecl::generate_types(Generator* g) {
    return type_->generate_types(g);
}

Boolean Declarator::generate_types(Generator* g) {
    if (subscripts_ != nil) {
	Expr* t = element_type_;
	g->emit("void _%Y_put(%b%i%B& _b, ", nil, this);
	g->emit("const %F _array[", nil, t);
	generate_list(subscripts_, &Expr::generate, g, "][");
	g->emit("]%b%u) {%i\n", nil, this);
	g->emit_array_setup(this, t, true);
	g->emit_put(t, "_tmp", t);
	g->emit_array_loop_finish(subscripts_->count());
	g->emit("%u}\n");

	g->emit("void _%Y_get(%b%i%B& _b, ", nil, this);
	g->emit("%F _array[", nil, t);
	generate_list(subscripts_, &Expr::generate, g, "][");
	g->emit("]%b%u) {%i\n", nil, this);
	g->emit_array_setup(this, t, false);
	g->emit_get(t, "_tmp", t);
	g->emit_array_loop_finish(subscripts_->count());
	g->emit("%u}\n");
	return true;
    }
    return false;
}

/*
 * Put out the code to perform a call accessing the parameters
 * through a MarshalBuffer.
 */

void InterfaceDef::put_receive(Generator* g) {
    String* s = ident_->string();
    g->emit("void _%_%I_receive(", s);
    g->emit("%S%p _object, ULong _m, %B& _b) {\n%i");
    g->emit("%:%I%p _this = (%:%I%p)", s);
    put_cast_down(g);
    g->emit("_object;\n");
    if (g->envclass() != nil) {
	g->emit(g->envclass());
	g->emit("* _env = _b.env();\n");
    }
    g->emit("switch (_m) {\n%i");
    g->enter_scope(info_->block);
    if (!generate_list(defs_, &Expr::generate_receive, g)) {
	g->emit("default:\n%ibreak;\n%u");
    }
    g->leave_scope();
    g->emit("%u}\n%u}\n");
}

Boolean ExprImpl::generate_receive(Generator*) { return false; }
Boolean IdentifierImpl::generate_receive(Generator*) { return false; }

Boolean Operation::generate_receive(Generator* g) {
    String* s = ident_->string();
    Boolean has_return = !g->void_type(type_);
    Boolean param_marshal = has_marshal_funcs(g);
    g->emit("case /* %N */ ", nil, this);
    g->emit_integer(index_);
    g->emit(": {\n%i");
    g->emit("extern %B::ArgInfo _%_%N_pinfo;\n", nil, this);
    g->emit("%B::ArgValue ");
    long nparams = params_ == nil ? 1 : params_->count() + 1;
    generate_arg(g, nparams, ";\n");
    if (params_ != nil) {
	long n = 1;
	for (ListItr(ExprList) e(*params_); e.more(); e.next(), n++) {
	    Parameter* p = e.cur()->symbol()->parameter();
	    /* valid params_ => p != nil */
	    generate_receive_addr(g, n, p->type(), p->declarator());
	}
	g->emit("_b.receive(_%_%N_pinfo, _arg);\n", nil, this);
    }
    long rdesc = type_desc(g, type_);
    if (has_return) {
	if (rdesc == 0) {
	    g->emit("%F _result = ", nil, type_);
	} else {
	    generate_receive_asg(g, type_);
	}
    }
    g->emit("_this->");
    if (need_indirect_) {
	g->emit("_c_");
    }
    g->emit("%I", s);
    Boolean b = g->array_decl(false);
    g->emit_param_list(params_, Generator::emit_env_actuals);
    g->array_decl(b);
    g->emit(";\n");
    if (has_return && rdesc == 0) {
	generate_receive_asg(g, type_);
	g->emit("_result;\n");
    }
    g->emit("_b.reply(_%_%N_pinfo, _arg);\n", nil, this);
    g->emit("break;\n%u}\n");
    return true;
}

void Operation::generate_receive_addr(
    Generator* g, long arg, Expr* type, Expr* value
) {
    g->emit("%F ", nil, type);
    g->emit("%E;\n", nil, value);
    generate_arg(g, arg, ".u_addr = ");
    if (g->actual_type(value)->tag() != Symbol::sym_array) {
	g->emit("&");
    }
    Boolean b = g->array_decl(false);
    g->emit("%E;\n", nil, value);
    g->array_decl(b);
}

void Operation::generate_receive_asg(Generator* g, Expr* type) {
    Symbol* s = g->actual_type(type);
    generate_arg(g, 0, ".u_");
    switch (s->tag()) {
    case Symbol::sym_string:
    case Symbol::sym_typedef:
	g->emit("%I = ", s->typename()->str());
	break;
    case Symbol::sym_enum:
	g->emit("long = ");
	break;
    case Symbol::sym_interface:
	g->emit("objref = ");
	break;
    default:
	g->emit("addr = &");
	break;
    }
}

Boolean AttrDecl::generate_receive(Generator* g) {
    long n = index_;
    Boolean has_marshal = Operation::has_marshal(g, type_);
    Expr* e;
    for (ListItr(ExprList) i(*declarators_); i.more(); i.next()) {
	e = i.cur();
	if (!readonly_) {
	    g->emit("case ");
	    g->emit("/* set %E */ ", nil, e);
	    g->emit_integer(n++);
	    g->emit(": {\n%i");
	    g->emit("extern %B::ArgInfo _%_set_%E_pinfo;\n", nil, e);
	    g->emit("%B::ArgValue ");
	    Operation::generate_arg(g, 2, ";\n");
	    Operation::generate_receive_addr(g, 1, type_, e);
	    g->emit("_b.receive(_%_set_%E_pinfo, _arg);\n", nil, e);
	    g->emit("_this->%E(%E);\n", nil, e);
	    g->emit("_b.reply(_%_set_%E_pinfo, _arg);\n", nil, e);
	    g->emit("break;\n%u}\n");
	}
	g->emit("case ");
	g->emit("/* get %E */ ", nil, e);
	g->emit_integer(n++);
	g->emit(": {\n%i");
	g->emit("extern %B::ArgInfo _%_get_%E_pinfo;\n", nil, e);
	g->emit("%B::ArgValue ");
	Operation::generate_arg(g, 1, ";\n");
	long rdesc = Operation::type_desc(g, type_);
	if (rdesc == 0) {
	    g->emit("%F _result = ", nil, type_);
	} else {
	    Operation::generate_receive_asg(g, type_);
	}
	g->emit("_this->%E();\n", nil, e);
	if (rdesc == 0) {
	    Operation::generate_receive_asg(g, type_);
	    g->emit("_result;\n");
	}
	g->emit("_b.reply(_%_get_%E_pinfo, _arg);\n", nil, e);
	g->emit("break;\n%u}\n");
    }
    return true;
}