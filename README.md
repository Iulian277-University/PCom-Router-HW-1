## Name: *Iulian-Marius Tăiatu (322CB)*
## Solved tasks: *[TODO]*
## Local score: *[TODO]*
---

# General presentation

**In this documentation, I will talk about some general ideas and the workflow of the program.**

**I implemented the forwarding process of a router, more exactly the behaviour of a router when it receives a packet and how the packets are send from the router.**


**First, I implemented the forwarding process, using a `static ARP table` and considering that all pairs of type `{IP, MAC}` are well-known a priori. After this, I started to work on the `ARP` functionality. If there was no tuple {IP, MAC} in the cached ARP table, an `arp_request` was sent to `everybody`. Until getting the `arp_reply`, next packets were inspected and the current `waiting_packet` was added in a `queue`. This allows the router to work normally, without needing to wait for that specific ARP reply.**

**For every received packet, the first step was to extract the `ETH frame`. Before inspecting the message more, I checked if the packet is of type `IPv4` or `ARP`. If isn't, then I dropped the packet, because in this project we are dealing only with `IPv4` and `ARP` messages.**

# IPv4
If the received message is an `IPv4` packet, then the following steps are done:
- Extract the `IP frame` from the `msg` (next frame from the `ETH header`)
- Then check if the message is for the router
    - **`For the router`**: Verify that the packet is of type `ICMP`
        - If it isn't an `ICMP` message, then drop the packet
        - Otherwise, extract the `ICMP header` and check if the type of the ICMP is `ECHO_REQUEST`. If that's the case, then send back an `ECHO_REPLY`
    - **`Not for the router`**: Here, the router isn't the destination of the packet, therefore the router needs to forward it
        - Before searching for the `next hop`, compute some sanity checks:
            - If the `checksum` of the packet is not zero, then the packet is broken, so drop it
            - If the `TTL` field (time to live) if 0 or 1, that means the packet has reached the maximum hops in the topology
        - If the packet is "still in the game", search for the best route in the `routing table`.
            - If there was no route found, then sent back an `ICMP` message of type `destination unreachable`.
            - Otherwise (the route was successfuly found), updat the TTL and checksum fields. Then complete the src/dest `MAC addresses`. For that, search in the ARP table
                - If not entry was found, then an `ARP request` was initiated and the current packet was added in a `queue`.
                - If the cached ARP table has the pair {IP, MAC} already in it, we just need to complete the src/dest MAC addresses and the interface through which the packet will go. After that, we can send the message.

# ARP
- Extract the `ARP frame` from the `msg` (next frame from the `ETH header`)
- Check if it's an `arp_request` or an `arp_reply`
    - `ARPOP_REQUEST`: Send and `arp_reply` by creating a new packet of type `[ETH][ARP]` and completing the ETH and ARP headers.
    - `ARPOP_REPLY`: Here the router has received an ARP reply message containing a struct of the following form: {IP, MAC}. Therefore, the router will insert this entry in the existing `ARP table`. After that, the router will traverse the `waiting_queue_of_pkts` and for every packet check if it has an IP - MAC correspondent
        - The packets for which the router received an arp reply in the meantime, are forwarded.
        - All the packets which didn't received and arp reply, are stored in an auxiliary queue, then at the end the original `waiting_pkts_queue` is reconstructed.

# ICMP
- For this section, the things are quite simple. A generic function can handle all kind of `ICMP type` (*ICMP_TIME_EXCEEDED, ICMP_DEST_UNREACH, ICMP_ECHO, ICMP_ECHOREPLY*)

- The packet looks like this: `[ETH][IPv4][ICMP]`. In this function, I completed each of those 3 frames, recomputed the checksum for the IP and ICMP headers and I kept the fields (setted by the host sender) `id number` and `sequence number` when the `type` of the ICMP packet was `ICMP_ECHOREPLY`.

# Longest prefix match

*[TODO]*

# Checksum via incremental update
Using the following function, I am able to recompute the checksum wihtout needing to recalculate it from zero.

This technique can be used only when substracting one from the TTL. It's equivalent with adding 1 or 256 as appropriate to the checksum field in the packet, using one's complement addition. [[RFC documentation](https://www.rfc-editor.org/rfc/pdfrfc/rfc1141.txt.pdf)]

```c
    uint16_t recalculate_checksum(struct iphdr *ip_hdr)
    {
        uint16_t sum = ntohs(ip_hdr->check) + 0x100;
        return htons(sum + (sum >> 16));
    }
