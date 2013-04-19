//Automated Window Blinds program
//This program is meant to be connected to 2" venetian window blinds using a servo directly connected to the blinds spool
//It will sample a photocell (light dependant resistor) and depending on the level of brightness act accordingly
//This version makes use of a TCRT5000 IR reflective sensor to off touch free manual control
//There is also a physical momentary button which exectutes the same code as the ir sensor
//This same switch can be tied into a home automation relay switch for control via computer, I use x10 for example.
//I have also included a kill switch to disable automation when needed
//This script can also be used with a tmp36 temperature sensor to tilt up when it's very hot out
//Currently it will tilt up when it's very bright out, but ideally we want this do be done by temperature.
//Written by Ssshake - Contact arduinocode@doobiest.net for help or to submit contributes to the code line.
//All products developed by ssshake c/o homeawesomation is licensed under GPLv3.
//You can view the license here: http://www.gnu.org/licenses/gpl-3.0.html


#include <Servo.h>  


//CONFIGURATION
int servoPin=9; //Input pin for servo
int photocellPin = 0; // the cell and 10K pulldown are connected to a0
const int button_pin = 2 ; //Button or relay driven by x10 switch for manual control
const int killswitch = 4 ; //Disables all automated actions when enabled
int sensorPin = 1;    //IR Reflective motion sensor pin
const int ledPin = 11;      // select the pin for the LED
//PROBABLY DONT NEED TO CHANGE THESE
int pos = 180;    // variable to store the servo position 
int spd = 50;   // how fast should the servo move? 50 is quiet


Servo myservo;  // create servo object to control a servo 
int photocellReading; // the analog reading from the analog resistor divider
int button_value;  //high or low button values
int killswitch_value;  //high or low killswitch values
int state = 0; //Keep track of state so we don't send signal to the servo without reason
int prevstate = 0; //Keeps history of prior state
int sensorValue = 0;  // variable to store the value coming from the sensor
int dest = 0;   // Servo destination depending on photocell reading
int buttonpos =1; //Keep state of motion sensor/button for movement
//ENABLE DEBUG
int debug = 0; //Set this to 1 for serial debug output
void setup(void) {
   //Serial.begin(9600); //Comment out this line if you don't want debugging enabled
    
    myservo.attach(servoPin); 
    //pinMode(ledPin, OUTPUT);
    pinMode(button_pin, INPUT_PULLUP);
    pinMode(killswitch, INPUT_PULLUP);  
    
}

//MAIN LOOP
void loop(void) {
killswitch_value = digitalRead(killswitch);
if(killswitch_value==HIGH) //IF kill switch isn't set then run main loop
	{  
		photocellReading = analogRead(photocellPin); //Query photo cell
		//  debug and Serial.print("Light Reading :");
		 // debug and Serial.print(photocellReading); // the raw analog reading
		  //debug and Serial.print(" | Pos: ");
		  //debug and Serial.print(pos);    
		  //debug and Serial.print(" | State: ");

		//Define the modes based on how bright it is, and set corresponding servo position
		if (photocellReading > 0 && photocellReading < 400) {
				button_press();	
//					debug and Serial.println("Night");
				dest=180;      
				state=1;
		}  

		//After setting LDR position, hold until not longer within current threshold  
		else if ( photocellReading > 410 && photocellReading < 600) {
				button_press();	    
//					debug and Serial.println("Dusk");
				dest=135;      
				state=2; 
		}

		//After setting LDR position, hold until not longer within current threshold  
		else if (photocellReading > 610 && photocellReading < 940) {
				button_press();	      
//					debug and Serial.println("Day");
				dest=105;
				state=3;
		}

		//After setting LDR position, hold until not longer within current threshold	
		else if (photocellReading > 950 && photocellReading < 1023) {
				button_press();	
//					  debug and Serial.println("Very Bright Day");
				dest=20;
				state=4;
		} 
		else {
//			debug and Serial.println(photocellReading);	
//			debug and Serial.println("Not in range");	
			button_press();	 
		}
		
		if (state != prevstate){ //IF the photocell reading is different from last sample then execute servo controls
                        debug and Serial.print("State Change to ");
                        debug and Serial.println(state);
			ldr_press();
	 	}
		prevstate = state; //Remember state so we can compare it again next round
	}
else
	{
		debug and Serial.println("Kill Switch");
		delay(4000);
	}
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


//This runs when the system is idle.. as in not moving already from one position to another
void button_press(){

    sensorValue = analogRead(sensorPin);   //Query motion sensor
    button_value = digitalRead(button_pin); //Query button or relay switch
//                 debug and Serial.print("Motion Sensor Value: ");
 //                debug and Serial.println(sensorValue);
                 
                 
    //Define the modes based on button press. Cycle through options
    if(button_value==LOW || sensorValue < 900){  // button press
        spd=10;   // how fast should the servo move? 50 is quie
	debug and Serial.println("button press"); 
	debug and Serial.println(sensorValue);

		if (pos <= 20) { //0 degrees closed upward.
			dest=105;		//new dest is lower than prev dest (servo arrangement is 180 is fully down an 0 is fully up in other words backwards for what youd expect
                        buttonpos=0;
				debug and Serial.print(dest);
				debug and Serial.println(" Degree");
		} 

                		else if (pos <= 105  && buttonpos == 0 ) { //move to next position
                			dest=135;
                				debug and Serial.print(dest);
				debug and Serial.println(" Degree");
                	    	} 
                		
                		else if (pos <= 135  && buttonpos == 0 ) {//move to next position
                			dest=180;      
                				debug and Serial.print(dest);				  	  
				debug and Serial.println(" Degree");
                		} 
                		
              

                		else if (pos <= 105  && buttonpos == 1 ) { //move to next position
                			dest=20;
                				debug and Serial.print(dest);
				debug and Serial.println(" Degree");
                		} 
                		
                		else if (pos <= 135  && buttonpos == 1) {//move to next position
                			dest=105;      
                				debug and Serial.print(dest);				  	  
				debug and Serial.println(" Degree");
                		} 


		else if (pos <= 180) {//move to next position
			dest=135;   
                        buttonpos=1;
				debug and Serial.print(dest);
				debug and Serial.println(" Degree");
		} 
      ldr_press();
      spd=50;
      //delay(500);
    }		
}


  
  

