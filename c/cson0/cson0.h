#ifndef CSON_H__
#define CSON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// object
// array
// "value"
// string
// number
// true, false, null

typedef enum {
	CSON0_NULL = 0,
	CSON0_BOOL,
	CSON0_STRING,
	CSON0_INT,
	CSON0_DOUBLE,
	CSON0_ARRAY,
	CSON0_OBJECT,
	CSON0_PARSE_ERROR = -1
} cson0_kind_t;

typedef struct cson0_object cson0_object_t;
typedef struct cson0_array cson0_array_t;
typedef struct cson0_elem cson0_elem_t;
typedef struct cson0_key_value cson0_key_value_t;

struct cson0_elem {
	cson0_kind_t kind;
	union {
		bool b;
		int i;
		double n;
		char *str;
		struct cson0_key_value *kv;
		struct cson0_array *arr;
		struct cson0_object *obj;
	};
};

struct cson0_array {
	size_t length;
	cson0_elem_t *elems;
};

struct cson0_key_value {
	char *key;
	struct cson0_elem *val;
};

struct cson0_object {
	size_t length;
	cson0_key_value_t *elems;
};

void cson0_free(cson0_elem_t el);

#endif /* CSON_H__ */
