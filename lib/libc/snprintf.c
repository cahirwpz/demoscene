#include "common.h"

struct print_buf {
	char *buf;
	size_t size;
};

static __regargs void snprint_func(int ch, void *arg) {
	struct print_buf *pbuf = arg;

	if (pbuf->size < 2) {
    /* Reserve last buffer position for the terminating character: */
		return;
	}
	*(pbuf->buf)++ = ch;
	pbuf->size--;
}

int snprintf(char *buf, size_t size, const char *cfmt, ...) {
	int retval;
	va_list ap;
	struct print_buf arg;

	arg.buf = buf;
	arg.size = size;

	va_start(ap, cfmt);
	retval = kvprintf(cfmt, &snprint_func, &arg, ap);
	va_end(ap);

	if (arg.size >= 1)
		*(arg.buf)++ = 0;
	return retval;
}
