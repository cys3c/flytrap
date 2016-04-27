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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/time.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fc/endian.h>
#include <fc/log.h>

#include "flycatcher.h"
#include "ethernet.h"
#include "iface.h"
#include "packet.h"

int
packet_analyze_ethernet(struct packet *p, const void *data, size_t len)
{
	const ether_hdr *eh;
	int ret;

	if (len < sizeof(ether_hdr)) {
		fc_notice("%d.%03d short Ethernet packet (%zd < %zd)",
		    p->ts.tv_sec, p->ts.tv_usec / 1000,
		    len, sizeof(ether_hdr));
		return (-1);
	}
	eh = data;
	data = eh + 1;
	len -= sizeof *eh;
	fc_debug("%d.%03d recv type %04x packet "
	    "from %02x:%02x:%02x:%02x:%02x:%02x "
	    "to %02x:%02x:%02x:%02x:%02x:%02x",
	    p->ts.tv_sec, p->ts.tv_usec / 1000, be16toh(eh->type),
	    eh->src.o[0], eh->src.o[1], eh->src.o[2],
	    eh->src.o[3], eh->src.o[4], eh->src.o[5],
	    eh->dst.o[0], eh->dst.o[1], eh->dst.o[2],
	    eh->dst.o[3], eh->dst.o[4], eh->dst.o[5]);
	switch (be16toh(eh->type)) {
	case ether_type_arp:
		ret = packet_analyze_arp(p, data, len);
		break;
	case ether_type_ip:
		ret = packet_analyze_ip(p, data, len);
		break;
	default:
		ret = -1;
	}
	return (ret);
}

int
ethernet_send(struct iface *i, ether_type type, ether_addr *dst, const void *data, size_t len)
{
	struct packet p;
	ether_hdr *eh;
	int ret;

	p.i = i;
	p.len = sizeof *eh + len;
	if ((eh = malloc(p.len)) == NULL)
		return (-1);
	p.data = eh;
	memcpy(&eh->dst, dst, sizeof eh->dst);
	memcpy(&eh->src, &i->ether, sizeof eh->src);
	eh->type = htobe16(type);
	memcpy(eh + 1, data, len);
	gettimeofday(&p.ts, NULL);
	fc_debug("%d.%03d send type %04x packet "
	    "from %02x:%02x:%02x:%02x:%02x:%02x "
	    "to %02x:%02x:%02x:%02x:%02x:%02x",
	    p.ts.tv_sec, p.ts.tv_usec / 1000, be16toh(eh->type),
	    eh->src.o[0], eh->src.o[1], eh->src.o[2],
	    eh->src.o[3], eh->src.o[4], eh->src.o[5],
	    eh->dst.o[0], eh->dst.o[1], eh->dst.o[2],
	    eh->dst.o[3], eh->dst.o[4], eh->dst.o[5]);
	ret = iface_transmit(i, &p);
	free(eh);
	return (ret);
}
