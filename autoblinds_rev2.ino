//Automated Window Blinds program
//This program is meant to be connected to 2" venetian window blinds using a servo directly connected to the blinds spool
//It will sample a photocell (light dependant resistor) and depending on the level of brightness act accordingly
//This script can also be used with a tmp36 temperature sensor to tilt up when it's very hot out
//Currently it will tilt up when it's very bright out, but ideally we want this do be done by temperature.
//Written by Ssshake - Contact arduinocode@doobiest.net for help or to submit contributes to the code line.
#include <Servo.h>  
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
int pos = 180;    // variable to store the servo position 
int dest = 0;   // Servo destination depending on photocell reading
int spd = 50;   // how fast should the servo move? 50 is quier

int servoPin=0; //havent used this yet
int photocellPin = 0; // the cell and 10K pulldown are connected to a0
int photocellReading; // the analog reading from the analog resistor divider

const int button_pin = 7 ; //Button (relay driven by x10 switch)
int button_value;  //high or low button values

int state = 0; //Keep track of state so we don't send signal to the servo without readon, better on battery life
int prevstate = 0;

int debug = 1; //Set this to 1 for serial debug output
void setup(void) {
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled
    myservo.attach(9); 
}


void loop(void) {

	photocellReading = analogRead(photocellPin); //Query photo cell
		  debug and Serial.print("Light Reading :");
		  debug and Serial.print(photocellReading); // the raw analog reading
		  debug and Serial.print(" | Position: ");
		  debug and Serial.print(pos);    
		  debug and Serial.print(" | State: ");

    //Define the modes based on how bright it is, and set corresponding servo position

	//Beginning of loop is night, set dest to closed and run ldr_press
   	  debug and Serial.println("Night");
	dest=180;      
	//state=1;
	ldr_press();
	
	//After setting LDR position to NIGHT, hold until not longer within current threshold
	//While holding, poll button/relay for presses. IF pressed then user intervention on position via button_press()
	while (photocellReading > 0 && photocellReading < 400) {
      	button_press();	
    } 
    
	  debug and Serial.println("Dusk");
	dest=135;      
	//state=2;    
	ldr_press(); 

	//After setting LDR position, hold until not longer within current threshold  
	while ( photocellReading > 400 && photocellReading < 600) {
      	button_press();	
    } 
    
	  debug and Serial.println("Day");
	dest=85;
	//state=3;
	ldr_press(); 

	//After setting LDR position, hold until not longer within current threshold  
	while (photocellReading > 600 && photocellReading < 950) {
      	button_press();	      
    } 
    
	  debug and Serial.println("Very Bright Day");
	dest=20;
	//state=4;
	ldr_press();

	//After setting LDR position, hold until not longer within current threshold	
	while (photocellReading > 950 && photocellReading < 1023) {
      	button_press();	
    } 
    
	
	
	
	
	
  //if (state != prevstate){ //IF the photocell reading is different from last sample then execute servo controls
  //		ldr_press();
  // }
  //
  //prevstate = state; //Remember state so we can compare it again next round
  //delay(1000); //Optional delay, this probalby needs to be removed when IR receiver code get's added.    

}

void ldr_press() {
	  debug and Serial.println("State Change");      
	myservo.attach(9);                     //Connect to servo
	if (pos > dest){  // If the current position is great than the destination then we must subtract
		  //for(pos = pos; pos>=dest; pos-=1)     // Change current position to desired position, one degree at a time.
		  while (pos > dest)  // Change current position to desired position, one degree at a time.
		  {                                
			  debug and Serial.print("Was :");
			  debug and Serial.print(pos);
			myservo.write(pos);         // tell servo to go to position in variable 'pos' 
			delay(spd);                 // waits desired time for the servo to reach the position 
			pos--;
			  debug and Serial.print(" | Is :");
			  debug and Serial.println(pos);
		  } 
		myservo.detach();                //Detach from Servo
	} 
	else {  // If the curren't position is greater than the destination then we must add
		myservo.attach(9);  //Connect to servo
		  //for(pos = pos; pos <= dest; pos+=1)     // goes from 180 degrees to 0 degrees 
		  while (pos < dest)  // goes from 180 degrees to 0 degrees 
		  {                        
			  debug and Serial.print("Was :");
			  debug and Serial.print(pos);        
			myservo.write(pos);             // tell servo to go to position in variable 'pos' 
			delay(spd);                     // waits desired time for the servo to reach the position 
			pos++;
			  debug and Serial.print(" | Is :");
			  debug and Serial.println(pos);
		  } 
	}
	myservo.write(pos); // Doing a write out side of the loop because I had a feeling the last position value was being skipped. I think I'm wrong though
	delay(spd);  
	myservo.detach(); //Detach servo to keep it from humming on strain
}




//
//
//I have a feeling this whole section isn't require
// I think you can just call ldr_presses where you do button_pressed because the photocell doesnt actually get polled in ldr_press
// So you can have slow movement probalby using the same logic or copying it.
//While holding a position/threshold, poll button for status.
void button_press(){
  
  
   //is this necessary??
    button_value = digitalRead(button_pin);
	photocellReading = analogRead(photocellPin); //Query photo cell
		  debug and Serial.print("Light Reading :");
		  debug and Serial.print(photocellReading); // the raw analog reading
		  debug and Serial.print(" | Position: ");
		  debug and Serial.print(pos);    
		  debug and Serial.println(" | State: ");

    //Define the modes based on button press. Cycle through options
    if(button_value==HIGH){  // button press
    
      //THIS MIGHT WORK?
      //ldr_press();
    
      //INSTEAD OF THIS...
      
        myservo.attach(9); //connect to servo
			debug and Serial.println("button press"); 
			
		if (pos <= 20) { //20 degrees is pointing up. go down a bit.
			dest=85;		//new dest is lower than prev dest (servo arrangement is 180 is fully down an 0 is fully up in other words backwards for what youd expect
			myservo.write(dest);	//Move to next position
			pos=dest;				//set position status to what we just moved it o
				debug and Serial.println("Very Bright Day");
				debug and Serial.println(dest);
		} 
		
		else if (pos <= 85) { //move to next position
			dest=135;
			myservo.write(dest);
			pos=dest;
				debug and Serial.println(dest);
				debug and Serial.println("Day");
		} 
		
		else if (pos <= 135) {//move to next position
			dest=180;      
			myservo.write(dest);          
			pos=dest;
				debug and Serial.println("Dusk");		
				debug and Serial.println(dest);				  	  
		} 
		
		else if (pos <= 180) {//move to next position
			dest=20;   
			myservo.write(dest);
			pos=dest;
				debug and Serial.println(dest);
				debug and Serial.println("Night");		
		} 
		delay(2000);	//Since we're moving the servo from one end of the 180 degree chart to another in one shot we need to give it time to get there before moving on
		myservo.detach();  //Detach servo, because if servo is say destined for 80 degrees, but to much weight/lack of torque keeps it hovering at 79-79.5 degrees, the motor will keep trying and humm
							// detaching shuts the motor off, thus making it rest at 79 degrees instead of trying too hard.
       
    }		
}


  
  
