#include "rtable.h"
#include "constans.h"

// General usage buffer
extern char buff[BUFF_CAP];

/* `qsort` comparator for sorting the `routing_table`
	Sort ascending by `prefix` and descending by `mask` */
int rtable_comparator(const void *rtable_entry_1, const void *rtable_entry_2)
{
	struct route_table_entry *e1 = (struct route_table_entry *) rtable_entry_1;
	struct route_table_entry *e2 = (struct route_table_entry *) rtable_entry_2;

	// if (e1->prefix == e2->prefix)
	// {
	// 	if (e1->mask < e2->mask)
	// 		return  1;
	// 	else
	// 		return -1;
	// }
	// else
	// {
	// 	if (e1->prefix > e2->prefix)
	// 		return  1;
	// 	else
	// 		return -1;
	// }

	// return 0;

	if (e1->prefix == e2->prefix)
		return ntohl(e2->mask) - ntohl(e1->mask);
	return ntohl(e1->prefix) - ntohl(e2->prefix);

	// return ntohl(e1->prefix & e1->mask) - ntohl(e2->prefix & e2->mask);
}

/* Returns a pointer to the best matching route or NULL if there is no matching route */
/* ~O(log(n)) time complexity */
// [TODO]: Repair this function
struct route_table_entry *get_best_route_log(struct route_table_entry *rtable, int rtable_len, in_addr_t dest_ip)
{
	int low = 0;
	int high = rtable_len - 1;

	while (low <= high)
	{
		int mid = low + (high - low) / 2;

		// Found the prefix
		if ((dest_ip & rtable[mid].prefix) == rtable[mid].prefix)
		{
			// printf("found\n");
			// return &rtable[mid];
			// Search the most specific mask
			uint32_t prefix = rtable[mid].prefix;
			while(rtable[mid].prefix == prefix)
				mid--;
			return &rtable[mid + 1];
		}
		
		if (ntohl((dest_ip & rtable[mid].mask)) < ntohl(rtable[mid].prefix))
		{
			// printf("here2: ");
			// print_rtable_entry(&rtable[mid], stdout);
			high = mid - 1;
		}
		else
		{
			// printf("here3: ");
			// print_rtable_entry(&rtable[mid], stdout);
			low  = mid + 1;
		}
	}

	return NULL;
}

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
	// fprintf(fp, "--- Routing table ---\n");
	for(int i = 0; i < rtable_len; ++i)
		print_rtable_entry(&rtable[i], fp);
}

/* Helper function used for searching an entry in the routing table
   using both the linear and logarithmic search */
void search_entry_rtable(struct route_table_entry *rtable, int rtable_len, FILE *fp)
{
    // Search for an entry in the `rtable` with/without using `binary_search`
	uint32_t dest_ip;
	inet_pton(AF_INET, "192.168.2.0", &dest_ip);
	inet_ntop(AF_INET, &dest_ip, buff, INET_ADDRSTRLEN);
	printf("%s\n", buff);
	struct route_table_entry *route     = get_best_route(rtable, rtable_len, dest_ip);
	struct route_table_entry *route_log = get_best_route_log(rtable, rtable_len, dest_ip);
	print_rtable_entry(route, fp);
	print_rtable_entry(route_log, fp);
}
