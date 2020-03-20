import SimpleHTTPServer
import SocketServer
import socket
import threading

PORT = 8000

# TCP/HTTP Server connection for client application.
class MyRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_PUT(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        print(body)
        if(body[0] == '#'):
            h = body[1:]
            print tuple(int(h[i:i+2], 16) for i in (0, 2, 4)) 
        return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

Handler = MyRequestHandler

httpd = SocketServer.TCPServer(("", PORT), Handler)

def ClientThread():
    print "serving at port", PORT
    httpd.serve_forever()

t1 = threading.Thread(target=ClientThread)
t1.start()

#Connection to light
HOST = ''                # Symbolic name meaning all available interfaces
PORT = 1234              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
while True:
    data = conn.recv(1024)
    if not data: break
    print(data)
