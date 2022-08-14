#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cson0.h"
#include "cson0_parser.h"
#include "cson0_printer.h"

#define INITIAL_BUF_PAGES		16
void cson0_free(cson0_elem_t el)
{
	switch (el.kind) {
		case CSON0_STRING:
			free(el.str);
			break;
		case CSON0_OBJECT:
			for (size_t i = 0; i < el.obj->length; ++i) {
				free(el.obj->elems[i].key);
				cson0_free(*el.obj->elems[i].val);
				free(el.obj->elems[i].val);
			}
			free(el.obj->elems);
			free(el.obj);
			break;
		case CSON0_ARRAY:
			for (size_t i = 0; i < el.obj->length; ++i) {
				cson0_free(el.arr->elems[i]);
			}
			free(el.arr->elems);
			free(el.arr);
			break;
	}
}


static char *read_stdin(size_t *total_sz)
{
	const long ps = sysconf(_SC_PAGE_SIZE);
	size_t prev_sz = 0;
	size_t sz = ps * INITIAL_BUF_PAGES;
	size_t read = 0;
	char *input = (char *)malloc(sz);
	char *p = input;
	while ((read = fread(p, 1, sz - prev_sz, stdin)) == sz - prev_sz) {
		prev_sz = sz;
		sz *= 2;
		input = realloc(input, sz);
		p = &input[prev_sz];
	}
	fprintf(stderr, "read = %lu, prev_sz = %lu, sz = %lu\n", read, prev_sz, sz);
	if (total_sz)
		*total_sz = prev_sz + read;
	input[prev_sz + read + 1] = '\0';
	return input;
}

int main(int argc, char **argv)
{
	size_t len = 0;
	char *input = read_stdin(&len);
	cson0_elem_t ast = parse(input, len);
	cson0_print(ast);
	free(input);
	cson0_free(ast);
}
