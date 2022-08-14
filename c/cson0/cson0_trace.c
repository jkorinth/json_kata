#include "cson0_trace.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG

typedef struct pos {
	const char *data;
	size_t len;
	size_t offset;
} pos_t;

static int _depth = 0;

static inline void print_indent(void)
{
	for (int i = 0; i < _depth; ++i)
		printf("  ");
}

void cson0_trace_enter(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);
	print_indent();
	printf("> ");
	vprintf(msg, va);
	printf("\n");
	va_end(va);
	++_depth;
}

void cson0_trace_exit(const char *msg, ...)
{
	va_list va;
	--_depth;
	va_start(va, msg);
	print_indent();
	printf("< ");
	vprintf(msg, va);
	printf("\n");
	va_end(va);
}

void cson0_trace_debug(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);
	print_indent();
	printf("<debug> ");
	vprintf(msg, va);
	printf("\n");
	va_end(va);
}

void cson0_trace_debug_pstr(const char *msg, struct pos start, struct pos end)
{
	size_t len = end.offset - start.offset;
	char *tmp = calloc(1, len + 1);
	strncpy(tmp, &start.data[start.offset], len);
	cson0_trace_debug(msg, tmp);
	free(tmp);
}

#endif /* NDEBUG */
