#include "minunit.h"
#include "p32_list.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static List *list = NULL;
char *test1 = "test1 data";
char *test2 = "test2 data";
char *test3 = "test3 data";

char *test_create() {
  list = List_create();
  mu_assert(list != NULL, "Failed to create list.");

  return NULL;
}

char *test_destroy() {
  List_clear_destroy(list);

  return NULL;
}

char *test_push_pop() {
  List_push(list, test1);
  mu_assert(List_last(list) == test1, "Wrong last value.");

  List_push(list, test2);
  mu_assert(List_last(list) == test2, "Wrong last value");

  List_push(list, test3);
  mu_assert(List_last(list) == test3, "Wrong last value.");
  mu_assert(List_count(list) == 3, "Wrong count on push.");

  char *val = List_pop(list);
  mu_assert(val == test3, "Wrong value on pop.");

  val = List_pop(list);
  mu_assert(val == test2, "Wrong value on pop.");

  val = List_pop(list);
  mu_assert(val == test1, "Wrong value on pop.");
  mu_assert(List_count(list) == 0, "Wrong count after pop.");

  return NULL;
}

char *test_unshift() {
  List_unshift(list, test1);
  mu_assert(List_first(list) == test1, "Wrong first value.");

  List_unshift(list, test2);
  mu_assert(List_first(list) == test2, "Wrong first value");

  List_unshift(list, test3);
  mu_assert(List_first(list) == test3, "Wrong last value.");
  mu_assert(List_count(list) == 3, "Wrong count on unshift.");

  return NULL;
}

char *test_remove() {
  // we only need to test the middle remove case since push/shift
  // already tests the other cases

  char *val = List_remove(list, list->first->next);
  mu_assert(val == test2, "Wrong removed element.");
  mu_assert(List_count(list) == 2, "Wrong count after remove.");
  mu_assert(List_first(list) == test3, "Wrong first after remove.");
  mu_assert(List_last(list) == test1, "Wrong last after remove.");

  return NULL;
}

char *test_shift() {
  mu_assert(List_count(list) != 0, "Wrong count before shift.");

  char *val = List_shift(list);
  mu_assert(val == test3, "Wrong value on shift.");

  val = List_shift(list);
  mu_assert(val == test1, "Wrong value on shift.");
  mu_assert(List_count(list) == 0, "Wrong count after shift.");

  return NULL;
}

char *test_connect() {
  List *list_b = List_create();

  List_push(list, test1);
  List_push(list_b, test2);
  List_push(list_b, test3);
  List_connect(list, list_b);

  mu_assert(list->count == 3, "Wrong count");

  char *val = List_pop(list);
  mu_assert(val == test3, "Wrong last value");
  val = List_pop(list);
  mu_assert(val == test2, "Wrong last value");
  val = List_pop(list);
  mu_assert(val == test1, "Wrong last value");
  return NULL;
}
#define NUM_TESTS_SORT 16
#define NUM_MAX_LENGTH 10000
int cmp(const void *a, const void *b) { return *(int *)a - *(int *)b; }

char *test_sort() {
  List *list2 = List_create();
  List *list3 = List_create();
  int data[NUM_MAX_LENGTH], data_standard[NUM_MAX_LENGTH];
  int cnt_data;
  for (int i = 0; i < NUM_TESTS_SORT; i++) {
    if (i < NUM_TESTS_SORT / 2)
      cnt_data = rand() % NUM_MAX_LENGTH + 1;
    else
      cnt_data = rand() % 3 + 1;
    // cnt_data = 4000;
    for (int j = 0; j < cnt_data; j++) {
      data[j] = rand() % cnt_data;
      data_standard[j] = data[j];
      List_push(list, data + j);
      List_push(list2, data + j);
      List_push(list3, data + j);
    }
    // printf("%d %d\n", i, cnt_data);
    printf("Test %d with %d numbers\n", i + 1, cnt_data);

    clock_t ticks = clock();
    List_mergesort(list, cmp);
    double seconds = (double)(clock() - ticks) / CLOCKS_PER_SEC;
    printf("Merge sort : %.3f ms\n", 1000 * seconds);
    ticks = clock();
    List_bubblesort(list2, cmp);
    seconds = (double)(clock() - ticks) / CLOCKS_PER_SEC;
    printf("Bubble sort : %.3f ms\n", 1000 * seconds);
    ticks = clock();
    List_sort_from_bottom_to_top(list3, cmp);
    seconds = (double)(clock() - ticks) / CLOCKS_PER_SEC;
    printf("Merge sort FBTT : %.3f ms\n", 1000 * seconds);
    qsort(data_standard, cnt_data, sizeof(int), cmp);
    for (int j = cnt_data - 1; j >= 0; j--) {
      int val = *(int *)List_pop(list);
      int val2 = *(int *)List_pop(list2);
      int val3 = *(int *)List_pop(list3);
      mu_assert(val == data_standard[j], "Wrong value after mergesorting");
      mu_assert(val2 == data_standard[j], "Wrong value after bubblesorting");
      mu_assert(val3 == data_standard[j],
                "Wrong value after mergesorting FBTT");
    }
    puts("");
  }
  List_clear_destroy(list2);
  List_clear_destroy(list3);

  return NULL;
}

char *all_tests() {
  srand(1919);
  mu_suite_start();
  printf("%p %p %p\n", test1, test2, test3);
  mu_run_test(test_create);
  mu_run_test(test_push_pop);
  mu_run_test(test_connect);
  mu_run_test(test_sort);
  mu_run_test(test_unshift);
  mu_run_test(test_remove);
  mu_run_test(test_shift);
  mu_run_test(test_destroy);
  return NULL;
}

RUN_TESTS(all_tests);
