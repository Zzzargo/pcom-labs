#!/usr/bin/python3
import socket
import ssl

html = """HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><body><h1>Hello, world!</h1></body></html>"""

SERVER_CERT = './server.crt'
SERVER_PRIVATE = './server.key'

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
# context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2) # Ubuntu 16.04
context.load_cert_chain(SERVER_CERT, SERVER_PRIVATE)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
sock.bind(('0.0.0.0', 4433))
sock.listen(5)

while True:
    newsock, fromaddr = sock.accept()
    ssock = context.wrap_socket(newsock, server_side=True)
    data = ssock.recv(1024)  # Read data over TLS
    ssock.sendall(html.encode('utf-8'))  # Send data over TLS
    ssock.shutdown(socket.SHUT_RDWR)  # Close the TLS connection
    ssock.close()
