import SimpleHTTPServer
import SocketServer
import socket
import threading
import sys

PORT = int(sys.argv[1])

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
  try:
    httpd.serve_forever()
  except KeyboardInterrupt, SystemExit:
    pass
  finally:
    httpd.shutdown()
    httpd.server_close()

t1 = threading.Thread(target=ClientThread)
t1.start()

class LightThread(threading.Thread):
  def __init__(self, sock, client_sock):
    threading.Thread.__init__(self)
    self.sock = sock
    self.client_sock = client_sock

  def run(self):
    print "client thread started"
    try:
      data = client_sock.recv(1024)
      print(data)
    except Exception:
      return

  
#Connection to light
print 'opening socket'
HOST = ''                # Symbolic name meaning all available interfaces
port = int(sys.argv[2])  # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, port))
s.listen(5)
while True:
    try:
      conn, addr = s.accept()
      lt = LightThread(s, conn)
      lt.daemon = True
      lt.start()
    except KeyboardInterrupt, SystemExit:
      s.close()
      sys.exit()
      print "Shutting down iOT Server"
