#ifndef P42_QUEUE_H
#define P42_QUEUE_H

#include <stdlib.h>
#include <assert.h>
#include "p32_list.h"

typedef struct {
	List* container;
} Queue;

static inline Queue* Queue_create() {
	Queue* ret = malloc(sizeof(Queue));
	ret->container = List_create();
	return ret;
}

static inline void* Queue_recv(Queue* queue) {
	assert(queue->container->count != 0);
	return List_remove(queue->container, queue->container->first);
}

static inline void Queue_send(Queue* queue, void* value) {
	List_push(queue->container, value);
}

static inline void Queue_destroy(Queue* queue) {
	List_destroy(queue->container);
	free(queue);
}

static inline int Queue_count(Queue* queue) {
	return queue->container->count;
}

static inline void* Queue_peek(Queue* queue) {
	return queue->container->first->value;
}

#define QUEUE_FOREACH(S,C) for(ListNode* C = S->container->first; C != NULL; C = C->next)


#endif
