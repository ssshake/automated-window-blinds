#include <Servo.h>  

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

int debug = 1; //Set this to 1 for serial debug output

void setup(void) {
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled    
    myservo.attach(servoPin);  
}


void loop(void) {
		photocellReading = analogRead(photocellPin); //Query photo cell
	  debug and Serial.print("Light Reading :");
	  debug and Serial.print(photocellReading); // the raw analog reading
	  debug and Serial.print(" | Pos: ");
	  debug and Serial.print(pos);    
	  debug and Serial.print(" | State: ");

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



  
  
