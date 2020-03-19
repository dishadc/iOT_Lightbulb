import SimpleHTTPServer
import SocketServer

PORT = 8000

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

print "serving at port", PORT
httpd.serve_forever()
