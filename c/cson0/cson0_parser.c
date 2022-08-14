#include "cson0_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cson0_trace.h"

typedef struct pos {
	const char *data;
	size_t len;
	size_t offset;
} pos_t;

static inline char at(pos_t p)
{
	return p.data[p.offset];
}

static inline pos_t adv(pos_t p)
{
	if (p.offset < p.len) {
		p.offset += 1;
	}
	return p;
}

static inline pos_t skip_ws(pos_t p, cson0_elem_t *el)
{
	char c = at(p);
	while (p.offset < p.len &&
	       (c == ' ' || c == '\n' || c == '\t' || c == '\r')) {
		p = adv(p);
		c = at(p);
	}
	return p;
}

static inline pos_t expect(pos_t p, char c)
{
	if (at(p) == c)
		return adv(p);
	fprintf(stderr, "PARSE ERROR: expected '%c', found '%c'\n", c, at(p));
	return p;
}

static inline pos_t parse_true(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_true");
	if (p.len - p.offset >= 4 && strncmp(&p.data[p.offset], "true", 4) == 0) {
		p.offset += 4;
		if (el) {
			el->kind = CSON0_BOOL;
			el->b = true;
		}
	}
	cson0_trace_exit("parse_true");
	return p;
}

static inline pos_t parse_false(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_false");
	if (p.len - p.offset >= 5 && strncmp(&p.data[p.offset], "false", 5) == 0) {
		p.offset += 5;
		if (el) {
			el->kind = CSON0_BOOL;
			el->b = false;
		}
	}
	cson0_trace_exit("parse_false");
	return p;
}

static inline pos_t parse_null(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_null");
	if (p.len - p.offset >= 4 && strncmp(&p.data[p.offset], "null", 4) == 0) {
		p.offset += 4;
		if (el) {
			el->kind = CSON0_NULL;
		}
	}
	cson0_trace_exit("parse_null");
	return p;
;
}

