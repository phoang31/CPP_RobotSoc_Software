import socket
import sys
from _thread import *

HOST = ''   #symbolic name implies all available interfaces can be used
PORT = 8888 #arbitrary unused port

##create a socket##
try:
    #create socket with IPv4 and TCP protocol
    #socket.socket() creates socket and returns socket descriptor
    s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error as msg:
    print ('Failed to create socket. Error code: ' + str(msg[0])
	    + 'Error message: ' + msg[1])
    s.close()
    sys.exit(1)
print ('Socket created')

##bind socket to port##
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print('Bind failed. Error Code: ' + str(msg[0])
        + ' Message ' + str(msg[1]))
    s.close()
    sys.exit(1)
print('Socket binded')

##listen for incoming connections##
s.listen(10)#puts socket in listening mode with queue of 10 connections
            #11th+ connection will be rejected, as queue is full
print('Listening for connections')


##Function for handling connections. This will be used to create threads
def clientthread(conn,addr):
    #Sending message to connected client
    welcome='Welcome to the server. Type something and hit enter\r\n'
    conn.send(welcome.encode('utf_8'))
     
    #infinite loop so that function does not terminate and thread does not end.
    while True:
         
        #Receiving from client
        data = conn.recv(1024)
        reply = data
        if not data: 
            break
     
        conn.sendall('Got it!...'.encode('utf_8')+reply+'\r\n'.encode('utf_8'))
     
    #came out of loop
    print(addr[0] + ':' + str(addr[1])+" disconnected")
    conn.close()
 
#now keep talking with the client
while 1:
    #wait to accept a connection - blocking call
    conn, addr = s.accept()
    print ('Connected with ' + addr[0] + ':' + str(addr[1]))
     
    #start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    start_new_thread(clientthread ,(conn,addr))
s.close()
