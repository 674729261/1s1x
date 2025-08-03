#ifndef List_h
#define List_h

#include <stdlib.h>

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;

List *List_create();
void List_destroy(List *list);
void List_clear(List *list);
void List_clear_destroy(List *list);
void List_connect(List *list_a, List *list_b);

typedef int (*Comparer)(const void*, const void*);

void List_mergesort(List *list, Comparer cmp);
void List_bubblesort(List *list, Comparer cmp);
void List_sort_from_bottom_to_top(List *list, Comparer cmp);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)
#define List_check(A) ((A)->count >= 0 && ((A)->count == 0 || (A)->first != NULL))
void List_push(List *list, void *value);
void *List_pop(List *list);

void List_unshift(List *list, void *value);
void *List_shift(List *list);

void *List_remove(List *list, ListNode *node);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
