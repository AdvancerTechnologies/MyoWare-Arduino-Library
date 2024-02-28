/*
  MyoWare BLE Peripheral Example Code
  Advancer Technologies, LLC
  Brian Kaminski
  8/01/2023

  This example sets up a MyoWare 2.0 Wireless Shield, and then reads the ENV, RAW,
  and REF data from the attached MyoWare 2.0 Muscle Sensor. The MyoWare 2.0
  Wireless Shield (the Peripheral) sends this data to a second BLE Device
  (the Central) over BLE.

  This MyoWare 2.0 Wireless Shield, aka the "BLE Peripheral", will read the sensor's
  output on A3-A5 where A3 is ENV, A4 is RAW, and A5 is REF. It will then store
  them in a single 32-bit variable, and then update that value to the
  "bluetooth bulletin board". When uploading to the MyoWare 2.0 Wireless Shield
  make sure to select "ESP32 Dev Module" as the board definition.

  Note, in BLE, you have services, characteristics and values.
  Read more about it here:

  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins reading the ADC and updating the data,
  It first sets up some BLE stuff:
    1. sets up as a peripheral
    2. sets up a service and characteristic (the data)
        -Note, Services and characteristics have unique 128-bit UUID,
        -These must match the UUIDs in the code on the central device.
    3. advertises itself

  In order for this example to work, you will need a ESP32 board,
  and it will need to be programmed with the provided code specific to
  being a central device, looking for this specific service/characteristic.

  Note, both the service and the characteristic get unique UUIDs.

  The "BLE Central", will subscribe to the MyoWare 2.0 Wireless
  Shield's charactieristic, read it, and parse it into 4 separate bytes,
  then print the values to the serial terminal.

  Hardware:
  MyoWare 2.0 Wireless Shield
  MyoWare 2.0 Muscle Sensor
  USB from BLE Device to Computer.

  ** For consistent BT connection follow these steps:
  ** 1. Reset Peripheral
  ** 2. Wait 5 seconds
  ** 3. Reset Central
  ** 4. Enjoy BT connection
  **
  ** ArduinoBLE does not support RE-connecting two devices.
  ** If you loose connection, you must follow this hardware reset sequence again.
  **
  ** ArduinoBLE does not support connecting more than four peripheral devices.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <MyoWare.h>

const String localName = "MyoWareSensor1";  // recommend making this unique for
                                            // each Wireless shield (e.g. MyoWareSensor1,
                                            // MyoWareSensor2, ...)

MyoWare::OutputType outputType = MyoWare::ENVELOPE; // select which output to print to serial
                                                    // EMG envelope (ENVELOPE) or Raw EMG (RAW))

// debug parameters
const bool debugLogging = false;      // set to true for verbose logging
const bool debugOutput = true;        // set to true to print output values to serial

// MyoWare class object
MyoWare myoware;

// BLE Service
BLEService myoWareService(MyoWareBLE::uuidMyoWareService.c_str());

// BLE Muscle Sensor Characteristics
BLEStringCharacteristic sensorCharacteristic(MyoWareBLE::uuidMyoWareCharacteristic.c_str(), BLERead | BLENotify, 128);

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  myoware.setConvertOutput(false);    // Set to true to convert ADC output to the amplitude of
                                      // of the muscle activity as it appears at the electrodes
                                      // in millivolts
  myoware.setGainPotentiometer(50.);  // Gain potentiometer resistance in kOhms.
                                      // adjust the potentiometer setting such that the
                                      // max muscle reading is below 3.3V then update this
                                      // parameter to the measured value of the potentiometer
  myoware.setENVPin(A3);              // Arduino pin connected to ENV (defult is A3 for Wireless Shield)
  myoware.setRAWPin(A4);              // Arduino pin connected to RAW (defult is A4 for Wireless Shield)
  myoware.setREFPin(A5);              // Arduino pin connected to REF (defult is A5 for Wireless Shield)

  pinMode(myoware.getStatusLEDPin(), OUTPUT);  // initialize the built-in LED pin to indicate
                                               // when a central is connected
  digitalWrite(myoware.getStatusLEDPin(), HIGH);

  // begin initialization
  bool error = !BLE.begin();
  if (error)
  {
    Serial.println("FAILED - BLE Initialization!");

    while (error);
  }

  BLE.setLocalName(localName.c_str());
  BLE.setAdvertisedService(myoWareService);
  myoWareService.addCharacteristic(sensorCharacteristic);
  BLE.addService(myoWareService);

  // set initial values for the characteristics
  sensorCharacteristic.writeValue("");

  BLE.advertise();

  if (debugLogging)
  {
    Serial.println("Setup Complete!");
    Serial.print(BLE.address());
    Serial.print(" '");
    Serial.print(localName.c_str());
    Serial.print("' ");
    Serial.print(myoWareService.uuid());
    Serial.println();
    Serial.print("Waiting to connect...");
  }

  digitalWrite(myoware.getStatusLEDPin(), LOW);
}

void loop()
{
  // wait for a BLE central
  BLEDevice central = BLE.central();
  if (central)
  {
    if (debugLogging)
    {
      Serial.print("Connected to central: ");
      Serial.println(central.address());
    }

    digitalWrite(myoware.getStatusLEDPin(), HIGH); // turn on the LED to indicate the
                                                   // connection

    while (central.connected())
    {
      // Read sensor output
      const String strValue = String(myoware.readSensorOutput(outputType));
      if (debugOutput)
        Serial.println(strValue.c_str());

      // "post" to "BLE bulletin board"
      sensorCharacteristic.writeValue(strValue);
    }

    // when the central disconnects, turn off the LED:
    digitalWrite(myoware.getStatusLEDPin(), LOW);

    if (debugLogging)
    {
      Serial.print("Disconnected from central: ");
      Serial.println(central.address());
    }
  }
  else
  {
    myoware.blinkStatusLED();
  }
}
