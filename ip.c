#include "ip.h"
#include "skel.h"
#include "rtable.h"
#include "arp.h"
#include "icmp.h"
#include "queue.h"
#include "constans.h"

extern struct route_table_entry *rtable;
extern int rtable_len;

extern struct arp_entry *arp_table;
extern int arp_table_len;

extern queue waiting_pkts;

// uint16_t update_cheksum(uint16_t old)
// {
// 	uint16_t sum = old + 0x100; // increment checksum high byte
// 	return (sum + (sum >> 16)); // add carry
// }

/* Create a new packet and add it to the `waiting_pkts` queue */
void append_packet_to_waiting_queue(packet *msg)
{
	packet pkt;
	pkt.len = msg->len;
	pkt.interface = msg->interface;
	memcpy(&pkt.payload, &msg->payload, MAX_LEN);
	queue_enq(waiting_pkts, &pkt);
}

/* This is the core function for manipulating IPv4 packets */
void manipulate_ip_packet(packet *msg, struct ether_header *eth_hdr)
{
    // Extract the [IP] frame from the `msg` (next frame from the [ETH] header)
    struct iphdr *ip_hdr = (struct iphdr *) (msg->payload + IP_FRAME_OFFSET);

    // Check if the `msg` is for the router (router is the `dest` of the `msg`)
    if (inet_addr(get_interface_ip(msg->interface)) == ip_hdr->daddr)
    {
        // We are dealing only with `ICMP` messages
        if (ip_hdr->protocol != IPPROTO_ICMP)
            return;

        /* -- Here, we have to deal with an ICMP packet -- */
        // Therefore, the frames will have the following form: [ETHER][IPv4][ICMP]
        // Extract the [ICMP] header from the `msg`
        struct icmphdr *icmp_hdr = (struct icmphdr *) (msg->payload + ICMP_FRAME_OFFSET);

        // Let's check if the type is `echo request (8)`
        if (icmp_hdr->type == ICMP_ECHO)
        {
            send_icmp(msg, ICMP_ECHOREPLY);
            return;
        }
    }

    /* -- Here, the packet isn't for the router (the router needs to forward it) -- */
    // Checksum
    if (ip_checksum((uint8_t *) ip_hdr, sizeof(struct iphdr)) != 0)
        return;

    // Time to live
    if (ip_hdr->ttl < 2)
    {
        send_icmp(msg, ICMP_TIME_EXCEEDED);
        return;
    }

    // Find the `best_route`
    // struct route_table_entry *route = get_best_route_log(rtable, rtable_len, ip_hdr->daddr); // O(log(n))
    struct route_table_entry *route = get_best_route(rtable, rtable_len, ip_hdr->daddr);        // O(n)
    if (route == NULL)
    {
        send_icmp(msg, ICMP_DEST_UNREACH);
        return;
    }

    // We have now computed the `best_route`
    // Descrease TTL
    ip_hdr->ttl--;
    // Recompute checksum (to recompute the checksum, we need to start with the `checksum` = 0)
    ip_hdr->check = 0;
    ip_hdr->check = ip_checksum((uint8_t *) ip_hdr, sizeof(struct iphdr));

    /* Now, we have to complete the src/dest MAC addresses */
    // Search the IP address in the `arp_table`
    struct arp_entry *arp_pair = get_arp_entry(arp_table, arp_table_len, ip_hdr->daddr);
    // If not found, send and `arp_request`
    if (arp_pair == NULL)
    {
        // `send_arp_request` for finding the pair (IP, MAC)
        send_arp_request(route, ip_hdr->daddr);
        // Add packet in queue until we will get an `arp_reply`, then move to the next packet
        // We don't have to wait until we get an ARP reply for that specific packet
        append_packet_to_waiting_queue(msg);
        return;
    }

    /* -- Here we have the MAC address stored in cache, we don't need to make an `arp_request` -- */

    // Overwrite the dest MAC addresses (`eth_hdr->ether_dhost`)
    memcpy(eth_hdr->ether_dhost, arp_pair->mac, MAC_LEN);

    // Overwrite the src  MAC addresses (`eth_hdr->ether_shost`)
    get_interface_mac(route->interface, eth_hdr->ether_shost);

    // Set the interface through which the packet will go
    msg->interface = route->interface;

    // Send the message
    send_packet(msg);
}