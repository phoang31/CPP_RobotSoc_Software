#Socket client example in python
 
import socket                   #for sockets
import sys                      #for exit
 
#create an INET, STREAMing socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)   #creating socket
except socket.error:
    print 'Failed to create socket'
    sys.exit()
     
print 'Socket Created'
 
host = '192.168.2.7';                               #host can be: IP address
port = 5002;

    
try:
    remote_ip = socket.gethostbyname( host )        #get host name
 
except socket.gaierror:
    #could not resolve
    print 'Hostname could not be resolved. Exiting'
    sys.exit()
 
#Connect to remote server
s.connect((remote_ip , port))                       #connect to the host server

while 1:     
    print 'Socket Connected to ' + host + ' on ip ' + remote_ip
 
    #Send some data to remote server

    i = 0
    while i < 4:                        #number of packet send right now is 4
        try :
            print("Enter a command: F,B,R,L, and a nuumber for duration: ")
            command = raw_input()
            s.sendall(command)
            print(i)
        except socket.error:
            print 'SendFailed'
            sys.exit()
        i = i + 1

    print 'Message send successfully'
     
    #Now receive data
   # reply = s.recv(4096)                #use s.recv() to receive data
     
   # print reply
    #s.close()                          
