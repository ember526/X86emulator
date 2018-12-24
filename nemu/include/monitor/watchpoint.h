#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expression[200];
  uint32_t current_value;
  /* TODO: Add more members if necessary */


} WP;
WP* new_wp(char *expr);
void free_wp(int NO);
void print_wp();
bool if_changed();
#endif
