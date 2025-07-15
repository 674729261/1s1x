#include <assert.h>
#include "p32_list.h"
#include "dbg.h"

List *List_create() { return calloc(1, sizeof(List)); }

void List_destroy(List *list) {
  assert(list);
  LIST_FOREACH(list, first, next, cur) {
	if (cur->prev) {
	  free(cur->prev);
	}
  }

  free(list->last);
  free(list);
}

void List_clear(List *list) {
  assert(list);
  LIST_FOREACH(list, first, next, cur) { free(cur->value); }
}

void List_clear_destroy(List *list) {
  // List_clear(list);
  // List_destroy(list);
  assert(list);
  LIST_FOREACH(list, first, next, cur) {
	  free(cur->value);
	  if (cur->prev)
		  free(cur->prev);
  }
  free(list->last);
  free(list);
}

void List_push(List *list, void *value) {
  assert(list);
  ListNode *node = calloc(1, sizeof(ListNode));
  check_mem(node);

  node->value = value;

  if (list->last == NULL) {
	list->first = node;
	list->last = node;
  } else {
	list->last->next = node;
	node->prev = list->last;
	list->last = node;
  }

  list->count++;

error:
  return;
}

void *List_pop(List *list) {
  assert(list);
  ListNode *node = list->last;
  return node != NULL ? List_remove(list, node) : NULL;
}

void List_unshift(List *list, void *value) {
  assert(list);
  ListNode *node = calloc(1, sizeof(ListNode));
  check_mem(node);

  node->value = value;

  if (list->first == NULL) {
	list->first = node;
	list->last = node;
  } else {
	node->next = list->first;
	list->first->prev = node;
	list->first = node;
  }

  list->count++;
}

void *List_shift(List *list) {
  assert(list);
  ListNode *node = list->first;
  return node != NULL ? List_remove(list, node) : NULL;
}

void *List_remove(List *list, ListNode *node) {
  assert(list);
  void *result = NULL;

  check(list->first && list->last, "List is empty.");
  check(node, "node can't be NULL");

  if (node == list->first && node == list->last) {
	list->first = NULL;
	list->last = NULL;
  } else if (node == list->first) {
	list->first = node->next;
	check(list->first != NULL,
		  "Invalid list, somehow got a first that is NULL.");
	list->first->prev = NULL;
  } else if (node == list->last) {
	list->last = node->prev;
	check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
	list->last->next = NULL;
  } else {
	ListNode *after = node->next;
	ListNode *before = node->prev;
	after->prev = before;
	before->next = after;
  }

  list->count--;
  result = node->value;
  free(node);

error:
  return result;
}

void List_connect(List *list_a, List *list_b) {
	assert(list_a && list_b && list_a != list_b);
	
	if (list_a->count == 0) {
		list_a->first = list_b->first;
		list_a->last = list_b->last;
		list_a->count = list_b->count;
		free(list_b);
		return;

	}
	list_a->last->next = list_b->first;
	list_b->first->prev = list_a->last;
	list_a->last = list_b->last;
	list_a->count += list_b->count;
	free(list_b);
}
// sort [left, right]

void __List_merge(ListNode **left, ListNode **right, ListNode *p_mid, Comparer cmp) {
	ListNode *m = p_mid->next;
	ListNode *end_pos = (*right)->next;
	ListNode *l = *left;
	ListNode *r = *right;
	while(m != end_pos && l != m) {
		// printf("%p %p %p %p\n", m, left, end_pos, right);
		if(cmp(l->value, m->value) < 0)
			l = l->next;
		else {
			ListNode *next_m = m->next;
			if(*right == m)
				*right = p_mid;
			if(*left == l)
				*left = m;
			if(m->next)
				m->next->prev = p_mid;
			
			p_mid->next = m->next;
			m->next = l;
			m->prev = l->prev;
			if(l->prev)
				l->prev->next = m;
			l->prev = m;
			m = next_m;
		}
	}
}


void __List_sort(ListNode **left, ListNode **right, Comparer cmp) {
	ListNode *p_mid = *left;
	ListNode *p_helper = *left;
	int a = 0, b = 0;
	while(p_helper != *right) {
		p_helper = p_helper->next;
		if(p_helper != *right) {
			p_helper = p_helper->next;
			p_mid = p_mid->next;
		}
	}

	if(p_mid == p_helper)
		return;  // COUNT(list) is 1
	
//	printf("%d %d\n", a, b);
	__List_sort(left, &p_mid, cmp);
	__List_sort(&(p_mid->next), right, cmp);

	//LIST_FOREACH(list, first, next, cur) {
	//	printf("%p ", cur);
	//}
	//putchar('\n');
	__List_merge(left, right, p_mid, cmp);
}

void List_bubblesort(List *list, Comparer cmp) {
	assert(list);
	int curi = 0, curj = 0;
	for(curi = 0; curi < list->count - 1; curi++) {
		curj = 0;
		for(ListNode *j = list->first; curj < list->count - 1 - curi; j = j->next, curj++) {
			if(cmp(j->value, j->next->value) > 0) {
				void *tmp = j->value;
				j->value = j->next->value;
				j->next->value = tmp;
			}
		}
	}
}

void List_sort_from_bottom_to_top(List* list, Comparer cmp) {
	int step = 1;
	while (step < list->count) {

		ListNode* l = list->first, * r = l, * p_mid = l;
		for (int i = 0; i < step - 1; i++) {
			p_mid = p_mid->next;
			r = r->next;
		}
		for (int i = 0; i < step && r != list->last; i++)
			r = r->next;
		int n_iter = (list->count + step - 1) / (step * 2);
		for (int i = 0; i < n_iter; i++) {
			int l_eq_first = (l == list->first);
			int r_eq_last = (r == list->last);
			__List_merge(&l, &r, p_mid, cmp);
			if(l_eq_first)
				list->first = l;
			if(r_eq_last)
				list->last = r;

			if (i != n_iter - 1) {
				for (int j = 0; j < step * 2; j++) {
					l = l->next;
					if (r != list->last)
						r = r->next;
				}
				p_mid = l;
				for (int i = 0; i < step - 1; i++) {
					p_mid = p_mid->next;
				}
			}

		}
		step *= 2;
	}
}

void List_mergesort(List *list, Comparer cmp) {
	assert(list);
	if(list->count <= 1)
		return;
	__List_sort(&(list->first), &(list->last), cmp);
}

