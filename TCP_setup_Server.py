import socket
import sys
from thread import *
 
HOST = '192.168.2.15'   # Symbolic name meaning all available interfaces
PORT = 4001 # Arbitrary non-privileged port
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print ('Socket created')
 
#Bind socket to local host and port
try:
    s.bind((HOST, PORT))
except socket.error , msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
print 'Socket bind complete'
 
#Start listening on socket
s.listen(10)
print 'Socket now listening'

#Function for handling connections. This will be used to create threads
def clientthread(conn):
    #Sending message to connected client
    #conn.send('Welcome to the server.\n') #send only takes string
    pc = 'S' 
    #infinite loop so that function do not terminate and thread do not end.
    while True:
         
        #Receiving from client
        data = conn.recv(1)          #PROBLEM: ONLY 1 CHARACTER IS READ
        
        if (data == 'F' or data == 'L' or data == 'R' or data== 'S'):
            if data != pc:
                print(data)
        pc = data
        if not data: 
            break
     
     ##   conn.sendall('Got it!...'.encode('utf_8')+reply+'\r\n'.encode('utf_8'))
     #   conn.sendall(data)
    #came out of loop
    conn.close()
 
#now keep talking with the client
while 1:
    #wait to accept a connection - blocking call
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])
     
    #start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    start_new_thread(clientthread ,(conn,))
 
s.close()
