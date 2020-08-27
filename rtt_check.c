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

struct node *root = NULL;

MODULE_AUTHOR("Ilia Krekerrr");
MODULE_DESCRIPTION("Check delay packets");
MODULE_LICENSE("GPL");

/*
struct packetInfo {
    struct rb_node node;
    char key[48];
    struct timespec spec;
};

/*
struct mytype {
  	struct rb_node node;
  	char *keystring;
  };

uint16_t k = 1;

struct rb_root mytree = RB_ROOT;

struct mytype *my_search(struct rb_root *root, char *string)
  {
  	struct rb_node *node = root->rb_node;
  	while (node) {
  		struct mytype *data = container_of(node, struct mytype, node);
		int result;
		result = strcmp(string, data->keystring);
		if (result < 0)
  			node = node->rb_left;
		else if (result > 0)
  			node = node->rb_right;
		else
  			return data;
	}
	return NULL;
  }

int *my_check(struct rb_root *root, char *string)
  {
  	struct rb_node *node = root->rb_node;
  	while (node) {
  		struct mytype *data = container_of(node, struct mytype, node);
		int result;
		result = strcmp(string, data->keystring);
		if (result < 0)
  			node = node->rb_left;
		else if (result > 0)
  			node = node->rb_right;
		else
  			return 1;
	}
	return 0;
  }

int my_insert(struct rb_root *root, struct mytype *data)
  {
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	while (*new) {
  		struct mytype *this = container_of(*new, struct mytype, node);
  		int result = strcmp(data->keystring, this->keystring);
		parent = *new;
  		if (result < 0)
  			new = &((*new)->rb_left);
  		else if (result > 0)
  			new = &((*new)->rb_right);
  		else
  			return 0;
  	}

  	printk("d");

    //rb_link_node(&data->node, parent, new);
  	//rb_insert_color(&data->node, root);
	return 1;
  }

/*
struct packetInfo *get_info(struct rb_root *root, uint32_t key) {
    struct rb_node *node = root->rb_node;

    while (node) {
        struct packetInfo *info = rb_entry(node, struct packetInfo, node);

        if (key > info->key) node = node->rb_left;
        else if (key < info->key) node = node->rb_right;
        else return info;
    } 
    return NULL;
}

int *check_info(struct rb_root *root, uint32_t key) {
    struct rb_node *node = root->rb_node;

    while (node) {
        struct packetInfo *info = rb_entry(node, struct packetInfo, node);

        if (key > info->key) node = node->rb_left;
        else if (key < info->key) node = node->rb_right;
        else return 1;
    } 
    return 0;
}

void insert_info(struct rb_root *root, struct packetInfo *new) {
    struct rb_node **link = &root->rb_node, *parent = NULL;
    
    while (*link) {
        parent = *link;
        struct packetInfo *info = rb_entry(parent, struct packetInfo, node);

        
        if (new->key > info->key) link = &parent->rb_left;
        else link = &parent->rb_right;
    }

    root->rb_node->rb_right;

    if (k == 1) {
        //rb_link_node(&new->node, parent, link);
        rb_insert_color(&new->node, root); 
        k++;
    }
}

struct rb_root root_tree = RB_ROOT;
*/
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
        uint16_t saddr, daddr, sport, dport;
        uint16_t seq, ack_seq;
        u_char sstr[16], ssport[8];
        u_char dstr[48], sdport[8];
        u_char sseq[8], sack_seq[16]; 

        saddr = (uint16_t)ntohs(ip->saddr);
        daddr = (uint16_t)ntohs(ip->daddr);
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
        strcat(dstr, sseq);
        //printk("in %s", dstr);
        
        struct node *old = search(root, dstr);
        //if (!old) printk("what?");
        if (old) {
            printk("&&");
            
            ktime_t current_time, old_time, result_time;
                char b1, b2, b3, b4;
                char sbuf[52], dbuf[52];
                
                current_time = spec.tv_sec * 1000000000 + spec.tv_nsec;
                old_time = old->spec.tv_sec * 1000000000 + old->spec.tv_nsec;
                result_time = current_time - old_time;
                
                b1 = saddr & 0xff;
                b2 = (saddr >> 8) & 0xff;
                b3 = (saddr >> 16) & 0xff;
                b4 = (saddr >> 24) & 0xff;
                sprintf(dbuf, "%i.%i.%i.%i:%u", b1, b2, b3, b4, sport);

                b1 = daddr & 0xff;
                b2 = (daddr >> 8) & 0xff;
                b3 = (daddr >> 16) & 0xff;
                b4 = (daddr >> 24) & 0xff;
                sprintf(sbuf, "%i.%i.%i.%i:%u", b1, b2, b3, b4, dport);
                
                printk("%s -> %s, rtt = %u ns", result_time);
                //delete_one_child(old);
        }

        /*
        if (!search(root, dstr)) {
            struct node *old = get_info(&root, dstr);
            if (unlikely(old == NULL)) printk("Error: old is NULL");
            else {
                
                ktime_t current_time, old_time, result_time;
                char b1, b2, b3, b4;
                char sbuf[52], dbuf[52];

                current_time = spec.tv_sec * 1000000000 + spec.tv_nsec;
                old_time = old->spec.tv_sec * 1000000000 + old->spec.tv_nsec;
                result_time = current_time - 
                b1 = saddr & 0xff;
                b2 = (saddr >> 8) & 0xff;old_time;

                b3 = (saddr >> 16) & 0xff;
                b4 = (saddr >> 24) & 0xff;
                sprintf(dbuf, "%i.%i.%i.%i:%u", b1, b2, b3, b4, sport);

                b1 = daddr & 0xff;
                b2 = (daddr >> 8) & 0xff;
                b3 = (daddr >> 16) & 0xff;
                b4 = (daddr >> 24) & 0xff;
                sprintf(sbuf, "%i.%i.%i.%i:%u", b1, b2, b3, b4, dport);

                printk("%s -> %s, rtt = %u ns", result_time);
                rb_erase(&old->node, &root);
            }
            
            
        } 
        */
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
        uint16_t saddr, daddr, sport, dport;          
        uint16_t seq, ack_seq;
        u_char sstr[48], ssport[8];
        u_char dstr[16], sdport[8];
        u_char sseq[16], sack_seq[8]; 

        saddr = (uint16_t)ntohs(ip->saddr);
        daddr = (uint16_t)ntohs(ip->daddr);
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

        
        if (!search(root, sstr)) {
            struct node *new = kmalloc(sizeof(struct node), GFP_KERNEL);
            memcpy(new->key, &sstr, 48);
            //printk("out %s", new->key);
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