#ifndef CSON0_TRACE_H__
#define CSON0_TRACE_H__

struct pos;

#ifndef NDEBUG
void cson0_trace_enter(const char *msg, ...);
void cson0_trace_exit(const char *msg, ...);

void cson0_trace_debug(const char *msg, ...);
void cson0_trace_debug_pstr(const char *msg, struct pos start, struct pos end);
#else
#define cson0_trace_enter(msg, ...)
#define cson0_trace_exit(msg, ...)
#define cson0_trace_debug(msg, ...)
#define cson0_trace_debug_pstr(msg, s, e) {(void)(s);(void)(e);}
#endif /* NDEBUG */

#endif /* CSON0_TRACE_H__ */
