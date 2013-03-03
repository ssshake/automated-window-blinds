//Automated Window Blinds program
//This program is meant to be connected to 2" venetian window blinds using a servo directly connected to the blinds spool
//It will sample a photocell (light dependant resistor) and depending on the level of brightness act accordingly
//This script can also be used with a tmp36 temperature sensor to tilt up when it's very hot out
//Currently it will tilt up when it's very bright out, but ideally we want this do be done by temperature.
//Written by Ssshake - Contact arduinocode@doobiest.net for help or to submit contributes to the code line.
#include <Servo.h>  
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
int pos = 0;    // variable to store the servo position 
int dest = 0;   // Servo destination depending on photocell reading
int spd = 50;   // how fast should the servo move? 50 is quier

int servoPin=0; //havent used this yet
int photocellPin = 0; // the cell and 10K pulldown are connected to a0
int photocellReading; // the analog reading from the analog resistor divider

int state = 0; //Keep track of state so we don't send signal to the servo without readon, better on battery life
int prevstate = 0;

int debug = 1; //Set this to 1 for serial debug output
void setup(void) {
    Serial.begin(9600); //Comment out this line if you don't want debugging enabled
}


void loop(void) {

    photocellReading = analogRead(photocellPin); //Query photo cell
      debug and Serial.print("Light Reading :");
      debug and Serial.print(photocellReading); // the raw analog reading
      debug and Serial.print(" | Position: ");
      debug and Serial.print(pos);    
      debug and Serial.print(" | State: ");

    //Define the modes based on how bright it is, and set corresponding servo position
    if (photocellReading < 400) {
        debug and Serial.println("Night");
      dest=0;      
      state=1;
    } 
    else if (photocellReading < 500) {
        debug and Serial.println("Dusk");
      dest=45;      
      state=2;           
    } 
    else if (photocellReading < 700) {
        debug and Serial.println("Day");
      dest=95;
      state=3;
      
    } 
    else if (photocellReading < 1023) {
        debug and Serial.println("Very Hot out");
      dest=160;
      state=4;
    } 
    else {
        debug and Serial.println("No reading");      
    }
    


    if (state != prevstate){ //IF the photocell reading is different from last sample then execute servo controls
          debug and Serial.println("State Change");      
        myservo.attach(9);                     //Connect to servo
        if (pos > dest){  // If the current position is great than the destination then we must subtract
              //for(pos = pos; pos>=dest; pos-=1)     // Change current position to desired position, one degree at a time.
              while (pos > dest)
              {                                
                  debug and Serial.print("Was :");
                  debug and Serial.print(pos);
                myservo.write(pos);              // tell servo to go to position in variable 'pos' 
                delay(spd);                 // waits 15ms for the servo to reach the position 
                pos--;
                  debug and Serial.print(" | Is :");
                  debug and Serial.println(pos);
              } 
            myservo.detach();                //Detach from Servo
        } 
        else {  // If the current position is great than the destination then we must add
            myservo.attach(9);
              //for(pos = pos; pos <= dest; pos+=1)     // goes from 180 degrees to 0 degrees 
              while (pos < dest)
              {                        
                  debug and Serial.print("Was :");
                  debug and Serial.print(pos);        
                myservo.write(pos);              // tell servo to go to position in variable 'pos' 
                delay(spd);                     // waits 15ms for the servo to reach the position 
                pos++;
                  debug and Serial.print(" | Is :");
                  debug and Serial.println(pos);
              } 
        }
        myservo.write(pos); // Doing a write out side of the loop because I had a feeling the last position value was being skipped. I think I'm wrong though
        delay(spd); 
        myservo.detach();
    }
    prevstate = state; //Remember state so we can compare it again next round
    delay(1000); //Optional delay, this probalby needs to be removed when IR receiver code get's added.    
}
