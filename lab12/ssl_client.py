#!/usr/bin/python3
import socket, ssl, sys, pprint

# Primim ca argument hostname-ul serverului, de exemplu google.com
hostname = sys.argv[1]

port = 443

# Create TCP connection
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((hostname, port))

input("After making TCP connection. Press any key to continue ...")

# You may need to change this depending on your Linux distro
cadir = '/etc/ssl/certs'

# Set up the TLS context
context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
# context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2) # For Ubuntu 16.04
context.load_verify_locations(capath=cadir)
context.verify_mode = ssl.CERT_REQUIRED
context.check_hostname = True

# Add the TLS
ssock = context.wrap_socket(sock, server_hostname=hostname,
                            do_handshake_on_connect=False)
ssock.do_handshake()  # Start the handshake
pprint.pprint(ssock.getpeercert())

input("After handshake. Press any key to continue ...")

# Close the TLS Connection
ssock.shutdown(socket.SHUT_RDWR)
ssock.close()
