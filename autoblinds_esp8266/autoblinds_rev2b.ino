#include <Servo.h>  
#include <ESP8266WiFi.h>

const char* ssid = "asdf";
const char* password = "asdf";

WiFiServer server(80);

int servoPin=5; //Input pin for servo
int photocellPin = A0; // the cell and 10K pulldown are connected to a0
int pos = 180;    // variable to store the servo position 
int spd = 50;   // how fast should the servo move? 50 is quiet


Servo myservo;  // create servo object to control a servo 
int photocellReading; // the analog reading from the analog resistor divider
int state = 0; //Keep track of state so we don't send signal to the servo without reason
int prevstate = 0; //Keeps history of prior state
int sensorValue = 0;  // variable to store the value coming from the sensor
int dest = 0;   // Servo destination depending on photocell reading

int debug = 0; //Set this to 1 for serial debug output

void setup(void) {
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled    
    myservo.attach(servoPin);
    startWebServer();
}

void startWebServer(){
    // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
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
}

void handleBlindAutomation(){
    photocellReading = analogRead(photocellPin); //Query photo cell
    if (debug) {
      photocellStats();
    }
    
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
      ldr_press();
    }
    prevstate = state;
}


void loop(void) {
    
		//handleBlindAutomation();
    handleWebClients();
}

void ldr_press() { //This executes when the photocell readings have dictated
	  debug and Serial.println("State Change");      
	myservo.attach(servoPin);                     //Connect to servo
	if (pos > dest){  // If the current position is great than the destination then we must subtract
		while (pos > dest)  // Change current position to desired position, one degree at a time.
		{                                
			  //debug and Serial.print("Was :");
			  //debug and Serial.print(pos);
			myservo.write(pos);         // tell servo to go to position in variable 'pos' 
			delay(spd);                 // waits desired time for the servo to reach the position 
			pos--;
			  //debug and Serial.print(" | Is :");
			  //debug and Serial.println(pos);
		} 
		myservo.detach();                //Detach from Servo
	} 
	else {  // If the curren't position is greater than the destination then we must add
		myservo.attach(9);  //Connect to servo
		while (pos < dest)  // goes from 180 degrees to 0 degrees 
		{                        
			  //debug and Serial.print("Was :");
			  //debug and Serial.print(pos);        
			myservo.write(pos);             // tell servo to go to position in variable 'pos' 
			delay(spd);                     // waits desired time for the servo to reach the position 
			pos++;
			  //debug and Serial.print(" | Is :");
			  //debug and Serial.println(pos);
		} 
	}
	myservo.write(pos); // Doing a write out side of the loop because I had a feeling the last position value was being skipped. I think I'm wrong though
	delay(spd);  
	myservo.detach(); //Detach servo to keep it from humming on strain
}



  
  
