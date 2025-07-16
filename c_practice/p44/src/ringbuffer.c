#include "stdlib.h"
#include "ringbuffer.h"
#include "dbg.h"

RingBuffer* RingBuffer_create(int length) {
	RingBuffer* ret = malloc(sizeof(RingBuffer));
	ret->length = length + 1;
	ret->buffer=malloc(ret->length * sizeof(char));
	ret->start = ret->end = 0;
	return ret;
}

void RingBuffer_destroy(RingBuffer* rb) {
	free(rb->buffer);
	free(rb);
}

int __RingBuffer_freeSpace(RingBuffer* rb) {
	if(rb->end >= rb->start)
		return rb->length - (rb->end - rb->start) - 1;
	return rb->start - rb->end - 1;
}

int __RingBuffer_dataLength(RingBuffer* rb) {
	if(rb->end >= rb->start)
		return rb->end - rb->start;
	return rb->length - (rb->start - rb->end);
}

int RingBuffer_read(RingBuffer* rb, char *target, int amount) {
	check(__RingBuffer_dataLength(rb) >= amount, "Not enough data to read");
	if(amount + rb->start <= rb->length) {
		memcpy(target, rb->buffer + rb->start, amount);
	} else {
		int first_half = rb->length - rb->start;
		memcpy(target, rb->buffer + rb->start, first_half);
		memcpy(target + first_half, rb->buffer, amount - first_half);
	}
	rb->start = (rb->start + amount) % rb->length;
	return amount;
error:
	return -1;
}

int RingBuffer_write(RingBuffer* rb, const char *data, int amount) {
	check(__RingBuffer_freeSpace(rb) >= amount, "Not enough space to write");
	if(amount + rb->end <= rb->length) {
		memcpy(rb->buffer + rb->end, data, amount);
	} else {
		int first_half = rb->length - rb->end;
		memcpy(rb->buffer + rb->end, data, first_half);
		memcpy(rb->buffer, data + first_half, amount - first_half);
	}
	rb->end = (rb->end + amount) % rb->length;
	return amount;
error:
	return -1;
}

bstring RingBuffer_gets(RingBuffer* rb, int amount) {
	check(amount > 0, "Need more than 0 for gets, you gave: %d ", amount);

	char* temp = malloc(amount * sizeof(char));
	int rc = RingBuffer_read(rb, temp, amount);
	if(rc == -1)
	{
		free(temp);
		goto error;
	}
	bstring ret = blk2bstr(temp, amount);
	free(temp);
	check(ret != NULL, "Failed to create gets result.");
	check(blength(ret) == amount, "Wrong result length.");
	return ret;
error:
	return NULL;
}
