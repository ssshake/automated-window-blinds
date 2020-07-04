#include <Servo.h>  
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

WiFiServer server(80);

int servoPin=5;
int photocellPin = A0;
int pos = 180;
int spd = 50;


Servo myservo;
int photocellReading;
int state = 0;
int prevstate = 0;
int sensorValue = 0;
int dest = 0;

int debug = 1;

void loop(void) {
    
    handleBlindAutomation();
    handleWebClients();
    ArduinoOTA.handle(); //port 8266
}

void setup(void) {
    
    WiFiManager wifiManager;
    wifiManager.autoConnect("HomeAwesomation");
    
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled    
    
    myservo.attach(servoPin);

    startWebServer();
 

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
    
    Serial.println("Ready");
    
    Serial.print("IP address: ");
    
    Serial.println(WiFi.localIP());
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
  if (request.indexOf("/LED=OFF") != -1)  {
    Serial.println("low");
  }
 
// Set ledPin according to the request
//digitalWrite(ledPin, value);
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Led pin is now: ");
 
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");  
}

void photocellStats(){
    Serial.print("Light Reading :");
    Serial.print(photocellReading); // the raw analog reading
    Serial.print(" | Pos: ");
    Serial.print(pos);    
    Serial.print(" | State: ");
    Serial.println(state);
}

void handleBlindAutomation(){
  
    photocellReading = analogRead(photocellPin); //Query photo cell
  
    if (debug) {
      photocellStats();
    }
    
    return;


    
    if (photocellReading > 0 && photocellReading < 400) {
        debug and Serial.println("Night");
        dest=180;      
        state=1;
    }  

    else if ( photocellReading > 400 && photocellReading < 600) { 
        debug and Serial.println("Dusk");
        dest=135;      
        state=2; 
    }

    else if (photocellReading > 600 && photocellReading < 950) {
        debug and Serial.println("Day");
        dest=85;
        state=3;
    }

    else if (photocellReading > 950 && photocellReading < 1023) {
        debug and Serial.println("Very Bright Day");
        dest=20;
        state=4;
    } 
    else {
      debug and Serial.println(photocellReading); 
      debug and Serial.println("Not in range"); 
    }
    
    if (state != prevstate){
      handleLightChange();
    }
    prevstate = state;
}

void handleLightChange() {
	debug and Serial.println("State Change");      
	myservo.attach(servoPin);
	if (pos > dest){
		while (pos > dest)
		{                                
			myservo.write(pos);
			delay(spd);
			pos--;
		} 
		myservo.detach();
	} 
	else {
		myservo.attach(servoPin);
		while (pos < dest)
		{                         
			myservo.write(pos);
			delay(spd); 
			pos++;
		} 
	}
	myservo.write(pos);
	delay(spd);  
	myservo.detach();
}

  
