#!/usr/bin/env python

"""
An echo server that uses select to handle multiple clients at a time.
Entering any line of input at the terminal will exit the server.
"""

import select
import socket
import sys
import signal

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def main(argv):
    
    signal.signal(signal.SIGHUP, SigTest)
    signal.signal(signal.SIGINT, SigTest)
    signal.signal(signal.SIGQUIT, SigTest)
    
    size = 1024
    maxrange = 4294967296
    minrange = 1
    perfect_nums = []
    
    initServer(sys.argv[1], int(sys.argv[2]), 70)
    
    clients = []
    inputs = [server,sys.stdin]
    running = 1
    while running:
        inputready,outputready,exceptready = select.select(inputs,[],[])

        for s in inputready:

            if s == server:
                # handle the server socket
                client, address = server.accept()
                print >>sys.stderr, 'new connection from', address
                clients.append([address[1]])
                client.setblocking(0)
                inputs.append(client)

            elif s == sys.stdin:
                # handle standard input
                junk = sys.stdin.readline()
                running = 0

            else:
                # handle all other sockets
                data = str(s.recv(size))
                if data:
                    if data[0] == 'i':
                        #it is IOPS
                        data = data.replace("i", "")
                        print >>sys.stderr, 'IOPS from ', address[1],': ', int(data)
                        #add IOPS to client
                        for x in clients:
                            if x[0] == address[1]:
                                x.append(int(data))
                        #Get range to send to client
                        uplimit = getRangeFromIOPS(minrange, maxrange, int(data))
                        toclient = str(minrange) + ", " + str(uplimit)
                        print "Sending range: ", toclient
                        s.send(toclient)
                        minrange = uplimit
                    elif data[0] == 'p':
                        #it is a perfect number
                        data = data.replace("p", "")
                        perfect_nums.append(int(data))
                    else:
                        #we need a range
                        pass
                else:
                    s.close()
                    print >>sys.stderr, 'Removing client ', address[1], 'from list'
                    clients = [(cid, iops) for cid, iops in clients if cid != address[1]]
                    inputs.remove(s)
    server.close()
    
def SigTest(SIG, FRM):
#    print "Caught signal: ", SIG
    server.close()
    sys.exit()
    
def initServer(ipaddr, port, num_clients):
    
    server.setblocking(0)
    
    server_address = (ipaddr, port)
    
    print >>sys.stderr, 'starting up on %s port %s' % server_address
    server.bind(server_address)
    
    server.listen(num_clients)
  
def getRangeFromIOPS(minimum, maximum, iops):
    
    i = 0
    value = minimum
    
    while i < 15*iops and value < maximum:
        value += 1
        i += value
    
    #check to make sure that min value increases as min get higher
    if value == minimum:
        value += 1
        
    return value


    
    
if __name__ == "__main__":
    main(sys.argv[1:])