#include <linux/kernel.h>
#include <linux/module.h>

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("Base LKM");
MODULE_LICENSE("GPL");

static int __init init_mod(void){
	return 0;
}

static void __exit exit_mod(void){
	return;
}

module_init(init_mod);
module_exit(exit_mod);
