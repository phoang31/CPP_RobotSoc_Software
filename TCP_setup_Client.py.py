#Socket client example in python
 
import socket                                                                            #need this for tcpip set up
import sys                                                                               #system specific parameters and functions. usually use to exit the program

def tcpClient(IP, Portnum):
    #Existing Message
    ExitMessage = 'Quitting on User Request\n'

    #Validating IP address
    print("Testing IP address")
    valid = 0
    while(valid == 0):
        try:
            socket.inet_aton(IP)
            valid = 1;
            print("The IP address is Valid")
        except socket.error:
            print("Invalid IP address. Please enter a valid IP address:\n")
            IP = raw_input()
            valid = 0;      
    #Validating port number
    while(Portnum < 0 | Portnum > 65536):
        print("Invalid Port Number. Please enter a valid port number:\n")
        Portnum = raw_input()
        
    #create an INET, STREAMing socket
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)                               
    except socket.error:
        print 'Failed to create socket'
        sys.exit() #exit from python
     
    print 'Socket Created'
 
    host = IP;                                                                  #host can be: IP address
    port = Portnum;             

    
    try:
        remote_ip = socket.gethostbyname( host )                                            #get host name
 
    except socket.gaierror:
        #could not resolve
        print 'Hostname could not be resolved. Exiting'
        sys.exit()
 
    #Connect to remote server
    s.connect((remote_ip , port))                                                           #connect to the host server

    while 1:     
        print 'Socket Connected to ' + host + ' on ip ' + remote_ip
 
        #Send some data to remote server

        i = 0
        while i < 4:   #this while loop is for testing purposes                                                                     #number of packets send right now is 4
            try :
                print("Enter a command: d,b,r,l, and a number for duration: ")
                command = raw_input()
                if(command == 'q'):
                    s.sendall(ExitMessage)
                    print(ExitMessage)
                    break
                elif(command == 'Q'):
                    s.sendall(ExitMessage)
                    print(ExitMessage)
                    break
                else:
                    s.sendall(command)
                print(i)
            except socket.error:
                print 'SendFailed'
                sys.exit()
            i = i + 1

        print 'Message sent successfully'
     
        #Now receive data
       # reply = s.recv(4096)                #use s.recv() to receive data
     
       # print reply
    s.close()                          

print("Welcome to TCP Client\n")
print("Please provide the IP address of the Host: ")
ip = raw_input()
print("Please enter a port number: ")
portn = raw_input()
tcpClient(ip, int(portn))
