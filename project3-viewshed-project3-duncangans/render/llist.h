#ifndef __llist_h
#define __llist_h

#include <stdbool.h>

typedef struct llist_node_t {
  void*                value;
  struct llist_node_t* next;
} LListNode;

typedef struct llist_t {
  LListNode* head;
  int        count;
} LList;

LList*     llist_init();
void       llist_destroy(LList* llist);
void       llist_destroy_with_values(LList* llist);
void       llist_insert(LList* llist, void* value);
void       llist_insert_ordered(LList* list, void* value, int (*comparator)(const void*, const void*));
void       llist_concat(LList* from_list, LList* to_list);
void       llist_delete(LList* llist, void* value);
void       llist_delete_by(LList* llist, void* value, bool (*comparator)(const void*, const void*));
int        llist_count(LList* llist);
LListNode* llist_head(LList* llist);
LListNode* llist_tail(LList* llist);
LListNode* llist_node_next(LListNode* llist_node);
void*      llist_node_value(LListNode* llist_node);

#endif
