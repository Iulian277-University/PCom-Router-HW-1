#include "skel.h"
#include "trie.h"

#ifndef _RTABLE_
#define _RTABLE_

int rtable_comparator(const void *rtable_entry_1, const void *rtable_entry_2);
struct route_table_entry *get_best_route(struct route_table_entry *rtable, int rtable_len, in_addr_t dest_ip);
struct route_table_entry *get_best_route_log(struct route_table_entry *rtable, int rtable_len, in_addr_t dest_ip);
void print_rtable_entry(struct route_table_entry *entry, FILE *fp);
void print_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp);
// void search_entry_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp, struct Node *root);
void search_entry_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp, struct Node *root);

#endif
