#include <stdlib.h>
#include <assert.h>
#include "llist.h"

LList* llist_init() {
  LList* list = malloc(sizeof(LList));
  assert(list);
  list->head = NULL;
  list->count = 0;
  return list;
}

void llist_destroy(LList* list) {
  LListNode* node_free;
  LListNode* node = list->head;
  while (node) {
    node_free = node;
    node = node->next;
    free(node_free);
  }
  free(list);
}

void llist_destroy_with_values(LList* list) {
  LListNode* node_free;
  LListNode* node = list->head;
  while (node) {
    node_free = node;
    node = node->next;
    free(llist_node_value(node_free));
    free(node_free);
  }
  free(list);
}

void llist_insert(LList* list, void* value) {
  LListNode* node_ins = malloc(sizeof(LListNode));
  assert(node_ins);
  node_ins->value = value;
  node_ins->next = list->head;
  list->head = node_ins;
  list->count += 1;
}

void llist_insert_ordered(LList* list, void* value, int (*comparator)(const void*, const void*)) {
  LListNode* node_ins = malloc(sizeof(LListNode));
  assert(node_ins);
  node_ins->value = value;

  // empty list
  if (!list->head) {
    list->head = node_ins;
    node_ins->next = NULL;

  // inserted at head
  } else if (comparator(value, list->head->value) < 0) {
    node_ins->next = list->head;
    list->head = node_ins;

  // inserted after head
  } else {
    LListNode* node = list->head;
    while (node && node->next) {
      // inserted in middle
      if (comparator(value, node->next->value) < 0) {
        node_ins->next = node->next;
        node->next = node_ins;
        return;
      }
      node = node->next;
    }
    // inserted at tail
    node->next = node_ins;
    node_ins->next = NULL;
  }

  list->count += 1;
}

void llist_concat(LList* from_list, LList* to_list) {
  LListNode* from_tail = llist_tail(from_list);
  if (from_tail) {
    LListNode* old_head = to_list->head;
    to_list->head = from_list->head;
    from_tail->next = old_head;
    to_list->count += from_list->count;
  }
  free(from_list);
}

void llist_delete(LList* list, void* value) {
  LListNode* node_prev = NULL;
  LListNode* node = list->head;
  while (node) {
    if (node->value == value) {
      if (node_prev) {
        node_prev->next = node->next;
      } else {
        list->head = node->next;
      }
      list->count -= 1;
      free(node);
      return;
    } else {
      node_prev = node;
      node = node->next;
    }
  }
}

void llist_delete_by(LList* list, void* value, bool (*comparator)(const void*, const void*)) {
  LListNode* node_prev = NULL;
  LListNode* node = list->head;
  while (node) {
    if (comparator(node->value, value)) {
      if (node_prev) {
        node_prev->next = node->next;
      } else {
        list->head = node->next;
      }
      list->count -= 1;
      free(node);
      return;
    } else {
      node_prev = node;
      node = node->next;
    }
  }
}

int llist_count(LList* list) {
  return list->count;
}

LListNode* llist_head(LList* list) {
  return list->head;
}

LListNode* llist_tail(LList* list) {
  if (!list->head) {
    return NULL;
  } else {
    LListNode* node = llist_head(list);
    while (llist_node_next(node)) {
      node = llist_node_next(node);
    }
    return node;
  }
}

LListNode* llist_node_next(LListNode* node) {
  return node->next;
}

void* llist_node_value(LListNode* node) {
  return node->value;
}
