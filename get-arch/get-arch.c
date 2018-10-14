#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("LKM to Retrieve CPU Architecture");
MODULE_LICENSE("GPL");

/* x86/include/asm/cpufeatures.h */
#define X86_64_LONG_MODE (1*32+29)

static void get_cpu_arch(void){
	struct cpuinfo_x86 *c = &cpu_data(0);
	int mode = cpu_has(c, X86_64_LONG_MODE) ? 64 : 32;
	printk(KERN_INFO "CPU is: %d-bit\n", mode);
}

static int __init init_mod(void){
	get_cpu_arch();
	return 0;
}

static void __exit exit_mod(void){
	return;
}

module_init(init_mod);
module_exit(exit_mod);
