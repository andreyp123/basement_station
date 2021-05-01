#pragma ones

#include <WiFi.h>

/*
 * Implements a web server processing http client requests and retrieving a simple web-page with various sensors information.
 */
class WebServer
{
private:
  const static int WEB_SERVER_PORT = 80; // web-server port (http default)
  
  Context* _context;
  WiFiServer _server;
  bool _init;

  String getHtml();
  String getUrl();

public:
  WebServer(Context* context):
    _context(context),
    _server(WEB_SERVER_PORT),
    _init(false)
  {
  }
  void init();
  bool process();
};
