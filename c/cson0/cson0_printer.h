#ifndef CSON0_PRINTER
#define CSON0_PRINTER

#include "cson0.h"

void cson0_print(cson0_elem_t el)
{
	switch (el.kind)
	{
		case CSON0_BOOL:
			printf("%s", el.b ? "true" : "false");
			break;
		case CSON0_NULL:
			printf("null");
			break;
		case CSON0_INT:
			printf("%d", el.i);
			break;
		case CSON0_DOUBLE:
			printf("%.16g", el.n);
			break;
		case CSON0_STRING:
			printf("\"%s\"", el.str);
			break;
		case CSON0_ARRAY:
			printf("[");
			for (size_t i = 0; i < el.arr->length; i++) {
				cson0_print(el.arr->elems[i]);
				if (i < el.arr->length - 1)
					printf(",");
			}
			printf("]");
			break;
		case CSON0_OBJECT:
			printf("{");
			for (size_t i = 0; i < el.obj->length; i++) {
				printf("\"%s\":", el.obj->elems[i].key);
				cson0_print(*el.obj->elems[i].val);
				if (i < el.arr->length - 1)
					printf(",");
			}
			printf("}");
			break;
		default:
			break;
	}
}

#endif /* CSON0_PRINTER */
