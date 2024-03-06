/*
  MyoWare BLE Central Example Code
  Advancer Technologies, LLC
  Brian Kaminski
  8/01/2023

  This example sets up a BLE Central device, Then, it connects
  to up to four MyoWare 2.0 Wireless Shields that are reading the ENV and RAW
  outputs of a MyoWare Muscle sensor. It then streams the selected output data
  (ENVELOPE or RAW) from all sensors on the Serial Terminal.

  Note, in BLE, you have services, characteristics and values.
  Read more about it here:
  https://www.arduino.cc/reference/en/libraries/arduinoble/

  Note, before it begins checking the data and printing it,
  It first sets up some BLE stuff:
    1. sets up as a central
    2. scans for and connects to any MyoWare 2.0 Wireless Shields for 10 seconds

  In order for this example to work, you will need a MyoWare 2.0 Wireless Shield,
  and it will need to be programmed with the MyoWare BLEnPeripheral code,
  and advertizing with the unique and correlating characteristic UUID.

  Note, both the service and the characteristic need unique UUIDs and each
  MyoWare 2.0 Wireless Shield needs a unique name (e.g. MyoWareSensor1, MyoWareSensor2)

  This "BLE Central", will read each MyoWare 2.0 Wireless Sensor,
  aka the "BLE Peripheral", charactieristic, parse it for the ENV and RAW values,
  and print them to the serial terminal.

  Hardware:
  BLE device (e.g. ESP32) 
  USB from BLE device to Computer.

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
#include <vector>

// debug parameters
const bool debugLogging = false; // set to true for verbose logging to serial

std::vector<BLEDevice> vecMyoWareShields;

// MyoWare class object
MyoWare myoware;

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  pinMode(myoware.getStatusLEDPin(), OUTPUT); // initialize the built-in LED pin to indicate 
                                              // when a central is connected

  // begin initialization
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!");

    while (1);
  }

  if (debugLogging)
  {
    Serial.println("MyoWare BLE Central");
    Serial.println("-------------------");
  }

  // start scanning for MyoWare Wireless Shields
  if (debugLogging)
  {
    Serial.print("Scanning for MyoWare Wireless Shields: ");
    Serial.println(MyoWareBLE::uuidMyoWareService.c_str());
  }

  BLE.scanForUuid(MyoWareBLE::uuidMyoWareService.c_str(), true);

  // scan for Wireless Shields for 10sec
  const long startMillis = millis();
  while (millis() - startMillis < 10000) 
  {
    myoware.blinkStatusLED();

    BLEDevice peripheral = BLE.available();
    if (peripheral && std::find(vecMyoWareShields.begin(), vecMyoWareShields.end(), peripheral) == vecMyoWareShields.end())
    {
      if (debugLogging)
      {
        Serial.print("Connecting to ");
        PrintPeripheralInfo(peripheral);
      }

      // connect to the peripheral
      BLE.stopScan();
      if (peripheral.connect())
      {
        if (!peripheral.discoverAttributes())
        {
          Serial.println("Discovering Attributes... Failed!");
          if (!peripheral.discoverAttributes())
          {
            Serial.println("Discovering Attributes... Failed!");
            Serial.print("Disconnecting... ");
            PrintPeripheralInfo(peripheral);
            peripheral.disconnect();
            Serial.println("Disconnected");
            continue;
          }
        }
        vecMyoWareShields.push_back(peripheral);
      }
      else
      {
        Serial.print("Failed to connect: ");        
        PrintPeripheralInfo(peripheral);
      }
      BLE.scanForUuid(MyoWareBLE::uuidMyoWareService.c_str(), true);
    }
  }
  BLE.stopScan();

  if (vecMyoWareShields.empty())
  {
    Serial.println("No MyoWare Wireless Shields found!");
    while (1);
  }  
    
  digitalWrite(myoware.getStatusLEDPin(), HIGH); // turn on the LED to indicate a 
                                                 // connection

  for (auto shield : vecMyoWareShields)
  {
    auto ritr = vecMyoWareShields.rbegin();
    if (ritr != vecMyoWareShields.rend() && shield != (*ritr))
    {
      Serial.print(shield.localName());
      Serial.print("\t");
    }
    else
    {
      Serial.println(shield.localName());
    }
  }
}

void loop()
{  
  for (auto shield : vecMyoWareShields)
  {
    if (!shield)
    {
      Serial.print("Invalid MyoWare Wireless Shields pointer! MAC Address: ");
      Serial.println(shield);
      auto itr = std::find(vecMyoWareShields.begin(), vecMyoWareShields.end(), shield);
      if (itr != vecMyoWareShields.end())
        vecMyoWareShields.erase(itr);
      continue;
    }

    if (debugLogging)
    {
      Serial.print("Updating ");
      PrintPeripheralInfo(shield);
    }

    if (!shield.connected())
    {
      // output zero if the Wireless shield gets disconnected
      // this ensures data capture can continue for the 
      // other shields that are connected
      Serial.print("0.0"); 
      Serial.print("\t"); 
      continue;
    }

    BLEService myoWareService = shield.service(MyoWareBLE::uuidMyoWareService.c_str());
    if (!myoWareService)
    {
      Serial.println("Failed finding MyoWare BLE Service!");
      shield.disconnect();
      continue;
    }
    
    // get sensor data
    BLECharacteristic sensorCharacteristic = myoWareService.characteristic(MyoWareBLE::uuidMyoWareCharacteristic.c_str());

    const double sensorValue = ReadBLEData(sensorCharacteristic);
    Serial.print(sensorValue);

    if (vecMyoWareShields.size() > 1)
      Serial.print(","); 
  }
  Serial.println("");
}

// Read the sensor values from the characteristic
double ReadBLEData(BLECharacteristic& dataCharacteristic)
{
  if (dataCharacteristic)
  {
    if (dataCharacteristic.canRead())
    {
      // read the characteristic value as string
      char characteristicValue[20];
      dataCharacteristic.readValue( &characteristicValue,20);
      const String characteristicString(characteristicValue); 

      return characteristicString.toDouble();
    }
    else
    {
      if (debugLogging)
      {
        Serial.print("Unable to read characteristic: ");
        Serial.println(dataCharacteristic.uuid());
      }
      return 0.0;
    }
  }
  else
  {
    if (debugLogging)
      Serial.println("Characteristic not found!");
  }
  return 0.0;
}

void PrintPeripheralInfo(BLEDevice peripheral)
{
  Serial.print(peripheral.address());
  Serial.print(" '");
  Serial.print(peripheral.localName());
  Serial.print("' ");
  Serial.println(peripheral.advertisedServiceUuid());
}