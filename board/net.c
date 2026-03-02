#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include "thread.h"
#include "ztimer.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"

#include "net.h"

#define MSG_QUEUE_SIZE 8               // a reasonable queue size
#define PROTOCOL 253                   // raw ipv6 packet
#define MULTICAST_ADDRESS "FF12::5120" // just a magic address
#define POWER 8                        // measured maximum transmit power

void init_net(void) {
    // join custom multicast group
    ipv6_addr_t addr = { 0 };
    if (!ipv6_addr_from_str(&addr, MULTICAST_ADDRESS)) {
        printf("ERROR: \"%s\" is not a valid ipv6 address\n", MULTICAST_ADDRESS);
        abort();
    }
    gnrc_netif_t *netif = gnrc_netif_iter(NULL);
    if (!netif) {
        puts("ERROR: cannot find the network interface");
        abort();
    }
    int res = gnrc_netif_ipv6_group_join(netif, &addr);
    if (res < 0) {
        printf("ERROR: cannot join the multicast group, error code is %d\n", res);
        abort();
    }

    // set the maximum transmit power
    const int16_t power = POWER;
    if (gnrc_netapi_set(netif->pid, NETOPT_TX_POWER, 0, &power, sizeof(power))) {
        puts("ERROR: cannot set the transmit power");
        abort();
    }
}

void *listener(void *arg) {
    (void)arg;

    // init current thread's message queue for asynchronous communication
    msg_t msg_queue[MSG_QUEUE_SIZE];
    msg_init_queue(msg_queue, MSG_QUEUE_SIZE);

    // register the server for the protocol
    gnrc_netreg_entry_t server = {
        .demux_ctx = PROTOCOL,
        .target.pid = thread_getpid(),
        .next = NULL,
    };
    if (gnrc_netreg_register(GNRC_NETTYPE_IPV6, &server)) {
        puts("ERROR: unable to register the protocol");
        abort();
    }

    // handle packets
    while (1) {
        // get the packet
        msg_t msg;
        msg_receive(&msg);
        gnrc_pktsnip_t *pkt = msg.content.ptr;

        // extract headrs
        ipv6_hdr_t *ip_hdr = NULL;
        gnrc_netif_hdr_t *mac_hdr = NULL;
        if (pkt->next) {
            ip_hdr = pkt->next->data;
            if (pkt->next->next) {
                mac_hdr = pkt->next->next->data;
            } else {
                puts("WARNING: packet does not have the link-layer headr");
                gnrc_pktbuf_release(pkt);
                continue;
            }
        } else {
            puts("WARNING: packet does not have the ipv6 headr");
            gnrc_pktbuf_release(pkt);
            continue;
        }

        // get source ip address
        char src[IPV6_ADDR_MAX_STR_LEN];
        if (!ipv6_addr_to_str(src, &ip_hdr->src, sizeof(src))) {
            puts("WARNING: failed to convert ipv6 address");
            gnrc_pktbuf_release(pkt);
            continue;
        }

        // print log
        printf("addr: %s RSSI: %d LQI: %d\n", src, mac_hdr->rssi, mac_hdr->lqi);

        // release the packet!!! or memory leaks
        gnrc_pktbuf_release(pkt);
    }

    // never reached
    gnrc_netreg_unregister(GNRC_NETTYPE_IPV6, &server);
    return NULL;
}

void *sender(void *arg) {
    struct SenderController *typed_arg = arg;
    const thread_flags_t thread_flags = typed_arg->thread_flags;
    int *const flag = typed_arg->flag;

    ipv6_addr_t addr = { 0 };
    if (!ipv6_addr_from_str(&addr, MULTICAST_ADDRESS)) {
        printf("ERROR: \"%s\" is not a valid ipv6 address\n", MULTICAST_ADDRESS);
        abort();
    }

    // empty payload is not allowed, just take a shit
    char const payload[] = "shit";

    while (1) {
        thread_flags_wait_any(thread_flags);
        while (*flag) {
            // build the packet
            gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(
                NULL,
                payload,
                sizeof(payload) - 1,
                GNRC_NETTYPE_UNDEF);
            // build ipv6 header
            pkt = gnrc_ipv6_hdr_build(pkt, NULL, &addr);
            if (!pkt) {
                puts("ERROR: failed to build the ipv6 header");
                abort();
            }
            // set the protocol number
            ((ipv6_hdr_t *)(pkt->data))->nh = PROTOCOL;
            // build netif header
            gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
            if (netif_hdr == NULL) {
                puts("ERROR: failed to build the netif header");
                abort();
            }
            // set the netif
            gnrc_netif_hdr_set_netif(netif_hdr->data, gnrc_netif_iter(NULL));
            pkt = gnrc_pkt_prepend(pkt, netif_hdr);

            // send the packet
            if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_IPV6, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
                puts("ERROR: missing ipv6 module");
                abort();
            }

            // packets will be droped if the frequency is higher
            ztimer_sleep(ZTIMER_MSEC, 5);
        }
    }

    // never reached
    return NULL;
}
