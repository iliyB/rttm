//#include <linux/rbtree.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/time.h>
#include "rbtree.c"

#define u "%u"
#define TRUE 1
#define FALSE 0

struct node *root = NULL;

MODULE_AUTHOR("Ilia Krekerrr");
MODULE_DESCRIPTION("Check delay packets");
MODULE_LICENSE("GPL");


unsigned int hook_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {

    struct iphdr *ip;
    struct tcphdr *tcp;
    //struct tcp_sock *s;
    //struct tcp_options_received opt;

    struct timespec spec;
    getnstimeofday(&spec);

    ip = (struct iphdr *)skb_network_header(skb);
    tcp = (struct tcphdr *)skb_transport_header(skb);
    //s = (struct tcp_sock *)skb;
    //opt = s->rx_opt;
    
    if ((tcp->ack) && (tcp->ack_seq) && (tcp->seq)) {
        uint32_t saddr, daddr;
        uint16_t sport, dport;
        uint16_t seq, ack_seq;
        u_char sstr[16], ssport[8];
        u_char dstr[48], sdport[8];
        u_char sseq[8], sack_seq[16]; 

        saddr = (uint32_t)ntohl(ip->saddr);
        daddr = (uint32_t)ntohl(ip->daddr);
        sport = (uint16_t)ntohs(tcp->source);
        dport = (uint16_t)ntohs(tcp->dest); 

        seq = (uint16_t)ntohs(tcp->seq);
        ack_seq = (uint16_t)ntohs(tcp->ack_seq);

        sprintf(sstr, u, saddr);
        sprintf(ssport, u, sport);
        strcat(sstr, ssport);

        sprintf(dstr, u, daddr);
        sprintf(sdport, u, dport);
        strcat(dstr, sdport);
        strcat(dstr, sstr);

        sprintf(sseq, u, seq);
        sprintf(sack_seq, u, ack_seq);
        strcat(sack_seq, sseq);
        strcat(dstr, sack_seq);
        
        struct node *old = search(root, dstr);
        
        if ((old) && (old->use)) {
            
            ktime_t current_time, old_time, result_time;
                char b1, b2, b3, b4;
                char sbuf[52], dbuf[52];
                
                current_time = spec.tv_sec * 1000000000 + spec.tv_nsec;
                old_time = old->spec.tv_sec * 1000000000 + old->spec.tv_nsec;
                result_time = current_time - old_time;

                const int NBYTES = 4;
                uint8_t ss[NBYTES], dd[NBYTES];
                int8_t i = 0;
                while (i < NBYTES) {
                    ss[i] = daddr >> (i * 8);
                    dd[i] = saddr >> (i * 8);
                    i++;
                }
                sprintf(sbuf, "%d.%d.%d.%d:%u", ss[3], ss[2], ss[1], ss[0], dport);
                sprintf(dbuf, "%d.%d.%d.%d:%u", dd[3], dd[2], dd[1], dd[0], sport);

                printk("%s -> %s, rtt = %u ns", sbuf, dbuf, result_time);
                old->use = FALSE;
        }

    }
    
    return NF_ACCEPT;
}

unsigned int hook_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip;
    struct tcphdr *tcp;
    //struct tcp_sock *s;
    //struct tcp_options_received opt;

    struct timespec spec;
    getnstimeofday(&spec);

    ip = (struct iphdr *)skb_network_header(skb);
    tcp = (struct tcphdr *)skb_transport_header(skb);
    //s = (struct tcp_sock *)skb;
    //opt = s->rx_opt;


    if ((tcp->ack) && (tcp->ack_seq) && (tcp->seq)) {
        uint32_t saddr, daddr;
        uint16_t sport, dport;          
        uint16_t seq, ack_seq;
        u_char sstr[48], ssport[8];
        u_char dstr[16], sdport[8];
        u_char sseq[16], sack_seq[8]; 

        saddr = (uint32_t)ntohl(ip->saddr);
        daddr = (uint32_t)ntohl(ip->daddr);
        sport = (uint16_t)ntohs(tcp->source);
        dport = (uint16_t)ntohs(tcp->dest);

        seq = (uint16_t)ntohs(tcp->seq);
        ack_seq = (uint16_t)ntohs(tcp->ack_seq);

        sprintf(sstr, u, saddr);
        sprintf(ssport, u, sport);
        strcat(sstr, ssport);

        sprintf(dstr, u, daddr);
        sprintf(sdport, u, dport);
        strcat(dstr, sdport);
        strcat(sstr, dstr);

        sprintf(sseq, u, seq);
        sprintf(sack_seq, u, ack_seq);
        strcat(sseq, sack_seq);
        strcat(sstr, sseq);
        
        struct node *new = search(root, sstr);

        if ((new) && (!new->use)) {
            new->use = TRUE;
            new->spec = spec;
        }
        else if (!new) {
            new = kmalloc(sizeof(struct node), GFP_KERNEL);
            memcpy(new->key, &sstr, 48);
            new->spec = spec;
            new->left = NULL;
            new->right = NULL;
            new->parent = NULL;
            new->color = RED;
            root = insert(root, new);
        }
    }
    return NF_ACCEPT;
}



struct nf_hook_ops bundle_pre_hook, bundle_out_hook, bundle_in_hook;

static int __init hook_init(void) {
    printk(KERN_INFO "Init my module\n");

    struct net *n;
    
    bundle_in_hook.hook = hook_in;
    bundle_in_hook.pf = PF_INET;
    bundle_in_hook.hooknum = NF_INET_PRE_ROUTING;
    bundle_in_hook.priority = NF_IP_PRI_FIRST;

    bundle_out_hook.hook = hook_out;
    bundle_out_hook.pf = PF_INET;
    bundle_out_hook.hooknum = NF_INET_POST_ROUTING;
    bundle_out_hook.priority = NF_IP_PRI_FIRST;

    

    for_each_net(n) {
        nf_register_net_hook(n, &bundle_in_hook);
        nf_register_net_hook(n, &bundle_out_hook);
    }

    LEAF = kmalloc(sizeof(struct node), GFP_KERNEL);
    LEAF->color = BLACK;
    LEAF->left = NULL;
    LEAF->right = NULL;    
    char s = '1';
    memcpy(&LEAF->key, &s, 1);

    return 0;
}

static void __exit hook_exit(void) {
    struct net *n;

    for_each_net(n) {
        nf_unregister_net_hook(n ,&bundle_in_hook);
        nf_unregister_net_hook(n ,&bundle_out_hook);
    }
    printk(KERN_INFO "Exit my module\n");
}



module_init(hook_init);
module_exit(hook_exit);