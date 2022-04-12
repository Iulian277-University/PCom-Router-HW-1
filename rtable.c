#include "rtable.h"
#include "constans.h"
#include "trie.h"

// General usage buffer
extern char buff[BUFF_CAP];

/* Returns a pointer to the best matching route or NULL if there is no matching route */
/* O(n) time complexity */
struct route_table_entry *get_best_route(struct route_table_entry *rtable, int rtable_len, in_addr_t dest_ip)
{
	int idx = -1;
	for (int i = 0; i < rtable_len; ++i)
	{
		if ((dest_ip & rtable[i].mask) == rtable[i].prefix)
		{
			if (idx == -1)
				idx = i;
			else if (ntohl(rtable[idx].mask) < ntohl(rtable[i].mask))
				idx = i;
		}
	}

	if (idx == -1)
		return NULL;

	return &rtable[idx];
}

/* Helper function used for printing an entry
   from the routing table to the given FILE pointer */
void print_rtable_entry(struct route_table_entry *entry, FILE *fp)
{
	fprintf(fp, "%s ", inet_ntop(AF_INET, &entry->prefix, 	buff, INET_ADDRSTRLEN));
	memset(buff, 0, strlen(buff));
	fprintf(fp, "%s ", inet_ntop(AF_INET, &entry->next_hop, buff, INET_ADDRSTRLEN));
	memset(buff, 0, strlen(buff));
	fprintf(fp, "%s ", inet_ntop(AF_INET, &entry->mask, 	buff, INET_ADDRSTRLEN));
	memset(buff, 0, strlen(buff));
	fprintf(fp, "%d\n", entry->interface);
}

/* Helper function used for printing the routing table to the given FILE pointer */
void print_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp)
{
	for(int i = 0; i < rtable_len; ++i)
		print_rtable_entry(&rtable[i], fp);
}

/* Helper function used for searching an entry in the routing table
   using both the linear and logarithmic search */
void search_entry_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp, struct Node *root)
{
    // Search for an entry in the `rtable` with/without using `binary_search`
	uint32_t dest_ip;
	inet_pton(AF_INET, "192.168.2.0", &dest_ip);
	inet_ntop(AF_INET, &dest_ip, buff, INET_ADDRSTRLEN);
	printf("%s\n", buff);
	struct route_table_entry *route      = get_best_route(rtable, rtable_len, dest_ip);
	struct route_table_entry *route_trie = get_best_route_trie(root, dest_ip);
	print_rtable_entry(route, fp);
	print_rtable_entry(route_trie, fp);
}
