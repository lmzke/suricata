/* Copyright (c) 2008 Victor Julien <victor@inliniac.net> */

#ifndef __DECODE_IPV4_H__
#define __DECODE_IPV4_H__

#include <sys/types.h>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#define IPV4_HEADER_LEN           20
#define IPV4_OPTMAX               40
#define	IPV4_MAXPACKET_LEN        65535 /* maximum packet size */

#define IPV4_OPT_EOL              0x00
#define IPV4_OPT_NOP              0x01
#define IPV4_OPT_RR               0x07
#define IPV4_OPT_RTRALT           0x94
#define IPV4_OPT_TS               0x44
#define IPV4_OPT_SECURITY         0x82
#define IPV4_OPT_LSRR             0x83

typedef struct IPV4Hdr_
{
    u_int8_t ip_verhl;     /* version & header length */
    u_int8_t ip_tos;
    u_int16_t ip_len;      /* length */
    u_int16_t ip_id;       /* id */
    u_int16_t ip_off;      /* frag offset */
    u_int8_t ip_ttl;
    u_int8_t ip_proto;     /* protocol (tcp, udp, etc) */
    u_int16_t ip_csum;     /* checksum */
    struct in_addr ip_src;
    struct in_addr ip_dst;
} IPV4Hdr;

#define IPV4_GET_RAW_VER(ip4h)            (((ip4h)->ip_verhl & 0xf0) >> 4)
#define IPV4_GET_RAW_HLEN(ip4h)           ((ip4h)->ip_verhl & 0x0f)
#define IPV4_GET_RAW_IPTOS(ip4h)          ((ip4h)->ip_tos)
#define IPV4_GET_RAW_IPLEN(ip4h)          ((ip4h)->ip_len)
#define IPV4_GET_RAW_IPID(ip4h)           ((ip4h)->ip_id)
#define IPV4_GET_RAW_IPOFFSET(ip4h)       ((ip4h)->ip_off)
#define IPV4_GET_RAW_IPTTL(ip4h)          ((ip4h)->ip_ttl)
#define IPV4_GET_RAW_IPPROTO(ip4h)        ((ip4h)->ip_proto)

/* we need to change them as well as get them */
#define IPV4_SET_RAW_VER(ip4h, value)     ((ip4h)->ip_verhl = (((ip4h)->ip_verhl & 0x0f) | (value << 4)))
#define IPV4_SET_RAW_HLEN(ip4h, value)    ((ip4h)->ip_verhl = (((ip4h)->ip_verhl & 0xf0) | (value & 0x0f)))
#define IPV4_SET_RAW_IPTOS(ip4h, value)   ((ip4h)->ip_tos = value)
#define IPV4_SET_RAW_IPLEN(ip4h, value)   ((ip4h)->ip_len = value)
#define IPV4_SET_RAW_IPPROTO(ip4h, value) ((ip4h)->ip_proto = value)

/* this is enough since noone will access the cache without first
 * checking the flags */
#define IPV4_CACHE_INIT(p)                (p)->ip4c.flags = 0

/* ONLY call these functions after making sure that:
 * 1. p->ip4h is set
 * 2. p->ip4h is valid (len is correct)
 * 3. cache is initialized
 */
#define IPV4_GET_VER(p) \
    ((p)->ip4c.flags & IPV4_CACHE_VER ? \
    (p)->ip4c.ver : ((p)->ip4c.flags |= IPV4_CACHE_VER, (p)->ip4c.ver = IPV4_GET_RAW_VER((p)->ip4h)))
#define IPV4_GET_HLEN(p) \
    ((p)->ip4c.flags & IPV4_CACHE_HLEN ? \
    (p)->ip4c.hl : ((p)->ip4c.flags |= IPV4_CACHE_HLEN, (p)->ip4c.hl = IPV4_GET_RAW_HLEN((p)->ip4h) << 2))
#define IPV4_GET_IPTOS(p) \
     IPV4_GET_RAW_IPTOS(p)
#define IPV4_GET_IPLEN(p) \
    ((p)->ip4c.flags & IPV4_CACHE_IPLEN ? \
    (p)->ip4c.ip_len : ((p)->ip4c.flags |= IPV4_CACHE_IPLEN, (p)->ip4c.ip_len = ntohs(IPV4_GET_RAW_IPLEN((p)->ip4h))))
#define IPV4_GET_IPID(p) \
    ((p)->ip4c.flags & IPV4_CACHE_IPID ? \
    (p)->ip4c.ip_id : ((p)->ip4c.flags |= IPV4_CACHE_IPID, (p)->ip4c.ip_id = ntohs(IPV4_GET_RAW_IPID((p)->ip4h))))
