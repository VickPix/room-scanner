#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h> 
#include "FS.h"

#define trigPin D8
#define echoPin D9

const char* ssid = "NON-SI-ENTRA";
const char* password = "c1m1n0c454";
long duration;
int distance;
bool autoConnect;
WiFiServer server2(80);
ESP8266WebServer server(80);
WiFiClient client;
String HTTP_req;

void setup() {

    pinMode(trigPin, OUTPUT); //t
    pinMode(echoPin, INPUT); //e
    digitalWrite(trigPin,LOW);
    
    Serial.begin(115200);
    SPIFFS.begin();

    setupWiFi();    
    
    // The root handler
    server.on("/", handleRoot);
    // Handlers for various user-triggered events
    server.on("/getRandom", getRandom);
    server.on("/status", sendStatus);
    
    // Start the server
    server.begin();
    Serial.println("Server started");
    
    // Print the IP address
    /*Serial.print("Use this URL : ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");*/
}

void loop() {
  /*
    WiFiClient client = server.available();
    if (client) { 
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) { 
                char c = client.read();
                HTTP_req += c;
                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                    if (HTTP_req.indexOf("ajax_switch") > -1) {
                        getTemperature(client);
                    } else {
                      /*
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Test</title>");
                        client.println("<script>");
                        client.println("function getTemperature() {");
                        client.println("nocache = \"&nocache=\"+ Math.random() * 1000000;");
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"switch_txt\").innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println("request.open(\"GET\", \"ajax_switch\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('getTemperature()', 1000);");
                        client.println("}");
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"getTemperature()\">");
                        client.println("<h1>Test ajax request</h1>");
                        client.println("<font color=\"#6a5acd\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n</div>\n<div style=\"clear:both;\"></div><p>");                                               
                        client.println("<p id=\"switch_txt\">Distanze</p>");
                        client.println("<p>RealTime Distance</p>");
                        //client.println("<p><a href=\"http://www.esp8266.com\">ESP8266 Support Forum</a></p>");
                        client.println("</body>");
                        client.println("</html>");
                    }
                    //Serial.print(HTTP_req);
                    HTTP_req = "";
                    break;
                }
            }
        }
        delay(1);
        client.stop();
    }
    */
    server.handleClient();
}

void setupWiFi(){
    /*WiFi.mode(WIFI_STA);
    WiFi.softAP ("Scanner", "123456789");
    //IPAddress IP = WiFi.softAPIP (); 
    //Serial.println(IP.toString());
  
    WiFi.begin(ssid, password); 

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");*/

    int ret;
    delay(500);
    ESP.eraseConfig();
    delay(500);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_AP_STA);
    Serial.println("\r\n========== SDK Saved parameters Start"); 
    WiFi.printDiag(Serial);
    Serial.println("========== SDK Saved parameters End"); 
    Serial.println("Connecting...");
  
    // Get current autoconnect state from SDK
    autoConnect = WiFi.getAutoConnect();
    WiFi.begin("ScannerTest", "123456789");
  
    uint8_t timeout = 10; // 10 * 500 ms = 5 sec time out
    while ( ((ret = WiFi.status()) != WL_CONNECTED) && timeout-- ) {
      Serial.print(".");
      delay(500);
    }
  
    // connected ? disable AP, client mode only
    if ((ret = WiFi.status()) == WL_CONNECTED) {
      Serial.println("connected!");
      Serial.print("STA IP address "); 
      Serial.println(WiFi.localIP());
    // not connected ? start AP
    } else {
      //WiFi.mode(WIFI_AP);
      Serial.println("Configuring access point...");
      WiFi.softAP("ESP-TESTAP","123456789");
      WiFi.softAPIP();
      Serial.print("AP IP address "); 
      Serial.println(WiFi.softAPIP());
    }
  
    Serial.println("Starting loop");
}

void getTemperature(WiFiClient cl) {
    delay(100);
    distance=calculateDistance();
    Serial.print(distance);
    Serial.print("\n");
    cl.print("Distance: ");
    cl.print(distance);
    cl.print(" cm");    
}

int calculateDistance()
{
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin,HIGH);
  distance=duration*0.034/2;
  if(distance>400) return 400;
  return distance;
}  

void getRandom(){
  int passi = 50;
  int distances[passi];
  Serial.println("Start");
  delay(1000);
  int i;
  
  for(i=0;i<passi;i++){
    distances[i]=calculateDistance();
    delay(100);
  }
  Serial.println("End");
  
  for(i=1;i<passi-1;i++){
    if(abs(distances[i-1]-distances[i+1])<30){
      distances[i]=(distances[i-1]+distances[i+1])/2;
    }
  }
  
  String s="[";
  for(i=0;i<passi-1;i++){
    s+=String(distances[i]);
    s+=",";
  }
  s+=String(distances[passi-1]); 
  s+="]";
  
  server.send(200, "application/json", s);
}

void printJson(){
  //First build the  response header
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Access-Control-Allow-Origin: *\r\n";
  s += "Content-Type: text/plain\r\n\r\n";
  s += "[1,2,3,4]";
  Serial.print(s);
  server.send(200, "application/json", "[100,222,300,400]");
  //return s;
}

void handleRoot() {
    // Just serve the index page from SPIFFS when asked for
    File index = SPIFFS.open("/index.html", "r");
    server.streamFile(index, "text/html");
    index.close();
}

// A function which sends the LED status back to the client
void sendStatus() {
//    if (ledStatus == HIGH) server.send(200, "text/plain", "HIGH");
    //else server.send(200, "text/plain", "LOW");
}

// Toggle the LED and back its status
void toggleLED() {
    //ledStatus = ledStatus == HIGH ? LOW : HIGH;
    //digitalWrite(LED_PIN, ledStatus);
    //sendStatus();
    
}
