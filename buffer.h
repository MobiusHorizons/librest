#ifndef __BUFFER_H_
#define __BUFFER_H_
#include <stdlib.h>

typedef struct{
	char * data;
	size_t size;
} buffer;

buffer buffer_init(buffer b, size_t size);
buffer buffer_free(buffer b);
buffer buffer_append(buffer b, char* data, size_t length);
buffer buffer_clone(buffer b);
#endif
