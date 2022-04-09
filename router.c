#include "skel.h"
#include "rtable.h"
#include "queue.h"
#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "constans.h"

// General usage buffer
char buff[BUFF_CAP];

// Routing table
struct route_table_entry *rtable;
int rtable_len;

// ARP table
struct arp_entry *arp_table;
int arp_table_len;

// Queue of `waiting_pkts` for L2 addresses (ARP replies)
queue waiting_pkts;


/*
* [TODO]
*  		 O(log(n)) routing table
*/


int main(int argc, char *argv[])
{
	// Init the program
	packet msg;
	int rc;
	init(argc - 2, argv + 2);

	// Alloc `rtable`
	rtable = (struct route_table_entry  *) calloc(RTABLE_CAP, sizeof(struct route_table_entry));
	DIE(rtable == NULL, ALLOC_ERR);

	// Parse `rtable`
	rtable_len  = parse_rtable(argv[1], rtable);
	DIE(rtable_len  < 1, PARSE_ERR);

	// Alloc `arp_table`
	arp_table = (struct arp_entry *) calloc(ARP_TABLE_CAP, sizeof(struct arp_entry));
	DIE(arp_table == NULL, ALLOC_ERR);

	// Sort the `routing_table`: O(nlog(n))
	qsort(rtable, rtable_len, sizeof(struct route_table_entry), rtable_comparator);
	

	// FILE *fp = fopen("rtable0_sorted.txt", "w");
	// print_rtable(rtable, rtable_len, fp);
	// fclose(fp);
	// // return -1;
	// search_entry_rtable(rtable, rtable_len, stdout);
	// return -1;


	// Initialize the `waiting_pkts` queue
	waiting_pkts = queue_create();

	// Router loop
	while (1)
	{
		rc = get_packet(&msg);
		DIE(rc < 0, GET_MSG_ERR);

		// Extract the [Ethernet] frame from the `msg`
		struct ether_header *eth_hdr = (struct ether_header *) msg.payload;

		/* -- Sanity check -- */
		// We are dealing only with IPv4 and ARP packets
		if ((ntohs(eth_hdr->ether_type) != ETHERTYPE_IP) &&
		   ((ntohs(eth_hdr->ether_type) != ETHERTYPE_ARP)))
			continue;

		/* -- Check if we have an IPv4 of ARP packet -- */
		if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP)
			manipulate_ip_packet(&msg, eth_hdr);
		else if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP)
			manipulate_arp_packet(&msg);
	}

	// Release the memory
	free(rtable);
	free(arp_table);
	return 0;
}
