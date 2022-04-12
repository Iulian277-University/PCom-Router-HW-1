#include "trie.h"

// [TODO]: Reformat + Doc

struct Node *new_node()
{
    struct Node *node = (struct Node *) calloc(1, sizeof(struct Node));
    node->left  = NULL;
    node->right = NULL;
	node->route = NULL;
    return node;
}

void add_route_entry(struct Node *root, struct route_table_entry *route)
{
	uint32_t bit_mask = 1 << 31;

	uint32_t route_prefix = ntohl(route->prefix);
	uint32_t subnet_mask  = ntohl(route->mask);

	while (bit_mask != 0 && subnet_mask != 0)
	{
		uint32_t bit_val = bit_mask & route_prefix;

		if (bit_val == 0)
		{
			if (root->left == NULL)
				root->left = new_node();
			root = root->left;
		}
		else
		{
			if (root->right == NULL)
				root->right = new_node();
			root = root->right;
		}
		
		bit_mask    >>= 1;
		subnet_mask <<= 1;
	}

	root->route = route;
}

struct route_table_entry *get_best_route_trie(struct Node *root, uint32_t dst_ip)
{
	uint32_t bit_mask = 1 << 31;
	struct route_table_entry *best_route = NULL;

	dst_ip = ntohl(dst_ip);

	while (bit_mask != 0 && root != NULL)
	{
		if (root->route != NULL)
			best_route = root->route;
		
		uint32_t bit_val = bit_mask & dst_ip;
		if (bit_val == 0)
			root = root->left;
		else
			root = root->right;

		bit_mask >>= 1;
	}

	return best_route;
}

