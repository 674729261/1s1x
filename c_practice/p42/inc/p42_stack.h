#ifndef P42_STACK_H
#define P42_STACK_H

#include <stdlib.h>
#include <assert.h>
#include "p32_list.h"

typedef struct {
	List* container;
} Stack;

static inline Stack* Stack_create() {
	Stack* ret = malloc(sizeof(Stack));
	ret->container = List_create();
	return ret;
}

static inline void* Stack_pop(Stack* stack) {
	assert(stack->container->count != 0);
	return List_pop(stack->container);
}

static inline void Stack_push(Stack* stack, void* value) {
	List_push(stack->container, value);
}

static inline void Stack_destroy(Stack* stack) {
	List_destroy(stack->container);
	free(stack);
}

static inline int Stack_count(Stack* stack) {
	return stack->container->count;
}

static inline void* Stack_peek(Stack* stack) {
	return stack->container->last->value;
}

#define STACK_FOREACH(S,C) for(ListNode* C = S->container->first; C != NULL; C = C->next)


#endif
