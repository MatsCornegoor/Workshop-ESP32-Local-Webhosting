//  Local chatroom made by Mats Cornegoor
//  Made for Wemos Lolin D32 Pro V2

//  How to use:
//  1. Connect to Chatroom access point
//  2. Open captive portal in webbrowser (force by browsing to 172.0.0.1)
//  3. Start a conversation


#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(172, 0, 0, 1);
DNSServer dnsServer;
WebServer server(80);

String Posts="";

String input(String argName) {
  String a = server.arg(argName);
  a.replace("<","&lt;");a.replace(">","&gt;");
  a.substring(0,200); return a; 
}


String header() {
  String CSS = "body { background-color: black; color: white; font-family:sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
    "input { font-size:26px; width: 100%; text-decoration: none; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
    "nav { color: #fff; display: block; border-bottom: 2px solid white; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "li { margin-left: 40px; }"
    ".posts { margin-bottom: 200px;}"
    ".button { margin-bottom: 20px; font-size:26px; text-decoration: none; display:block; color:white; background-color:black; border:2px solid white; cursor:pointer; padding:10px 20px;}"
    ".sender { background-color:black; padding-left: 20px; padding-right: 20px; position: fixed; bottom: 0px; width: calc(100% - 40px); }";
    ".posts { margin-bottom: 200px;}";
    
  String h = "<!DOCTYPE html>"
    "<head><title>Chatroom</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>Chatroom</b></nav><div><h1></div><div class=\"posts\">";
  return h; 
}

String index() {
  return header() + Posts + "<div></ol></div><div><form action=/post method=post>" +
    "<div class=\"sender\"><input type=text name=message autocomplete=off></input>" +
    "<input class=\"button\" type=submit value=\"Send\"></form></div></div><script>window.scrollTo(0, document.body.scrollHeight);</script></body></html>";
}


String posted() {
  String message = input("message");
  Posts= Posts +"<li><b>" + message + "</b></br></li>";

  return header() + Posts + "<div></ol></div><div><form action=/post method=post>" +
    "<div class=\"sender\"><input type=text name=message autocomplete=off></input>" +
    "<input class=\"button\" type=submit value=\"Send\"></form></div></div><script>window.scrollTo(0, document.body.scrollHeight);</script></body></html>";
}

void handleRoot() {
  server.send(200, "text/html", index());
}

void setup() { 
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Chatroom");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // use "*" to open webpage on all urls
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.onNotFound(handleRoot);
  server.on("/post",[]() { 
    server.send(HTTP_POST, "text/html", posted()); 
  });
  
  server.begin();
}


void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
