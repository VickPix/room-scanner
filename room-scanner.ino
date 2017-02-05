#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h> 
#include <ESP8266mDNS.h>
#include "FS.h"
#include <Servo.h>

#define trigPin D5 // pin trigger ultrasuoni
#define echoPin D6 // pin echo ultrasuoni
#define servoPin D7 // pin di controllo servo
#define highPin D0 // pin sempre alto
#define modePin D1 // pin utilizzato per un controllo (alto/basso)
#define lowPin D2 // pin sempre basso
#define MAX_DISTANCE 300 // massima distanza stabilita

// dichiarazioni
const char* ssid = "test";
const char* password = "123456789";
long duration;
int distance;
bool autoConnect;
WiFiServer server2(80);
ESP8266WebServer server(80);
WiFiClient client;
String HTTP_req;
Servo scanServo;
int pos=0;

void setup() {
    // inizializzazione dei pin
    pinMode(trigPin, OUTPUT); //t
    pinMode(echoPin, INPUT); //e
    pinMode(highPin, OUTPUT);
    pinMode(lowPin, OUTPUT);
    pinMode(modePin, INPUT); 
    
    digitalWrite(trigPin,LOW); 
    digitalWrite(highPin,HIGH);
    digitalWrite(lowPin,LOW);

    scanServo.attach(servoPin);
    scanServo.write(0);
    
    Serial.begin(115200);

    // inizializzazione della partizione spiffs
    SPIFFS.begin();

    // setup del wifi
    setupWiFi();    
  
    // setup del server (index e chiamate asincrone)
    server.on("/", handleRoot);
    // Handlers for various user-triggered events
    server.on("/getRandom", getRandom);
    server.on("/pulse", pulseIndex);
    
    // Start the server
    server.begin();
    Serial.println("Server started");
}

void loop() {
    server.handleClient();
}

void setupWiFi(){
    /*WiFi.mode(WIFI_STA);
    //WiFi.softAP ("Scanner", "123456789");
    //IPAddress IP = WiFi.softAPIP (); 
    //Serial.println(IP.toString());
  
    WiFi.begin(ssid, password); 

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());*/

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
    WiFi.begin(ssid, password);
  
    uint8_t timeout = 10; // 10 * 500 ms = 5 sec time out
    while ( ((ret = WiFi.status()) != WL_CONNECTED) && timeout-- ) {
      Serial.print(".");
      delay(1000);
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
      WiFi.softAP("ESP","123456789");
      WiFi.softAPIP();
      Serial.print("AP IP address "); 
      Serial.println(WiFi.softAPIP());
    }
  
    Serial.println("Starting loop");
}

int calculateDistance(){ // calcolo della distanza con il sensore ultrasuoni
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin,HIGH);
  distance=duration*0.034/2;
  if(distance>MAX_DISTANCE) return MAX_DISTANCE;
  return distance;
}  

void getRandom(){ // scansione delle distanze con il servo motore
  int i = 0, j = 0, it = 3;
  int a,b=-1,aOld=-1;
  int passi = 90;
  int distances[passi];
  for(i=0; i<180; i += (180/passi)){
    scanServo.write(i);
    delay(10);
  }
  String s="[";
  for (i = 180; i >= 0; i -= (180/passi)) { 
    for(j=0;j<it;j++){
      a += calculateDistance();
      delay(15);
    }
    a = a/it; 
    
    /*if( aOld >= 0){
      b = (aOld+a)/2;
      s += String(b);
    }else{
      b = a;
    }*/
    s += String(a);
    //aOld=a;
    
    s+=",";
    scanServo.write(i);
    a=0;              
    delay(10);                       
  }
  s+=String(calculateDistance()); 
  s+="]";
  server.send(200, "application/json", s);
}



void printJson(){ // test di stampa di un json
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Access-Control-Allow-Origin: *\r\n";
  s += "Content-Type: text/plain\r\n\r\n";
  s += "[1,2,3,4]";
  Serial.print(s);
  server.send(200, "application/json", "[100,222,300,400]");
}


void handleRoot() { // index
    // Just serve the index page from SPIFFS when asked for
    File index = SPIFFS.open("/index.html", "r");
    server.streamFile(index, "text/html");
    index.close();
}

void pulseIndex() { // index2
    // Just serve the index page from SPIFFS when asked for
    File index = SPIFFS.open("/index_pulse.html", "r");
    server.streamFile(index, "text/html");
    index.close();
}


void sendStatus() {
    Serial.println(digitalRead(modePin));
}

