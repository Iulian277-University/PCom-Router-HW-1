#include "skel.h"

#include "skel.h"

#ifndef _TRIE_
#define _TRIE_

/* The `trie` structure will have the following form. This is more an illustrative example,
   because the nodes aren't filled with 0/1 bits, but with a pointer to the `rtable_route`.
   You can imagine that the 0/1 bits are placed on the connections between the nodes.

      root
     /   \ 
    0     1
   / \   / \ 
  0   1 0   1
 .............
*/

struct Node {
    struct Node *left;  // bits of 0
    struct Node *right; // bits of 1
    struct route_table_entry *route;
};

struct Node *new_node();
void insert_rtable_entry(struct Node *root, struct route_table_entry *route);
struct route_table_entry *get_best_route_trie(struct Node *root, uint32_t dest_ip);
size_t count_set_bits(uint32_t number);
void free_trie(struct Node *root);

#endif
