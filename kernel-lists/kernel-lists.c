#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("Kernel Lists");
MODULE_LICENSE("GPL");

struct node {
	int data;
	struct list_head list;
};

static LIST_HEAD(old_list);
static LIST_HEAD(new_list);

void append_node(struct list_head *list, int data){
	struct node *entry;

	entry = kmalloc(sizeof *entry, GFP_KERNEL);
	if (!entry)
		return;

	entry->data = data;

	INIT_LIST_HEAD(&entry->list);
	list_add_tail(&entry->list, list);
}

void find_removed_nodes(void){
	struct node *ptr_old, *tmp_old;
	struct node *ptr_new, *tmp_new;

	list_for_each_entry_safe(ptr_new, tmp_new, &new_list, list){
		list_for_each_entry_safe(ptr_old, tmp_old, &old_list, list){
			if (ptr_new->data == ptr_old->data){
				break;
			} else if (list_is_last(&ptr_old->list, &old_list)){
				printk("%d has been added to the list\n", ptr_new->data);
				list_move_tail(&ptr_new->list, &old_list);
				break;
			}
		}
	}
}

void find_added_nodes(void){
	struct node *ptr_old, *tmp_old;
	struct node *ptr_new, *tmp_new;

	list_for_each_entry_safe(ptr_old, tmp_old, &old_list, list){
		list_for_each_entry_safe(ptr_new, tmp_new, &new_list, list){
			if (ptr_old->data == ptr_new->data){
				break;
			} else if (list_is_last(&ptr_new->list, &new_list)){
				printk("%d has been removed from list\n", ptr_old->data);
				list_del(&ptr_old->list);
				kfree(ptr_old);
				break;
			}
		}
	}
}

void build_lists(void){
	append_node(&old_list, 100);
	append_node(&old_list, 200);
	append_node(&old_list, 300);
	append_node(&old_list, 400);

	append_node(&new_list, 100);
	append_node(&new_list, 200);
	append_node(&new_list, 300);
	append_node(&new_list, 500);
}

static int __init init_mod(void){
	struct node *ptr;

	build_lists();
	find_added_nodes();
	find_removed_nodes();

	printk("Updated Old List:\n");
	list_for_each_entry(ptr, &old_list, list){
		printk(KERN_CONT "%d->", ptr->data);
	}
	printk(KERN_CONT "NULL\n");

	return 0;
}

static void __exit exit_mod(void){
	struct node *ptr, *tmp;

	list_for_each_entry_safe(ptr, tmp, &old_list, list){
		list_del(&ptr->list);
		kfree(ptr);
	}

	list_for_each_entry_safe(ptr, tmp, &new_list, list){
		list_del(&ptr->list);
		kfree(ptr);
	}
}

module_init(init_mod);
module_exit(exit_mod);