/* _IPV4_GET_IPOFFSET: get the content of the offset header field in host order */
#define _IPV4_GET_IPOFFSET(p) \
    ((p)->ip4c.flags & IPV4_CACHE__IPOFF ? \
    (p)->ip4c._ip_off : ((p)->ip4c.flags |= IPV4_CACHE__IPOFF, (p)->ip4c._ip_off = ntohs(IPV4_GET_RAW_IPOFFSET((p)->ip4h))))
/* IPV4_GET_IPOFFSET: get the final offset */
#define IPV4_GET_IPOFFSET(p) \
    ((p)->ip4c.flags & IPV4_CACHE_IPOFF ? \
    (p)->ip4c.ip_off : ((p)->ip4c.flags |= IPV4_CACHE_IPOFF, (p)->ip4c.ip_off = _IPV4_GET_IPOFFSET(p) & 0x1fff))
/* IPV4_GET_RF: get the RF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_RF(p) \
    ((p)->ip4c.flags & IPV4_CACHE_RF ? \
    (p)->ip4c.rf : ((p)->ip4c.flags |= IPV4_CACHE_RF, (p)->ip4c.rf = (u_int8_t)((_IPV4_GET_IPOFFSET((p)) & 0x8000) >> 15)))
/* IPV4_GET_DF: get the DF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_DF(p) \
    ((p)->ip4c.flags & IPV4_CACHE_DF ? \
    (p)->ip4c.df : ((p)->ip4c.flags |= IPV4_CACHE_DF, (p)->ip4c.df = (u_int8_t)((_IPV4_GET_IPOFFSET((p)) & 0x4000) >> 14)))
/* IPV4_GET_MF: get the MF flag. Use _IPV4_GET_IPOFFSET to save a ntohs call. */
#define IPV4_GET_MF(p) \
    ((p)->ip4c.flags & IPV4_CACHE_MF ? \
    (p)->ip4c.mf : ((p)->ip4c.flags |= IPV4_CACHE_MF, (p)->ip4c.mf = (u_int8_t)((_IPV4_GET_IPOFFSET((p)) & 0x2000) >> 13)))
#define IPV4_GET_IPTTL(p) \
     IPV4_GET_RAW_IPTTL(p)
#define IPV4_GET_IPPROTO(p) \
    ((p)->ip4c.flags & IPV4_CACHE_IPPROTO ? \
    (p)->ip4c.ip_proto : ((p)->ip4c.flags |= IPV4_CACHE_IPPROTO, (p)->ip4c.ip_proto = IPV4_GET_RAW_IPPROTO((p)->ip4h)))

#define IPV4_CACHE_VER                    0x0001 /* 1 */
#define IPV4_CACHE_HLEN                   0x0002 /* 2 */
#define IPV4_CACHE_IPTOS                  0x0004 /* 4 */
#define IPV4_CACHE_IPLEN                  0x0008 /* 8 */
#define IPV4_CACHE_IPID                   0x0010 /* 16 */
#define IPV4_CACHE_IPOFF                  0x0020 /* 32 */
#define IPV4_CACHE__IPOFF                 0x0040 /* 64 */
#define IPV4_CACHE_RF                     0x0080 /* 128*/
#define IPV4_CACHE_DF                     0x0100 /* 256 */
#define IPV4_CACHE_MF                     0x0200 /* 512 */
#define IPV4_CACHE_IPTTL                  0x0400 /* 1024*/
#define IPV4_CACHE_IPPROTO                0x0800 /* 2048 */

/**
 * IPv4 decoder cache
 *
 * Used for storing parsed values.
 */
typedef struct IPV4Cache_
{
    u_int16_t flags;

    u_int8_t ver;
    u_int8_t hl;
    u_int8_t ip_tos;        /* type of service */
    u_int16_t ip_len;       /* datagram length */
    u_int16_t ip_id;        /* identification  */
    u_int16_t ip_off;       /* fragment offset */
    u_int16_t _ip_off;      /* fragment offset - full field value, host order*/
    u_int8_t rf;
    u_int8_t df;
    u_int8_t mf;
    u_int8_t ip_ttl;        /* time to live field */
    u_int8_t ip_proto;      /* datagram protocol */
    u_int16_t ip_csum;      /* checksum */
    u_int32_t ip_src_u32;   /* source IP */
    u_int32_t ip_dst_u32;   /* dest IP */

} IPV4Cache;

/* helper structure with parsed ipv4 info */
typedef struct IPV4Vars_
{
    u_int8_t ip_opts_len;
} IPV4Vars;

#endif /* __DECODE_IPV4_H__ */

