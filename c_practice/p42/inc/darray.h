#ifndef DARRAY_H
#define DARRAY_H

#include <stdlib.h>
#include <assert.h>
#include "dbg.h"

typedef struct {
	void** content;
	int size, capacity;
} DArray;

static inline DArray* DArray_create() {
	DArray* ret = malloc(sizeof(DArray));

	ret->size = 0;
	ret->capacity = 0;
	return ret;
}

static inline void DArray_destroy(DArray* arr) {
	if(arr->capacity != 0)
		free(arr->content);
	free(arr);
}

static inline void DArray_push(DArray* arr, void* value) {
	if(arr->capacity == 0) {
		arr->content = malloc(sizeof(void*));
		check_mem(arr->content);
		arr->capacity = 1;
		arr->size = 1;
		*(arr->content) = value;
	} else if(arr->size < arr->capacity) {
		arr->content[arr->size++] = value;
	} else {
		void** new_mem = realloc(arr->content, 2 * arr->capacity * sizeof(void*));
		check_mem(new_mem);
		arr->content = new_mem;
		arr->content[arr->size++] = value;
		arr->capacity *= 2;
	}
error:
	return;
}

static inline void* DArray_pop(DArray* arr) {
	assert(arr->size != 0);
	return arr->content[--(arr->size)];
}

static inline void DArray_shrink(DArray* arr) {
	if(arr->capacity == arr->size)
		return;
	if(arr->size == 0) {
		free(arr->content);
		arr->capacity = 0;
	} else {
		void** new_mem = realloc(arr->content, arr->size * sizeof(void*));
		check_mem(new_mem);
		arr->content = new_mem;
		arr->capacity = arr->size;
	}
error:
	return;
}

#endif
