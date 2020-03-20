import SimpleHTTPServer
import SocketServer
import socket
import threading
import sys

#Port to serve client web application, defaults to 8000.
HTTP_PORT = 8000 if len(sys.argv) < 2 else int(sys.argv[1])

#Listening port for incoming bulb connection, defaults to 1234.
BULB_PORT = 1234 if len(sys.argv) < 3 else int(sys.argv[2])

#Queue for colors produced by PUT requests and consumed by light bulb.
queue = []
lock = threading.Lock()

# TCP/HTTP Server connection for client application.
class MyRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def do_PUT(self):
    content_length = int(self.headers['Content-Length'])
    body = self.rfile.read(content_length)
    print(body)
    if(body[0] == '#'):
      h = body[1:]
      vals = tuple(int(h[i:i+2], 16) for i in (0, 2, 4))
      print vals
      lock.acquire()
      queue.append(vals)
      lock.release()
    return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

#Thread to serve client HTTP web app.
def ClientThread():
  print "serving at port", HTTP_PORT
  httpd.serve_forever()

#Thread to handle bulb connection.
class BulbThread(threading.Thread):
  def __init__(self, client_sock):
    threading.Thread.__init__(self)
    self.client_sock = client_sock

  def run(self):
    try:
      data = self.client_sock.recv(1024)

      #Send queued colors to light bulb, if present. 
      while 1:
        vals = None
        lock.acquire()
        if queue:
          vals = queue.pop(0)
        lock.release()
        if vals:
          for i, color in enumerate(('R', 'G', 'B')):
            data = color + str(int(vals[i] * 100 / 256)).zfill(3) + '\r\n'
            try:
              self.client_sock.send(data)
            except Exception:
              print 'Error sending colors to light bulb.'
              lock.acquire()
              queue.append(vals)
              lock.release()
              return
    except KeyboardInterrupt, SystemExit:
      return

if __name__ == '__main__':
  #Start HTTP web app server in new thread.
  Handler = MyRequestHandler
  httpd = SocketServer.TCPServer(("", HTTP_PORT), Handler)
  http_thd = threading.Thread(target=ClientThread)
  http_thd.daemon = True
  http_thd.start()

  #Create socket to listen for incoming light bulb connection.
  HOST = ''  # Symbolic name meaning all available interfaces.
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  sock.bind((HOST, BULB_PORT))
  sock.listen(5)

  while 1:
    try:
      #Connection received, spawn thread to handle bulb connection.
      client_sock, client_addr = sock.accept()
      bt = BulbThread(client_sock)
      bt.daemon = True
      bt.start()
    except KeyboardInterrupt, SystemExit:
      print 'Shutting down. Goodbye!'
      sock.close()
      httpd.shutdown()
      httpd.server_close()
      http_thd.join()
      sys.exit()
