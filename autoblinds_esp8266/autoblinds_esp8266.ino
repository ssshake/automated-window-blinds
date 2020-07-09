#include <Servo.h>  
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
int debug = 0;

WiFiServer server(80);

int servoPin=5;
int photocellPin = A0;
int pos = 180;
int spd = 50;

Servo myservo;
int photocellReading;

//questionable
int state = 0;
int prevstate = 0;
int dest = 0;
int lightBuffer = 20;
int noStateChangeInterval = 4000;
int lastStateChangeTimestamp = 0;

String stateNames[]       = { "night", "dusk", "day" };
int stateUpperLimits[]    = { 400, 600, 1024 };
int stateAngleInDegrees[] = { 180, 135, 85 };

int stateUpperLimitsSize = sizeof stateUpperLimits;
String currentStateName = "";


//new variables
//handle without delay
unsigned long previousMillis = 0;
const long checkInterval = 1000;

unsigned long previousNextPositionMillis = 0;
const long nextPositionInterval = 50;

void loop(void) {  
    handleWebClients();
    handleBlindAutomationWithoutDelay();
    ArduinoOTA.handle(); //port 8266
}

void handleBlindAutomationWithoutDelay(){

  if (hasServoTimeoutPassed()){
    updateServo();
  }
  
  if (hasCheckTimeoutPassed() && hasStateChangeTimeoutPassed() && pos == dest){
    updateStateFromLight();
  }
  
}

void updateServo(){
  if (pos == dest) {
    myservo.detach();
    return;
  }

  if (pos < dest) {
    myservo.attach(servoPin);
    pos++;
    myservo.write(pos);
    return;
  }
  
  if (pos > dest) {
    myservo.attach(servoPin);
    pos--;
    myservo.write(pos);
    return;
  }  
}

void updateStateFromLight(){
  photocellReading = analogRead(photocellPin);
  photocellStats();
  for (int i=0; i < stateUpperLimitsSize; i++) {

      
      if ( photocellReading <= stateUpperLimits[i] ){
          Serial.println("--> Found A State ");

          //if the state didnt change dont update anything
          if (i != prevstate){
            Serial.println("--> State updated");
            lastStateChangeTimestamp = millis();
            currentStateName = stateNames[i];
            dest = stateAngleInDegrees[i];
            prevstate = state;
            state = i;
          }

          break;
          
      }
  }
}

bool hasServoTimeoutPassed(){
  unsigned long currentMillis = millis();
  bool answer;
  
  if (currentMillis - previousNextPositionMillis >= nextPositionInterval) {
    answer = true;
    previousNextPositionMillis = currentMillis;
  } else {
    answer = false;  
  }
  
  return answer;
}

bool hasCheckTimeoutPassed(){
  unsigned long currentMillis = millis();
  bool answer;
  
  if (currentMillis - previousMillis >= checkInterval) {
    answer = true;
    previousMillis = currentMillis;
  } else {
    answer = false;  
  }
  
  return answer;
}

bool hasStateChangeTimeoutPassed(){
  Serial.println("hasStateChangeTimeoutPassed");
  unsigned long currentMillis = millis();
  bool answer;
  
  if (currentMillis - lastStateChangeTimestamp >= noStateChangeInterval) {
    Serial.println("State change interval has passed");
    answer = true;
  } else {
    Serial.println("hasStateChangeTimeoutPassed ==== false");
    answer = false;  
  }
  
  return answer;  
}

void photocellStats(){
    Serial.print("Light Reading :");
    Serial.print(photocellReading); // the raw analog reading
    Serial.print(" | Pos: ");
    Serial.print(pos);    
    Serial.print(" | State: ");
    Serial.println(state);
}

void handleWebClients(){
  // Check if a client has connected
 
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  // Match the request
 
  if (request.indexOf("/stats") != -1)  {
    photocellStats();
  }
  if (request.indexOf("/STATE=0") != -1)  {
    Serial.println("closed");
    dest=180;
  }
  if (request.indexOf("/STATE=2") != -1)  {
    Serial.println("open");
    dest=0;
  }
  
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Blinds are now ");
 
  client.println("<br><br>");
  client.println("<a href=\"/STATE=0\"\"><button>Closed</button></a>");
  client.println("<a href=\"/STATE=2\"\"><button>Open</button></a><br />");  
  client.println("<a href=\"/stats\"\"><button>Serial Stats</button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");  
}

void setupServo(){
    myservo.attach(servoPin);
    myservo.write(180);
    delay(1000);
    myservo.write(0);
    delay(1000);    
    myservo.write(100);
    myservo.detach();  
    delay(1000);
    pos = 100;
    dest = 100;
}

void setupWifi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("HomeAwesomation");
    startWebServer();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());    
}

void setupOTA(){
ArduinoOTA.setPort(8266);

    ArduinoOTA.setHostname("homeawesomation");
    
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }
  
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

    ArduinoOTA.begin();  
}

void setup(void) {
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled    

    setupServo();
    setupWifi();
    setupOTA();
    
    Serial.println("Ready");
}

void startWebServer(){
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}
  
