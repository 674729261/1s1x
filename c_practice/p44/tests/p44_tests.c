#include <string.h>
#include "minunit.h"
#include "ringbuffer.h"
#include <assert.h>

static RingBuffer *rb = NULL;



char *test_create()
{
    rb = RingBuffer_create(8);
    mu_assert(rb != NULL, "Failed to create stack.");

    return NULL;
}

char *test_destroy()
{
    mu_assert(rb != NULL, "Failed to make stack #2");
    RingBuffer_destroy(rb);

    return NULL;
}

char *test_write_read()
{
	const char data[16] = "abcdefgh";
	char buffer[16] = {};
	
	RingBuffer_write(rb, data, 5);
	RingBuffer_read(rb, buffer, 3);
	mu_assert(strcmp(buffer, "abc") == 0, "Read Error");
	RingBuffer_write(rb, data + 2, 6);
	RingBuffer_read(rb, buffer, 4);
	mu_assert(strcmp(buffer, "decd") == 0, "Read Error");
	RingBuffer_read(rb, buffer, 4);
	mu_assert(strcmp(buffer, "efgh") == 0, "Read Error");
	RingBuffer_write(rb, data, 8);
	RingBuffer_read(rb, buffer, 8);
	mu_assert(strcmp(buffer, "abcdefgh") == 0, "Read Error");	
    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_write_read);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);
