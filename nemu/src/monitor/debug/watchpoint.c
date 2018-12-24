#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].expression[0] = '\0';
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *expression)  {
	bool success = true;
  	uint32_t value = expr(expression, &success);
  	if(!success) {
  	  printf("Invalid expression : %s\n", expression);
  	  return NULL;
  	}
	Assert(free_, "Too many watchpoints!");
	WP *new_point = free_;
	free_ = new_point->next;
	new_point->next = head;
	head = new_point;
	strncpy(new_point->expression, expression, 200);
	new_point->current_value = value;
	return new_point;
}

void free_wp(int NO) {
	if (NO >= NR_WP || NO <0) {
		printf("Invalid watchpoint number\n");
		return;
	}
	WP * iter = head, *to_free = NULL;
	if(head->NO == NO) {
		to_free = head;
		head = head->next;
	}
	else
		while (iter->next) {
			if(iter->next ->NO  == NO) {
				to_free = iter->next;
				iter->next = to_free->next;
				break;
			}
			iter = iter->next;
		}
	if(to_free) {
		to_free->next = free_;
		free_ = to_free;
		to_free->expression[0] = '\0';
	}
	return;
}
 void print_wp() {
 	WP *i = head;
    if(i == NULL) {
      printf("No watchpoints\n");
      return;
    }
    printf("%-5s\t%-15s\t%-s\n", "No.", "Expr", "Value");
    while(i) {
      printf("%-5d\t%-15s\t0x%-x / %-d\n", i->NO, i->expression, i->current_value, i->current_value);
      i = i->next; 
    }
 }

 bool if_changed() {
 	bool has_changed = false;
 	WP *iter = head;
 	while(iter) {
 		bool success = true;
 		uint32_t new_value = expr(iter->expression, &success);
 		if(new_value != iter->current_value) {
 			printf("No.%d watchpoint (%s) has changed. Previous value:0x%x/%d, new value:0x%x/%d\n"
 				, iter->NO, iter->expression, iter->current_value, iter->current_value, new_value, new_value);
 			has_changed = true;
 			iter->current_value = new_value;
 		}
 		iter = iter->next;
 	}
 	return has_changed;
 }