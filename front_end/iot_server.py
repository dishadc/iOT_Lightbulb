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
  try:
    httpd.serve_forever()
  except KeyboardInterrupt, SystemExit:
    httpd.shutdown()
    httpd.server_close()


if __name__ == '__main__':
  #Start HTTP Server.
  Handler = MyRequestHandler
  httpd = SocketServer.TCPServer(("", HTTP_PORT), Handler)

  t1 = threading.Thread(target=ClientThread)
  t1.start()

  #Connection to light
  print 'opening socket'
  HOST = ''                # Symbolic name meaning all available interfaces
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind((HOST, BULB_PORT))
  s.listen(5)
  conn, addr = s.accept()
  data = conn.recv(1024)
  while True:
    vals = None
    lock.acquire()
    if queue:
      vals = queue.pop(0)
    lock.release()
    if vals:
      print "consumed", vals
      for i, color in enumerate(('R', 'G', 'B')):
        data = color + str(int(vals[i] * 100 / 256)).zfill(3) + '\r\n'
        print data
        try:
          conn.send(data)
        except Exception:
          print 'exception raised'
