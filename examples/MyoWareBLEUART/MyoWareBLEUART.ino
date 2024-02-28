/*
  MyoWare BLE UART Example Code
  Advancer Technologies, LLC
  Brian Kaminski
  12/16/2023

  This example sets up a MyoWare 2.0 Wireless Shield to connect to the free app 
  Bluefruit Connect (https://learn.adafruit.com/bluefruit-le-connect/) from
  Adafruit Industries. The code reads the ENV, RAW, and REF data from the
  attached MyoWare 2.0 Muscle Sensor and sends the data to the Bluefruit Connect
  app over BLE UART.

  This MyoWare 2.0 Wireless Shield, aka the "BLE Server", will read the sensor's
  output on A3-A5 where A3 is ENV, A4 is RAW, and A5 is REF. It then removes
  the DC offset from the RAW data using the REF value and notifies the
  "BLE Client" which is your smartphone running the Bluefruit Connect app.
  
  Note, in BLE, you have services, characteristics and values.
  Read more about BLE here: 
  https://www.arduino.cc/reference/en/libraries/arduinoble/
  
  This example uses Nordic UART Service (NUS) UUIDs for the service and the characteristics.
  Read more about Nordic UART Service here:
  https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v14.0.0%2Fble_sdk_app_nus_eval.html
 
  Note, before it begins reading the ADC and updating the data,
  It first sets up some BLE stuff:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service
   6. Start advertising

  In order for this example to work, you will need a smartphone, with Bluetooth
  turned on, and the Bluefruit Connect app installed.
  1. Open the Bluefruit Connect app
  2. The device list will display all BLE devices discovered by the app
  3. Find the device named "MyoW1" and connect
  * The device name is defined by variable shieldDeviceName below
  * If using multiple MyoWare IoT Shields, use "Multiple UART mode"
  4. Once connected, you can choose UART or Plotter module to view 
     incoming data as text or graphically  

  Hardware:
  MyoWare 2.0 Wireless Shield (or ESP32-based device)
  MyoWare 2.0 Muscle Sensor
  Smartphone with Bluefruit Connect app installed

  ** For consistent BT connection follow these steps:
  ** 1. Reset Server
  ** 2. Wait 5 seconds
  ** 3. Reset Client
  ** 4. Enjoy BT connection

  This example code is in the public domain.
*/

#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <MyoWare.h>

// user-definable parameters
const String myowareShieldName = "MyoW1"; // device name can be customized
                                          // and is the name that will show 
                                          // up in the Bluefruit Connect app's 
                                          // device list. recommend making this
                                          // unique for each Wireless shield 
                                          // (e.g. MyoW1, MyoW2, ...)

// debug parameters
const bool debugLogging = false;     // set to true for verbose logging
const bool debugOutput = false;      // set to true to print output values to serial

// MyoWare class object
MyoWare myoware;

// misc. parameters
bool isConnected = false;
bool previousConnected = false;

BLEServer* pServer = NULL;
BLECharacteristic* pTXCharacteristic = NULL;
BLECharacteristic* pRXCharacteristic = NULL;

class ServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) 
    {
      isConnected = true;
    }

    void onDisconnect(BLEServer* pServer) 
    {
      isConnected = false;
    }
};

void setup() 
{
  Serial.begin(115200);

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

  pinMode(myoware.getStatusLEDPin(), OUTPUT); // initialize the built-in LED pin to indicate 
                                              // when a central is connected
  digitalWrite(myoware.getStatusLEDPin(), HIGH);

  // Create the BLE Device
  BLEDevice::init(myowareShieldName.c_str());

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  if (!pServer)
  {
    Serial.println("Error creating Server!");
    while(!pServer);
  }
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Service  
  BLEService* pService = pServer->createService(MyoWareBLE::uuidUARTService.c_str());
  if (!pService)
  {
    Serial.println("Error creating UART Service!");
    while(!pService);
  }

  // Create the BLE TX Characteristic
  pTXCharacteristic = 
    pService->createCharacteristic(MyoWareBLE::uuidUARTTXCharacteristic.c_str(), 
                                   BLECharacteristic::PROPERTY_NOTIFY);
  if (!pTXCharacteristic)
  {
    Serial.println("Error creating TX characteristic!");
    while(!pTXCharacteristic);
  }
  pTXCharacteristic->addDescriptor(new BLE2902());

  // Create the BLE RX Characteristic
  pRXCharacteristic = 
    pService->createCharacteristic(MyoWareBLE::uuidUARTRXCharacteristic.c_str(), 
                                   BLECharacteristic::PROPERTY_WRITE);
  if (!pRXCharacteristic)
  {
    Serial.println("Error creating RX characteristic!");
    while(!pRXCharacteristic);
  }

  // Start the service
  pService->start();

  /// Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();  
  pAdvertising->addServiceUUID(MyoWareBLE::uuidUARTService.c_str());
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  if (debugLogging)
  {
    Serial.println("Setup Complete!");
    Serial.println("Waiting to connect...");
  }

  digitalWrite(myoware.getStatusLEDPin(), LOW);
}

void loop() 
{
  // if connected... read the sensor value, update the transmit characteristic
  // value, and notify that the value changed
  if (isConnected) 
  {
    digitalWrite(myoware.getStatusLEDPin(), HIGH); // turn on the LED to indicate the 
                                                   // connection

    // Read sensor output
    const String strValue = ReadSensorData() + "\n";
    if (debugOutput)
      Serial.println(strValue.c_str());

    if (pTXCharacteristic)
    {
      pTXCharacteristic->setValue(strValue.c_str());
      pTXCharacteristic->notify();
    }
  }
  // blink status LED while not connected
  else
  {
    myoware.blinkStatusLED();
  }
  
  // if the shield disconnected, restart advertising and blink the status LED
  if (!isConnected && previousConnected) 
  {
      delay(500);
      
      // restart advertising
      pServer->startAdvertising(); 
      
      // blink status LED while not connected
      if (debugLogging)
        Serial.println("Waiting to reconnect...");
      myoware.blinkStatusLED();
      previousConnected = isConnected;
  }
  // if the shield just connected, update the previousConnected flag
  else if (isConnected && !previousConnected) 
  {
      previousConnected = isConnected;
  }
}

// reads MyoWare Sensor output values and returns them as a comma delimited string
String ReadSensorData() 
{
  const double env = myoware.readSensorOutput(MyoWare::ENVELOPE);
  const double raw = myoware.readSensorOutput(MyoWare::RAW);

  return String(env) + "," + String(raw);
}