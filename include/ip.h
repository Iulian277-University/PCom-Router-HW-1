#ifndef _IP_
#define _IP_

void append_packet_to_waiting_queue(packet *msg);
void manipulate_ip_packet(packet *msg, struct ether_header *eth_hdr);

#endif