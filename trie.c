#include "trie.h"
#include "constans.h"

/* Alloc memory for a new `trie node`, filling the pointers with NULL */
struct Node *new_node()
{
    struct Node *node = (struct Node *) calloc(1, sizeof(struct Node));
    node->left  = NULL;
    node->right = NULL;
	node->route = NULL;
    return node;
}

/* Count the number of set bits (bits of 1) given a 32 bits number */
size_t count_set_bits(uint32_t number)
{
	size_t count = 0;
	while (number)
	{
		count += number & 1;
		number >>= 1;
	}
	return count;
}

/* Insert a new `route` in the trie, given the `root` of it */
void insert_rtable_entry(struct Node *root, struct route_table_entry *route)
{
	// Extract the `prefix` and `mask` from the given `route`
	uint32_t route_prefix = ntohl(route->prefix);
	uint32_t route_mask   = ntohl(route->mask);

	// Traverse all relevant bits and insert them in the trie
	uint32_t bit_mask = 1 << 31;
	for(int i = 0; i < MIN(count_set_bits(route_mask), IPV4_ADDR_LEN); ++i)
	{
		uint32_t curr_bit = route_prefix & bit_mask;

		// Insert the current node as left/right child 
		if (curr_bit == 0)
		{
			// Check if the node already exists (otherwise, create it)
			if (!root->left)
				root->left = new_node();
			
			// Move the root to the current inserted bit
			root = root->left;
		}
		else
		{
			// Check if the node already exists (otherwise, create it)
			if (!root->right)
				root->right = new_node();
			
			// Move the root to the current inserted bit
			root = root->right;
		}
		
		// Move to the next bit
		bit_mask >>= 1;
	}

	// At the end, save the pointer to the `rtable entry`
	root->route = route;
}

/* Search for the best `route` in the trie given a `dest_ip` address */
struct route_table_entry *get_best_route_trie(struct Node *root, uint32_t dest_ip)
{
	// Don't return the `best_route` immediately when the first route was found
	// in the trie. We want to find the most specific route (stepping more and more in the trie)
	struct route_table_entry *best_route = NULL;

	// Start from the first bit and traverse the tree
	uint32_t bit_mask = 1 << 31;

	// Convert from network order (BE) to host order (LE)
	dest_ip = ntohl(dest_ip);

	// Traverse the trie
	while (bit_mask && root)
	{
		// Update the `best_route` 
		if (root->route)
			best_route = root->route;
		
		// Move to the left/right depending on the current bit
		uint32_t curr_bit = dest_ip & bit_mask;
		if (curr_bit == 0)
			root = root->left;
		else
			root = root->right;

		// Move to the next bit in the `dest_ip`
		bit_mask >>= 1;
	}

	// Return the best route found (or NULL if no route was found)
	return best_route;
}

void free_trie(struct Node *root)
{
	// Safe guard including root node
	if (root == NULL)
		return;

	// Recursive case
    free_trie(root->left);
    free_trie(root->right);

	// Base case
	free(root);
}