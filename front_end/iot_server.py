import SimpleHTTPServer
import SocketServer
import socket
import threading
import sys

HTTP_PORT = 8000 if len(sys.argv) < 2 else int(sys.argv[1])
BULB_PORT = 1234 if len(sys.argv) < 3 else int(sys.argv[2])

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


def ClientThread():
  print "serving at port", HTTP_PORT
  httpd.serve_forever()

class BulbThread(threading.Thread):
  def __init__(self, sock, client_sock):
    threading.Thread.__init__(self)
    self.sock = sock
    self.client_sock = client_sock

  def run(self):
    try:
      data = self.client_sock.recv(1024)
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
              print 'exception raised'
              lock.acquire()
              queue.append(vals)
              lock.release()
              return
    except KeyboardInterrupt, SystemExit:
      return

if __name__ == '__main__':
  #Start HTTP Server.
  Handler = MyRequestHandler
  httpd = SocketServer.TCPServer(("", HTTP_PORT), Handler)

  t1 = threading.Thread(target=ClientThread)
  t1.daemon = True
  t1.start()

  #Connection to light
  HOST = ''                # Symbolic name meaning all available interfaces
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind((HOST, BULB_PORT))
  s.listen(5)

  while 1:
    try:
      conn, addr = s.accept()
      bt = BulbThread(s, conn)
      bt.daemon = True
      bt.start()
      #data = conn.recv(1024)
    
    except KeyboardInterrupt, SystemExit:
      s.close()
      httpd.shutdown()
      httpd.server_close()
      t1.join()
      sys.exit()
  '''
  while 1:
    try:
      vals = None
      lock.acquire()
      if queue:
        vals = queue.pop(0)
      lock.release()
      if vals:
        for i, color in enumerate(('R', 'G', 'B')):
          data = color + str(int(vals[i] * 100 / 256)).zfill(3) + '\r\n'
          try:
            conn.send(data)
          except Exception:
            print 'exception raised'
    except KeyboardInterrupt, SystemExit:
      s.close()
      httpd.shutdown()
      httpd.server_close()
      t1.join()
      sys.exit()
  '''