static inline pos_t parse_hex(pos_t p)
{
	char c = at(p);
	while (p.offset < p.len &&
	       ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f')))
		p = adv(p);
	return p;
}

static inline pos_t parse_escape(pos_t p)
{
	pos_t org = p, tmp;
	if (at(p) == '\\') {
		p = adv(p);
		switch (at(p)) {
		case '"': /* fallthrough */
		case '\\': /* fallthrough */
		case '/': /* fallthrough */
		case 'b': /* fallthrough */
		case 'f': /* fallthrough */
		case 'n': /* fallthrough */
		case 'r': /* fallthrough */
		case 't': /* fallthrough */
			adv(p);
			break;
		case 'u':
			tmp = p;
			p = parse_hex(p);
			if (p.offset - tmp.offset != 4)
				return org;
			break;
		default:
			return org;
		}
	}
	return p;
}

static pos_t parse_chars(pos_t p)
{
	char c;
	pos_t n;
	do {
		n = p;
		c = at(p);
		while (p.offset < p.len && (c != '"' && c != '\\')) {
			p = adv(p);
			c = at(p);
		}
		if (at(p) == '\\') {
			n = p;
			p = parse_escape(p);
		}
	} while (n.offset < p.offset);
	return p;
}

static pos_t parse_string(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_string");
	p = expect(p, '"');
	pos_t start = p;
	p = parse_chars(p);
	p = expect(p, '"');
	el->kind = CSON0_STRING;
	el->str = (char *)calloc(1, p.offset - start.offset);
	memcpy(el->str, &start.data[start.offset], p.offset - start.offset - 1);
	return p;
}

static inline pos_t parse_value(pos_t p, cson0_elem_t *el);
static inline pos_t parse_element(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_element");
	p = skip_ws(p, NULL);
	p = parse_value(p, el);
	p = skip_ws(p, NULL);
	cson0_trace_exit("parse_element");
	return p;
}

static inline pos_t parse_member(pos_t p, cson0_key_value_t *kv)
{
	cson0_trace_enter("parse_member");
	cson0_elem_t k;
	p = skip_ws(p, NULL);
	p = parse_string(p, &k);
	kv->key = k.str;
	cson0_trace_debug("key = \"%s\"", k.str);
	p = skip_ws(p, NULL);
	p = expect(p, ':');
	kv->val = malloc(sizeof(cson0_elem_t));
	p = parse_element(p, kv->val);
	cson0_trace_exit("parse_member");
	return p;
}

static pos_t parse_members(pos_t p, cson0_object_t *obj)
{
	cson0_trace_enter("parse_members");
	obj->length = 1;
	obj->elems = (cson0_key_value_t *)malloc(sizeof(cson0_key_value_t));
	cson0_key_value_t *kv = obj->elems;
	bool cont = true;
	do {
		p = parse_member(p, kv);
		cont = at(p) == ',';
		if (cont) {
			size_t new_len = sizeof(cson0_key_value_t) * ++obj->length;
			obj->elems = realloc(obj->elems, new_len);
			kv = &obj->elems[obj->length - 1];
			p = adv(p);
		}
	} while (cont);
	cson0_trace_exit("parse_members");
	return p;
}

static pos_t parse_object(pos_t p, cson0_elem_t *el)
{
	cson0_object_t *members = (cson0_object_t *)calloc(1, sizeof(cson0_object_t));
	cson0_trace_enter("parse_object");
	p = expect(p, '{');
	p = skip_ws(p, NULL);
	if (at(p) != '}') {
		p = parse_members(p, members);
	}
	p = expect(p, '}');
	el->kind = CSON0_OBJECT;
	el->obj = members;
	cson0_trace_exit("parse_object");
	return p;
}

static pos_t parse_array(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_array");
	el->kind = CSON0_ARRAY;
	el->arr = (cson0_array_t *)calloc(1, sizeof(cson0_array_t));
	p = expect(p, '[');
	p = skip_ws(p, NULL);
	if (at(p) != ']') {
		bool cont = true;
		el->arr->length = 1;
		el->arr->elems = (cson0_elem_t *)malloc(sizeof(cson0_elem_t));
		cson0_elem_t *elm = el->arr->elems;
		do {
			p = parse_element(p, elm);
			cont = at(p) == ',';
			if (cont) {
				p = adv(p);
				size_t len = sizeof(cson0_elem_t) * ++(el->arr->length);
				el->arr->elems = (cson0_elem_t *)realloc(el->arr->elems, len);
				elm = &el->arr->elems[el->arr->length - 1];
			}
		} while (cont);
	}
	p = expect(p, ']');
	cson0_trace_exit("parse_array");
	return p;
}

static inline pos_t maybe(pos_t p, char c)
{
	return at(p) == c ? adv(p) : p;
}

static pos_t parse_integer(pos_t p, cson0_elem_t *el)
{
	pos_t start = p;
	char c = at(maybe(p, '-'));
	cson0_trace_enter("parse_integer");
	if (c == '0') {
		cson0_trace_exit("parse_integer");
		return adv(p);
	} else {
		while (c >= '0' && c <= '9') 
			p = adv(p), c = at(p);
	}
	cson0_trace_debug_pstr("%s", start, p);
	cson0_trace_exit("parse_integer");
	return p;
}

static pos_t parse_fraction(pos_t p, cson0_elem_t *el)
{
	char c;
	pos_t start = p;
	cson0_trace_enter("parse_fraction");
	if (at(p) != '.') {
		cson0_trace_exit("parse_fraction");
		return p;
	}
	p = adv(p);
	c = at(p);
	while (c >= '0' && c <= '9') 
		p = adv(p), c = at(p);
	cson0_trace_debug_pstr("%s", start, p);
	cson0_trace_exit("parse_fraction");
	return p;
}

static pos_t parse_exp(pos_t p, cson0_elem_t *el)
{
	pos_t start = p;
	char c = at(p);
	cson0_trace_enter("parse_exp");
	if (c != 'E' && c != 'e') {
		cson0_trace_exit("parse_exp");
		return p;
	}
	p = adv(p);
	c = at(p);
	if (c == '-' || c == '+')
		p = adv(p), c = at(p);
	while (c >= '0' && c <= '9') 
		p = adv(p), c = at(p);
	cson0_trace_debug_pstr("%s", start, p);
	cson0_trace_exit("parse_exp");
	return p;
}

static pos_t parse_number(pos_t p, cson0_elem_t *el)
{
	char *end = NULL;
	pos_t start = p, integer;
	cson0_trace_enter("parse_number");
	integer = p = parse_integer(p, el);
	p = parse_fraction(p, el);
	p = parse_exp(p, el);
	if (integer.offset != p.offset) {
		el->kind = CSON0_DOUBLE;
		el->n = strtod(&start.data[start.offset], &end);
		cson0_trace_debug("double: %f", el->n);
	} else {
		el->kind = CSON0_INT;
		el->i = strtol(&start.data[start.offset], &end, 0);
		cson0_trace_debug("int: %d", el->i);
	}
	if (end != &p.data[p.offset]) {
		fprintf(stderr, "PARSE ERROR: number didn't consume expected tokens");
	}
	cson0_trace_exit("parse_number");
	return p;
}

static inline pos_t parse_value(pos_t p, cson0_elem_t *el)
{
	cson0_trace_enter("parse_value: '%s'", &p.data[p.offset]);
	switch (at(p)) {
		case '{': p =  parse_object(p, el); break;
		case '[': p = parse_array(p, el); break;
		case '"': p = parse_string(p, el); break;
		case 't': p = parse_true(p, el); break;
		case 'f': p =  parse_false(p, el); break;
		case 'n': p =  parse_null(p, el); break;
		default: p =  parse_number(p, el); break;
	}
	cson0_trace_exit("parse_value");
	return p;
}

cson0_elem_t parse(const char *str, size_t len)
{
	pos_t p = { .data = str, .len = len, .offset = 0, };
	cson0_elem_t ret;
	parse_value(p, &ret);
	return ret;
}
