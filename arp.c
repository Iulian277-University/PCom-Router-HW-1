#include "arp.h"
#include "skel.h"
#include "rtable.h"
#include "queue.h"
#include "constans.h"

extern struct route_table_entry *rtable;
extern int rtable_len;

extern struct arp_entry *arp_table;
extern int arp_table_len;

extern queue waiting_pkts;

/* This is the core function for manipulating ARP packets */
void manipulate_arp_packet(packet *msg)
{
	// Extract the [ARP] frame
	struct arp_header *arp_hdr = (struct arp_header *) (msg->payload + ARP_FRAME_OFFSET);

	// Check if it's an `ARP request` or an `ARP reply`
	if (ntohs(arp_hdr->op) == ARPOP_REQUEST)
		send_arp_reply(msg);
	else if (ntohs(arp_hdr->op) == ARPOP_REPLY)
	{
		cache_arp_entry(msg);
		traverse_waiting_packets(waiting_pkts);
	}
}

/* Traverse all the packets from the `waiting_pkts` queue
   If we received the `ARP reply` for that given packet, then forward the packet */
void traverse_waiting_packets(queue waiting_pkts)
{
	// Temporary queue for being able to reconstruct the
	// `waiting_pkts` queue which didn't get an `arp_reply` yet
	queue q = queue_create();
	
	// For each packet from the `waiting_pkts` queue, check if we received an ARP reply
	while (!queue_empty(waiting_pkts))
	{
		packet *msg = queue_deq(waiting_pkts);
		
		// Extract the [Ethernet] frame from the `msg`
		struct ether_header *eth_hdr = (struct ether_header *) msg->payload;
		
		// Extract the [IP] frame from the `msg`
    	struct iphdr *ip_hdr = (struct iphdr *) (msg->payload + IP_FRAME_OFFSET);

		struct arp_entry *arp_pair = get_arp_entry(arp_table, arp_table_len, ip_hdr->daddr);
		// Didn't received an ARP reply yet
		if (arp_pair == NULL)
			queue_enq(q, msg);
		else
		{
			// Overwrite the dest MAC addresses (`eth_hdr->ether_dhost`)
			memcpy(eth_hdr->ether_dhost, arp_pair->mac, MAC_LEN);

			// Overwrite the src  MAC addresses (`eth_hdr->ether_shost`)
			struct route_table_entry *route = get_best_route(rtable, rtable_len, ip_hdr->daddr);
			// struct route_table_entry *route = get_best_route_log(rtable, rtable_len, ip_hdr->daddr);
			get_interface_mac(route->interface, eth_hdr->ether_shost);
			
			// Set the interface through which the packet will go
			msg->interface = route->interface;
			
			// Send the message
			send_packet(msg);
		}
	}

	// Reconstruct the `waiting_pkts` queue
	while (!queue_empty(q))
	{
		packet *msg = queue_deq(q);
		queue_enq(waiting_pkts, msg);
	}
}

/* Add a new entry in the `arp_table`
   This isn't a real cache memory, because all the entries are permanent */
void cache_arp_entry(packet *msg)
{
	struct arp_header *arp_hdr  = (struct arp_header *) (msg->payload + ARP_FRAME_OFFSET);
	arp_table[arp_table_len].ip = arp_hdr->spa;
	memcpy(&arp_table[arp_table_len].mac, &arp_hdr->sha, MAC_LEN);
	arp_table_len++;
}

/* This function takes an IPv4 address and returns a struct {IP, MAC} */
struct arp_entry *get_arp_entry(struct arp_entry *arp_table, int arp_table_len, in_addr_t ip)
{
	for (int i = 0; i < arp_table_len; ++i)
	{
		if (arp_table[i].ip == ip)
			return &arp_table[i];
	}
	return NULL;
}

/* Send an ARP request throught the `route` for finding
   the target MAC address for the given `daddr` IP */
void send_arp_request(struct route_table_entry *route, in_addr_t daddr)
{
	// Create a new packet of type [ETH][ARP]
	packet req;

	// Set `len` and `interface` fields of the packet
	req.len = sizeof(struct ether_header) + sizeof(struct arp_header);
	req.interface = route->interface;

	// Complete the [ETH] frame
	struct ether_header *eth_hdr = (struct ether_header *) req.payload;
	// Set the `eth_type` to `0x0806 (ARP)`
	eth_hdr->ether_type = htons(ETHERTYPE_ARP);
	// Complete the `dest_addr` with the `broadcast mac address` (everybody needs to listen)
	memset(eth_hdr->ether_dhost, 0xFF, MAC_LEN);
	// Complete the `src_addr`
	get_interface_mac(route->interface, eth_hdr->ether_shost);

	// Complete the [ARP] frame
	struct arp_header *arp_hdr = (struct arp_header *) (req.payload + ARP_FRAME_OFFSET);
	arp_hdr->htype = htons(1);
	arp_hdr->ptype = htons(ETH_IP_PROTO);
	arp_hdr->hlen  = MAC_LEN;
	arp_hdr->plen  = IPV4_LEN;
	arp_hdr->op    = htons(ARPOP_REQUEST);

	// Complete the `sender_hardware_addr`
	get_interface_mac(route->interface, arp_hdr->sha);
	// Set the `target_hardware_addr` to 0 (this is the field we want to receive)
	memset(&arp_hdr->tha, 0, MAC_LEN);
	// Complete the `sender` and `target` IP addresses
	in_addr_t sender_ip_addr = inet_addr(get_interface_ip(route->interface));
	memcpy(&arp_hdr->spa, &sender_ip_addr, IPV4_LEN);
	memcpy(&arp_hdr->tpa, &daddr,          IPV4_LEN);

	// Send the packet
	send_packet(&req);
}

/* Send an ARP reply with the MAC address completed */
void send_arp_reply(packet *msg)
{	
	// Create a new packet of type [ETH][ARP]
	packet rep;

	// Set `len` and `interface` fields of the packet
	rep.len = sizeof(struct ether_header) + sizeof(struct arp_header);
	rep.interface = msg->interface;

	// Complete the [ETH] frame
	struct ether_header *eth_hdr       = (struct ether_header *) msg->payload;
	struct ether_header *eth_hdr_reply = (struct ether_header *) rep.payload;
	eth_hdr_reply->ether_type = eth_hdr->ether_type;
	get_interface_mac(msg->interface, eth_hdr_reply->ether_shost);
	memcpy(eth_hdr_reply->ether_dhost, eth_hdr->ether_shost, MAC_LEN);

	// Complete the [ARP] frame
	struct arp_header *arp_hdr       = (struct arp_header *) (msg->payload + ARP_FRAME_OFFSET);
	struct arp_header *arp_hdr_reply = (struct arp_header *) (rep.payload  + ARP_FRAME_OFFSET);
	arp_hdr_reply->htype = htons(1);
	arp_hdr_reply->ptype = htons(ETH_IP_PROTO);
	arp_hdr_reply->hlen  = MAC_LEN;
	arp_hdr_reply->plen  = IPV4_LEN;
	arp_hdr_reply->op    = htons(ARPOP_REPLY);

	// Complete the `sender` and `target` hardware addresses
	get_interface_mac(msg->interface, arp_hdr_reply->sha);
	memcpy(&arp_hdr_reply->tha, &arp_hdr->sha, MAC_LEN);
	
	// Interchange `src` IP with `dst` IP
	memcpy(&arp_hdr_reply->spa, &arp_hdr->tpa, IPV4_LEN);
	memcpy(&arp_hdr_reply->tpa, &arp_hdr->spa, IPV4_LEN);

	send_packet(&rep);
}
