//  Simple local webserver
//  Made for Wemos Lolin D32 Pro V2


#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const byte DNS_PORT = 53; //  DNS port, used for domain name resolution
IPAddress apIP(172, 0, 0, 1); //  Web server IP adress, this can be changed
DNSServer dnsServer; //  Assign DNS server libary to global variable
WebServer server(80); // Assign webserver libary to global variable with default network port for webpages


//  This String variable is for storing text
String html = "<h1>Hello World</h1><p>This is a captive portal example.</p>";


//  A function is an action that is executed ones or multiple times
//  This function is used to load the html variable
void loadPage(){
  server.send(200, "text/html", html);
}

//  The setup function only executes at startup
void setup() { 
  //  Start captive portal
  WiFi.mode(WIFI_AP);
  //  Assign name to captive portal
  WiFi.softAP("Webpage");
  //  Captive portal config
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  //  Use "*" to open webpage on all urls
  dnsServer.start(DNS_PORT, "*", apIP);

  //  Execute loadPage function when client tries to visit server
  server.onNotFound(loadPage);
  server.begin();
}

//  This loop function executes every 2 ms
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
