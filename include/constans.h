#ifndef _CONSTANTS_
#define _CONSTANTS_

/* */
#define BUFF_CAP 1024

/* */
#define ETH_IP_PROTO 2048

/* IP frame */
#define IPV4_VERSION 4
#define IPV4_IHL     5
#define IPV4_TOS     0
#define IPV4_ID      25

/* */
#define IPV4_LEN 4
#define MAC_LEN  6

/* Frame offsets */
#define IP_FRAME_OFFSET   (sizeof(struct ether_header))
#define ARP_FRAME_OFFSET  (sizeof(struct ether_header))
#define ICMP_FRAME_OFFSET (sizeof(struct ether_header) + sizeof(struct iphdr))

/* Errors */
#define ALLOC_ERR   "Allocation error! Out of memory!"
#define PARSE_ERR   "Couldn't parse the table!"
#define GET_MSG_ERR "Couldn't get the packet!"

/* Routing tables */
#define RTABLE_CAP 100000

/* ARP table */
#define ARP_TABLE_CAP 100
#define ARP_TABLE_FILE_PATH "arp_table.txt"


#endif