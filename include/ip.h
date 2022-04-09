#include "skel.h"

#ifndef _IP_
#define _IP_

uint16_t recalculate_checksum(struct iphdr *ip_hdr);
void append_packet_to_waiting_queue(packet *msg);
void manipulate_ip_packet(packet *msg, struct ether_header *eth_hdr);

#endif