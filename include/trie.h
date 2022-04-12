#include "skel.h"

#include "skel.h"

#ifndef _TRIE_
#define _TRIE_

struct Node {
    struct Node *left;
    struct Node *right;
    struct route_table_entry *route;
};

struct Node *new_node();
void add_route_entry(struct Node *root, struct route_table_entry *route);
struct route_table_entry *get_best_route_trie(struct Node *root, uint32_t dst_ip);

#endif
