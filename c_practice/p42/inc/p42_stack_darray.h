#ifndef P42_STACK_H
#define P42_STACK_H

#include <stdlib.h>
#include <assert.h>
#include "darray.h"

typedef struct {
	DArray* container;
} Stack;

static inline Stack* Stack_create() {
	Stack* ret = malloc(sizeof(Stack));
	ret->container = DArray_create();
	return ret;
}

static inline void* Stack_pop(Stack* stack) {
	assert(stack->container->size != 0);
	return DArray_pop(stack->container);
}

static inline void Stack_push(Stack* stack, void* value) {
	DArray_push(stack->container, value);
}

static inline void Stack_destroy(Stack* stack) {
	DArray_destroy(stack->container);
	free(stack);
}

static inline int Stack_count(Stack* stack) {
	return stack->container->size;
}

static inline void* Stack_peek(Stack* stack) {
	return stack->container->content[stack->container->size - 1];
}


#endif
