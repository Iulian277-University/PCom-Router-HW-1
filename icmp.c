#include "icmp.h"
#include "skel.h"
#include "constans.h"

/* This is the core function for manipulating ICMP packets */
void send_icmp(packet *msg, int icmp_type)
{
    // Extract the headers from the original `msg` of type [ETH][IPv4][ICMP]
    struct ether_header *eth_hdr  = (struct ether_header *) msg->payload;
    struct iphdr        *ip_hdr   = (struct iphdr *)       (msg->payload + IP_FRAME_OFFSET);
    struct icmphdr      *icmp_hdr = (struct icmphdr *)     (msg->payload + ICMP_FRAME_OFFSET);

    // Create a new packet of type [ETH][IPv4][ICMP]
	packet reply;
    memset(reply.payload, 0, sizeof(reply.payload));

    // Set `len` and `interface` fields of the packet
	reply.interface = msg->interface;
	reply.len = sizeof(struct ether_header) +
                sizeof(struct iphdr) +
                sizeof(struct icmphdr);

    // Complete the [ETH] frame
    struct ether_header *eth_hdr_reply = (struct ether_header *) reply.payload;
    // Interchange `src` IP with `dst` IP
    memcpy(eth_hdr_reply->ether_dhost, eth_hdr->ether_shost, MAC_LEN);
    memcpy(eth_hdr_reply->ether_shost, eth_hdr->ether_dhost, MAC_LEN);
    eth_hdr_reply->ether_type = htons(ETHERTYPE_IP);

    // Complete the [IP] frame
    struct iphdr *ip_hdr_reply = (struct iphdr *) (reply.payload + IP_FRAME_OFFSET);
    ip_hdr_reply->version  = IPV4_VERSION;
    ip_hdr_reply->ihl      = IPV4_IHL;
    ip_hdr_reply->tos      = IPV4_TOS;
    ip_hdr_reply->tot_len  = htons(reply.len - IP_FRAME_OFFSET);
    ip_hdr_reply->id       = htons(IPV4_ID);
    ip_hdr_reply->ttl      = ip_hdr->ttl;
    ip_hdr_reply->protocol = IPPROTO_ICMP;

    // Recompute the `checksum`
	ip_hdr_reply->check = 0;
	ip_hdr_reply->check = ip_checksum((uint8_t *) ip_hdr_reply, sizeof(struct iphdr));
    
    // Interchange `src` IP with `dst` IP
	ip_hdr_reply->daddr = ip_hdr->saddr;
	ip_hdr_reply->saddr = inet_addr(get_interface_ip(msg->interface));

    // Complete the [ICMP] frame
    struct icmphdr *icmp_hdr_reply  = (struct icmphdr *) (reply.payload + ICMP_FRAME_OFFSET);
    icmp_hdr_reply->type = icmp_type;
    icmp_hdr_reply->code = 0;
    if (icmp_type == ICMP_ECHOREPLY)
    {
        // Keep the same values for `id number` and `sequence number`
        icmp_hdr_reply->un.echo.id       = icmp_hdr->un.echo.id;
        icmp_hdr_reply->un.echo.sequence = icmp_hdr->un.echo.sequence;
    }
    else
    {
        icmp_hdr_reply->un.echo.id       = 0;
        icmp_hdr_reply->un.echo.sequence = 0;
    }
    
    // Recompute the ICMP frame checksum
    icmp_hdr_reply->checksum = 0;
    icmp_hdr->checksum = ip_checksum((uint8_t *) icmp_hdr_reply, sizeof(struct icmphdr));

    // Send the ICMP packet reply
    send_packet(&reply);
}
