import socket
import sys  			                #for error handling and exiting

##create a socket##
try:
	#create socket with IPv4 and TCP protocol
	#socket.socket() creates socket and returns socket descriptor
	s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error as msg:
	print ('Failed to create socket. Error code: ' + str(msg[0])
		+ 'Error message: ' + msg[1])
	sys.exit(1)
print ('Socket created')

##get host ip from url##

host = 'www.google.com'
port = 80

try:
        remote_ip=socket.gethostbyname(host)
except socket.gaierror:                         #for address related errors
        #could not resolve hostname
        print ('Hostname could not be resolved. Exiting')
        s.close()
        sys.exit(2)
print ('IP address of ' + host + ' is ' + remote_ip)

##connect to remote server##
s.connect((remote_ip,port))#nested parenthesis are required.
print ('Socket connected to ' + host + ' on ip ' + remote_ip)


##Send http request to remote server
message = "GET / HTTP/1.1\r\n\r\n"
try :
    #Send the whole string
    s.sendall(message.encode('utf_8'))#sendall takes bytes as argument,not string
except socket.error:
    #Send failed
    print('Send failed')
    s.close()
    sys.exit(3)
print ('Request sent successfully\n')

##receive data response
reply = s.recv(4096)
print ('Response:')
print (reply)

##close the socket upon completion
s.close()
