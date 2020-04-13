/*
 * Injector Control Code
 * This code controls a single fuel injector, firing once per revolution.  Created by Richard Moyer (ramoyer@alum.mit.edu) 2.014 Spring 2018
 * This uses the time between rotations, dt, for calculations, as this is easier to measure than engine speed.    
 * Speed (in rpm) = 60,000,000/dt (in µs)
 */

// Change the following lines as needed

// pin numbers:
#define THROTTLEPIN 7   // Analog input from potentiometer to control speed.  REQUIRED TO BE AN ANALOG INPUT
#define INJECTORPIN 12  // Output to injector circuit.  This goes through a 200 Ω resistor (limits current to 25mA) to a darlington transistor/mosfet base for injection control
#define TIMINGPIN 2     // Input from Hall Effect sensor, or equivalently tachometer connection on ignition module. LOOK UP WIRING DIAGRAMS FOR THIS as hall effect sensor pull the sense pin low, but need an external resistor attached to pull it high when not sensing a magnet. 
#define LEDPIN 13       // Makes the on-board LED turn on whenever the injector does

// values
#define DT_LOW 5000     // Rotation time at full speed, in µs.  = 60,000,000/rpm
#define DT_HIGH 50000   // Rotation time at idle, in µs.  = 60,000,000/rpm
const unsigned long MAXFUEL = 1200;  // microseconds of flow for one cylinder of stochiometric fuel INSERT CALIBRATED VALUE HERE 









// DON'T CHANGE CODE BELOW THIS LINE UNLESS YOU UNDERSTAND THE CODE (or do so at your own risk)

const unsigned long FACTOR = 0.2;    // what fraction the engine speed has to drop from the set speed before full throttle is applied.  Required to be between 0 and 1, should be about 0.2
volatile unsigned long dtset = 0; // in microseconds
volatile unsigned long dtlow = 0; // in microseconds
int flag = 0;                   // Has to go high then low before injection, used to read edges of a square wave from the hall effect sensor
volatile byte inject = 0;       // Triggers injection
volatile unsigned long dt = 0;  // Time between rotations, in microseconds
volatile unsigned long lastMicros = 0; // time storage variable in microseconds
volatile long injectionDuration = 0; // injector pulse length in microseconds.  


void setup() {
  // put your setup code here, to run once:
  pinMode(TIMINGPIN,INPUT);
  pinMode(THROTTLEPIN,INPUT);
  pinMode(LEDPIN,OUTPUT);
  pinMode(INJECTORPIN,OUTPUT);
  
}

void loop() {
  
  //Check for edges, and set dt to the time since last trigger
  if (digitalRead(TIMINGPIN) == HIGH){
    flag = 1;
  }
  if (digitalRead(TIMINGPIN)== LOW && flag == 1){
    inject = 1;
    dt = micros() - lastMicros;
    lastMicros = micros();
    flag = 0;
  }

  // Injection Sequence
  if (inject == 1)
  {
    
    //Read setpoint from potentiometer.
    dtset = map(analogRead(THROTTLEPIN),0,1023,DT_HIGH,DT_LOW);// Use for potentiometer throttle control
    // dtset = 5000; // Uncomment this line for a constant set speed of 12,000 rpm 
    
    
    // if dt is outside of the control band, set it to the edges
    if (dt > (dtset + FACTOR*dtset)){
      injectionDuration = MAXFUEL;
    }
    else if (dt < dtset){
      injectionDuration = 0;
    }

    // else, set it to error * gain
    else{
      injectionDuration = (dt - dtset)*MAXFUEL/FACTOR/dtset;// Error*actuator scaling*gain.  (dt-dtset)/dtset = error.  1/FACTOR = gain
    }  

    // Fire injector
    digitalWrite(INJECTORPIN,HIGH);
    digitalWrite(LEDPIN,HIGH);
    delayMicroseconds(injectionDuration);
    digitalWrite(INJECTORPIN,LOW);
    digitalWrite(LEDPIN,LOW);

    // clear the inject variable
    inject = 0;
  }
}// end loop
