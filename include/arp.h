#ifndef _ARP_TABLE_
#define _ARP_TABLE_

void manipulate_arp_packet(packet *msg);
void cache_arp_entry(packet *msg);
void traverse_waiting_packets(queue waiting_pkts);
struct arp_entry *get_arp_entry(struct arp_entry *arp_table, int arp_table_len, in_addr_t ip);
void send_arp_request(struct route_table_entry *best_route, in_addr_t daddr);
void send_arp_reply(packet *msg);

#endif