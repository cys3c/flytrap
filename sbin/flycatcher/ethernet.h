/*-
 * Copyright (c) 2016 Universitetet i Oslo
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef FLYCATCHER_ETHERNET_H_INCLUDED
#define FLYCATCHER_ETHERNET_H_INCLUDED

typedef union { uint8_t o[6]; } __attribute__((__packed__)) ether_addr;
typedef union { uint8_t o[4]; uint32_t q; } __attribute__((__packed__)) ipv4_addr;
typedef union { uint8_t o[16]; uint16_t w[8]; } __attribute__((__packed__)) ipv6_addr;

#define FLYCATCHER_ETHER_ADDR { 0x02, 0x00, 0x18, 0x11, 0x09, 0x02 }
extern ether_addr flycatcher_ether_addr;

typedef enum ether_type {
	ether_type_ip	 = 0x0800,
	ether_type_arp	 = 0x0806,
	ether_type_vlan	 = 0x8100,
	ether_type_ipv6	 = 0x86dd,
} ether_type;

typedef struct ether_hdr {
	ether_addr	 dst;
	ether_addr	 src;
	uint16_t	 type;
} __attribute__((__packed__)) ether_hdr;

typedef struct ether_ftr {
	uint32_t	 fcs;
} __attribute__((__packed__)) ether_ftr;

typedef enum arp_oper {
	arp_oper_who_has = 1,
	arp_oper_is_at	 = 2,
} arp_oper;

typedef enum arp_type {
	arp_type_ether	 = 1,
	arp_type_ipv4	 = 0x0800,
} arp_type;

typedef struct arp_pkt {
	uint16_t	 htype;
	uint16_t	 ptype;
	uint8_t		 hlen;
	uint8_t		 plen;
	uint16_t	 oper;
	ether_addr	 sha;
	ipv4_addr	 spa;
	ether_addr	 tha;
	ipv4_addr	 tpa;
} __attribute__((__packed__)) arp_pkt;

typedef struct ip_hdr {
#define ip_hdr_ver(ih) ((ih)->ver_ihl >> 4)
#define ip_hdr_ihl(ih) ((ih)->ver_ihl & 0xf)
	uint8_t		 ver_ihl;
#define ip_hdr_dscp(ih) ((ih)->dscp_ecn >> 2)
#define ip_hdr_ecn(ih) ((ih)->dscp_ecn & 0x3)
	uint8_t		 dscp_ecn;
	uint16_t	 len;
	uint16_t	 id;
#define ip_hdr_fl(ih) (be16toh((ih)->fl_off) >> 2)
#define ip_hdr_off(ih) (be16toh((ih)->fl_off) & 0x3)
	uint16_t	 fl_off;
	uint8_t		 ttl;
	uint8_t		 proto;
	uint8_t		 sum;
	ipv4_addr	 srcip;
	ipv4_addr	 dstip;
	uint8_t		 opt[];
} __attribute__((__packed__)) ip_hdr;

int	 arp_reserve(const ipv4_addr *);
int	 arp_find(const ipv4_addr *, ether_addr *);

uint32_t ether_crc32(const uint8_t *, size_t);

char	*ipv4_fromstr(const char *, ipv4_addr *);

int	 packet_analyze_ethernet(struct packet *, const void *, size_t);
int	 packet_analyze_arp(struct packet *, const void *, size_t);
int	 packet_analyze_ip(struct packet *, const void *, size_t);
int	 packet_analyze_icmp(struct packet *, const void *, size_t);
int	 ethernet_send(struct iface *, ether_type, ether_addr *, const void *, size_t);

#endif