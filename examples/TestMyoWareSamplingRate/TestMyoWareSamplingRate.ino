/*
  Test MyoWare Sample Rate Example Code
  Advancer Technologies, LLC
  Brian Kaminski
  2/24/2024

  This example calculates the sampling rate of a microcontroller reading muscle
  activity from a MyoWare Muscle Sensor.
  
  The MyoWare Muscle Sensor is an analog sensor therefore the sampling rate
  will be entirely determined by the hardware specifications of the Arduino and the 
  performance of the code used.

  For the most accurate sampling rate, insert your code inside the loop indicated 
  below. This will more closely mirror your intended use case to see how additional 
  commands/calls would impact the sampling rate.

  Hardware:
  MyoWare 2.0 Muscle Sensor
  Arduino-compatible Microcontroller

  This example code is in the public domain.
*/

#include <MyoWare.h>

const long samples = 1000000;  // number of samples

// MyoWare class object
MyoWare myoware;

void setup() 
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Sampling Test");
  Serial.println("------------------------------");

  myoware.setENVPin(A3);                      // Arduino pin connected to ENV

  pinMode(myoware.getStatusLEDPin(), OUTPUT); // initialize the built-in LED pin to indicate 
                                              // when a central is connected
  digitalWrite(myoware.getStatusLEDPin(), LOW);
}

void loop() 
{
  // initialize variables
  int testValue = 0; 

  digitalWrite(myoware.getStatusLEDPin(), HIGH); 
  Serial.println("Start Time:\t" + String(micros()) + " microsec");

  // perform numerous read actions
  // modify this to read all the output pins on the MyoWare Muscle Sensor that
  // you are planning to use in your setup.
  unsigned long startMicros = micros(); // variable for starting time in microseconds 
  for (long i = 0; i < samples; i++) 
  {

    // INSERT YOUR CODE HERE
    
    testValue = myoware.readSensorOutput(MyoWare::ENVELOPE);
  }
  const unsigned long endMicros = micros();
  
  Serial.println("Finish Time:\t" + String(endMicros) + " microsec");
  digitalWrite(myoware.getStatusLEDPin(), LOW);
  
  // calculate the microseconds per sample (elapsed_time / number_of_samples)
  // then convert to seconds
  const double secondsPerSample = (endMicros - startMicros) / samples;

  // write out the sampling rate in Hz
  Serial.println("Sampling Rate:\t" + String((1.0 / secondsPerSample) * 1000000) + " Hz");
  Serial.println("------------------------------");
  delay(1000);
}
