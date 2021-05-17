# __init__.py
from tornado.httpserver import HTTPServer
from tornado.options import define, options
from tornado.web import Application

define('port', default=61924, help='port to listen on')

def main():
    """Construct and serve the tornado application."""
    app = Application()
    http_server = HTTPServer(app)
    http_server.listen(options.port)