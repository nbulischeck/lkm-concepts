#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("Read Directories from the Kernel");
MODULE_LICENSE("GPL");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

struct linux_dirent {
	unsigned long 	d_ino;
	unsigned long 	d_off;
	unsigned short 	d_namlen;
	unsigned long 	d_type;
	char 			d_name[];
};

struct readdir_data {
	struct dir_context 	ctx;
	char 				*dirent;
	size_t 				used;
	int 				full;
};

static int filldir_fn(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int type){
	struct readdir_data *buf;
	struct linux_dirent *d;
	unsigned int reclen;

	buf = container_of(ctx, struct readdir_data, ctx);
	d = (void *)(buf->dirent + buf->used);

	reclen = ALIGN(sizeof(struct linux_dirent) + namlen, sizeof(u64));
	if (buf->used + reclen > PAGE_SIZE) {
		buf->full = 1;
		return -EINVAL;
	}

	d->d_ino = ino;
	d->d_off = offset;
	d->d_namlen = namlen;
	d->d_type = type;
	memcpy(d->d_name, name, namlen);
	buf->used += reclen;

	return 0;
}

void list_directory(char *directory){
	int size, error;
	char *buffer;
	struct file *fp;
	struct linux_dirent *d;
	struct readdir_data buf = {
		.ctx.actor = filldir_fn,
		.dirent = (void *)__get_free_page(GFP_KERNEL),
	};

	fp = filp_open(directory, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp))
		return;

	while (1){
		unsigned int reclen;
		buf.used = 0;
		buf.full = 0;

		error = iterate_dir(fp, &buf.ctx);
		if ((!buf.full && (error < 0)) || !buf.used)
			break;

		size = buf.used;
		d = (struct linux_dirent *)buf.dirent;

		while (size > 0){
			buffer = kzalloc(d->d_namlen+1, GFP_KERNEL);
			memcpy(buffer, d->d_name, d->d_namlen);
			printk("%s\n", buffer);
			kfree(buffer);

			reclen = ALIGN(sizeof(*d) + d->d_namlen, sizeof(u64));
			d = (struct linux_dirent *)((char *)d + reclen);
			size -= reclen;			
		}

		if (size > 0)
			break;
	}

	free_page((unsigned long)(buf.dirent));

	return;
}

static int __init init_mod(void){
	list_directory("/proc");
	return 0;
}

static void __exit exit_mod(void){
	return;
}

module_init(init_mod);
module_exit(exit_mod);
