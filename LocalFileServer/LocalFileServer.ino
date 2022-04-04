//  Local webserver made by Mats Cornegoor
//  Made for Wemos Lolin D32 Pro V2

//  How to use:
//  1. Connect to FileServer access point
//  2. Open captive portal in webbrowser (force by browsing to 172.0.0.1)
//  3. Browse files, upload using buttons or delete by going to 172.0.0.1/delete/-filename-

//  Inspired by demo shared on http://www.iotsharing.com/2019/07/how-to-turn-esp-with-sdcard-or-spiffs-a-web-file-server.html


#include <WiFiClient.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <SPI.h>
#include <SD.h>

//  Server setup
const byte DNS_PORT = 53; 
IPAddress apIP(172, 0, 0, 1);
DNSServer dnsServer;
WebServer server(80);

//  SD
File root;
bool opened = false;

//  Web interface
String indexHeader = "<!DOCTYPE html><html><head><style>"

"body {background-color:black;}"
"p, a {font-size:15px; font-family:sans-serif; color:white; text-decoration:none;}"
"a::before {content:\" -- \";} "
"h1 {font-size:18px; font-family:sans-serif; color:white;}"
".button {font-size:26px; text-decoration: none; margin:10px; display:block; color:white; background-color:black; border:2px solid white; cursor:pointer; padding:10px 20px;}"
".files {margin-left: 15px;} "
"</style></head><body>"

"<div>"
"<button class='button' id='fileBtn' onclick='getFile()'>Select File</button>"
"<button onclick='uploadFile()' class='button'>Upload File</button>"
"</div>"

"<div style='height:0px; width: 0px; overflow:hidden;'>"
"<form method='POST' name='uploadForm' action='/update' enctype='multipart/form-data' id='upload_form'>"
    "<input id='upfile' name='update' type='file' value='upload' onchange='sub(this)' />"
"</form>"
"</div><div class=\"files\">";


String indexFooter = "</div>"
"<script>"
"function getFile() {"
  "document.getElementById('upfile').click();"
"}"

"function sub(obj) {"
  "var file = obj.value;"
  "file = file.replace(/ /g, '_');"
  "document.getElementById('fileBtn').innerHTML = file;"
  "event.preventDefault();"
"}"

"function uploadFile() {"
  "document.uploadForm.submit();"
"}"

"</script>";


void setup(void){
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("FileServer");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // use "*" to open webpage on all urls
  dnsServer.start(DNS_PORT, "*", apIP);
  
  if (!SD.begin(4)) {
    Serial.println("Card Mount failed!");
    return;
  }
  Serial.println("initialization done.");

  //  Handle root 
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  //  Delete file path after /delete/
  server.on(UriBraces("/delete/{}"), []() {
    String fileToDelete = "/" + server.pathArg(0);
    SD.remove(fileToDelete);
    handleRoot();
  });

  //  upload file to SD
  server.on("/update", HTTP_POST, [](){
    server.sendHeader("Connection", "close");
    handleRoot();
  },[](){
    
    HTTPUpload& upload = server.upload();
    if(opened == false){
      opened = true;
      String fileName = upload.filename;
      fileName.replace(" ","_");
      
      root = SD.open((String("/") + fileName).c_str(), FILE_WRITE);  
      if(!root){
        Serial.println("- failed to open file for writing");
        return;
      }
    } 
    if(upload.status == UPLOAD_FILE_WRITE){
      if(root.write(upload.buf, upload.currentSize) != upload.currentSize){
        Serial.println("- failed to write");
        return;
      }
    } else if(upload.status == UPLOAD_FILE_END){
      root.close();
      Serial.println("UPLOAD_FILE_END");
      opened = false;
    }

  });


  server.begin();
  Serial.println("Server started");
}

void loop(void){
  dnsServer.processNextRequest();
  server.handleClient();
  delay(1);
}

void handleRoot() {
  root = SD.open("/");
  server.send(200, "text/html", index());
}

String index(){
  String res = printDirectory(root, 0);
  return indexHeader + res + indexFooter;
}

void handleNotFound(){
  if(loadFromSDCARD(server.uri())) return;
  handleRoot();
}

//  Load file from SD
bool loadFromSDCARD(String path){
  path.toLowerCase();
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".txt")) dataType = "text/plain";
  else if(path.endsWith(".zip")) dataType = "application/zip";  
  Serial.println(dataType);
  File dataFile = SD.open(path.c_str());

  if (!dataFile)
    return false;

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

//  Print directories as strings
String printDirectory(File dir, int numTabs) {
  String response = "";
  dir.rewindDirectory();
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // No more files
       break;
     }

     // Recurse for directories, otherwise print the file size
     if (entry.isDirectory()) {
       printDirectory(entry, numTabs+1);
     } else {
       response += String("<a href='") + String(entry.name()) + String("'>") + String(entry.name()) + String("</a>") + String("</br></br>");
     }
     entry.close();
   }
   return String("List files:</br>") + response + String("</br></br> Upload file:");
}
