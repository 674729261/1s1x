#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Our old friend die from ex17. */
void die(const char *message) {
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  exit(1);
}

// a typedef creates a fake type, in this
// case for a function pointer
typedef int (*compare_cb)(int a, int b);
typedef int *(*sort_func)(int *, int, compare_cb);

/**
 * A classic bubble sort function that uses the
 * compare_cb to do the sorting.
 */
int *slow_sort(int *numbers, int count, compare_cb cmp) {
  int temp = 0;
  int i = 0;
  int j = 0;
  int *target = malloc(count * sizeof(int));

  if (!target)
    die("Memory error.");

  memcpy(target, numbers, count * sizeof(int));
  int sorted_flag = 0;
  while (!sorted_flag) {
    for (i = 0; i < count; i++) {
      int swap_index = rand() % (i + 1);
      temp = target[swap_index];
      target[swap_index] = target[i];
      target[i] = temp;
    }
    sorted_flag = 1;
    for (i = 0; i < count - 1; i++) {
      if (cmp(target[i], target[i + 1]) > 0)
        sorted_flag = 0;
    }
  }
  return target;
}

int *bubble_sort(int *numbers, int count, compare_cb cmp) {
  int temp = 0;
  int i = 0;
  int j = 0;
  int *target = malloc(count * sizeof(int));

  if (!target)
    die("Memory error.");

  memcpy(target, numbers, count * sizeof(int));

  for (i = 0; i < count; i++) {
    for (j = 0; j < count - 1; j++) {
      if (cmp(target[j], target[j + 1]) > 0) {
        temp = target[j + 1];
        target[j + 1] = target[j];
        target[j] = temp;
      }
    }
  }

  return target;
}

int sorted_order(int a, int b) { return a - b; }

int reverse_order(int a, int b) { return b - a; }

int strange_order(int a, int b) {
  if (a == 0 || b == 0) {
    return 0;
  } else {
    return a % b;
  }
}

/**
 * Used to test that we are sorting things correctly
 * by doing the sort and printing it out.
 */
void test_sorting(int *numbers, int count, compare_cb cmp, sort_func srt) {
  int i = 0;
  clock_t begin = clock();
  int *sorted = srt(numbers, count, cmp);
  double duration = (double)(clock() - begin) / CLOCKS_PER_SEC;

  if (!sorted)
    die("Failed to sort as requested.");
  printf("Time : %fms\n", duration * 1000);
  for (i = 0; i < count; i++) {
    printf("%d ", sorted[i]);
  }
  printf("\n");

  free(sorted);

  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    die("USAGE: ex18 4 3 1 5 6");
  srand(114);
  int count = argc - 1;
  int i = 0;
  char **inputs = argv + 1;

  int *numbers = malloc(count * sizeof(int));
  if (!numbers)
    die("Memory error.");

  for (i = 0; i < count; i++) {
    numbers[i] = atoi(inputs[i]);
  }

  printf("Use slow sort:\n");
  test_sorting(numbers, count, sorted_order, slow_sort);
  test_sorting(numbers, count, reverse_order, slow_sort);
  test_sorting(numbers, count, strange_order, slow_sort);
  printf("Use bubble sort:\n");
  test_sorting(numbers, count, sorted_order, bubble_sort);
  test_sorting(numbers, count, reverse_order, bubble_sort);
  test_sorting(numbers, count, strange_order, bubble_sort);

  free(numbers);

  return 0;
}
