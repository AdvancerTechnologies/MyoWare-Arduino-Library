/*
  MyoWare Class
  Advancer Technologies, LLC
  Brian Kaminski
  2/25/2024
  
  MIT license, all text here must be included in any redistribution.  
*/

#ifndef MyoWare_h
#define MyoWare_h
#include "Arduino.h"

namespace MyoWareBLE 
{
  // BLE UART service / characteristics parameters
  const String uuidUARTService = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  const String uuidUARTRXCharacteristic = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
  const String uuidUARTTXCharacteristic = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

  // MyoWare service / characteristics parameters
  const String uuidMyoWareService = "EC3AF789-2154-49F4-A9FC-BC6C88E9E930";
  const String uuidMyoWareCharacteristic = "F3A56EDF-8F1E-4533-93BF-5601B2E91308";
}

class MyoWare 
{
public:
  MyoWare() = default;
  ~MyoWare() = default;

  enum OutputType { ENVELOPE = 0,  // EMG envelope (ENV pin)
                    RAW,           // Amplified and filtered EMG (RAW pin)
                    RECTIFIED };   // Full-wave rectified, amplified, and filtered EMG (RECT pin)
  
  // \brief  Reads MyoWare Sensor output values and returns the desired output |
  // NOTE: The RAW output will have the reference voltage offset removed
  //
  // \param[in]  type:  OutputType value of the desired output type
  // \return  double value of the desired output | Note: will be converted to millivolts if 
  //          setConvertOutput is set to true
  double readSensorOutput(const OutputType type = OutputType::ENVELOPE) const;
  
  // \brief  Blinks the status LED on/off
  // \param[in]  duration:  Integer value of the duration of the on/off cycle in milliseconds |
  //                        Note: Default is 1000 ms
  // \return  none
  void blinkStatusLED(const int duration = 1000);
  
  // \brief  Sets the flag to convert ADC output to the amplitude of the muscle
  // activity as it appears at the electrodes in millivolts
  // \param[in]  flag:  Boolean value of the flag to have output converted to millivolts
  //                    as it appears at the electrodes in millivolts
  // \return none
  void setConvertOutput(const bool flag) { m_convertOutput = flag; }

  // \brief Defines the Arduino pin that is connected to the MyoWare Sensor ENV pin
  // \param[in]  pin:  Integer value of the Arduino pin that is connected to the MyoWare Sensor ENV pin
  // \return  none
  void setENVPin(const int pin) { m_envPin = pin; }
  
  // \brief Gets the Arduino pin that is connected to the MyoWare Sensor ENV pin
  // \param  none
  // \return  Integer value of the Arduino pin that is connected to the MyoWare Sensor ENV pin
  int getENVPin() const { return m_envPin; }

  // \brief Defines the Arduino pin that is connected to the MyoWare Sensor RAW pin
  // \param[in]  pin:  Integer value of the Arduino pin that is connected to the MyoWare Sensor RAW pin
  // \return  none
  void setRAWPin(const int pin) { m_rawPin = pin; }
  
  // \brief Gets the Arduino pin that is connected to the MyoWare Sensor RAW pin
  // \param  none
  // \return  Integer value of the Arduino pin that is connected to the MyoWare Sensor RAW pin
  int getRAWPin() const { return m_rawPin; }

  // \brief Defines the Arduino pin that is connected to the MyoWare Sensor REF pin
  // \param[in]  pin:  Integer value of the Arduino pin that is connected to the MyoWare Sensor REF pin
  // \return  none
  void setREFPin(const int pin) { m_refPin = pin; }
  
  // \brief Gets the Arduino pin that is connected to the MyoWare Sensor REF pin
  // \param  none
  // \return  Integer value of the Arduino pin that is connected to the MyoWare Sensor REF pin
  int getREFPin() const { return m_refPin; }

  // \brief Defines the Arduino pin that is connected to the MyoWare Sensor RECT pin |
  // NOTE: Currently not connected by default by the Wireless Shield
  // \param[in]  pin:  Integer value of the Arduino pin that is connected to the MyoWare Sensor RECT pin
  // \return  none
  void setRECTPin(const int pin) { m_rectPin = pin; }
  
  // \brief Gets the Arduino pin that is connected to the MyoWare Sensor RECT pin  
  // \param  none
  // \return  Integer value of the Arduino pin that is connected to the MyoWare Sensor RECT pin
  int getRECTPin() const { return m_rectPin; }

  // \brief Defines the Arduino pin that is connected to an LED used as status indicator
  // \param[in]  pin:  Integer value of the Arduino status indicator pin
  // \return  none
  void setStatusLEDPin(const int pin) { m_statusLEDPin = pin; }
  
  // \brief Gets the Arduino pin that is connected to an LED used as status indicator
  // \param  none
  // \return  Integer value of the Arduino status indicator pin
  int getStatusLEDPin() const { return m_statusLEDPin; }

  // \brief Define the Arduino ADC resolution in number of bits (Wireless Shield is 12-bit)
  // \param[in]  bits:  double value of the Arduino ADC resolution in number of bits
  // \return  none
  void setADCResolution(const double bits) { m_adcBits = bits; }
  
  // \brief Gets the Arduino ADC resolution value in number of bits
  // \param  none
  // \return  double value of Arduino ADC resolution in bits
  double getADCResolution() const { return m_adcBits; }

  // \brief Define the Arduino ADC reference voltage (Wireless Shield is 3.3V)
  // \param[in]  bits:  double value of the Arduino ADC reference voltage in volts
  // \return  none
  void setADCVoltage(const double voltage) { m_adcVoltage = voltage; }
  
  // \brief Gets the Arduino ADC voltage value
  // \param  none
  // \return  double value of Arduino ADC voltage in volts
  double getADCVoltage() const { return m_adcVoltage; }

  // \brief Define the MyoWare Sensor gain potentiomenter resistance in kOhms |
  // NOTE: The MyoWare Sensor gain potentiometer resistance should be adjusted such that the
  // max muscle reading is below the ADC Voltage then update this
  // \param[in]  resistance:  double value of the MyoWare Sensor gain potentiomenter resistance in kOhms
  // \return  none
  void setGainPotentiometer(const double resistance) { m_gainPotentiometer = resistance; }
  
  // \brief Gets the MyoWare Sensor gain potentiomenter resistance
  // \param  none
  // \return  double value of the MyoWare Sensor gain potentiomenter resistance in kOhms
  double getGainPotentiometer() const { return m_gainPotentiometer; }

  // \brief Gets the calculated gain of the ENV output | 
  // NOTE: ENV gain depends on the gain potentiometer resistance
  // \param  none
  // \return  double value of the calculated gain of the ENV output
  double getENVGain() const;
  
  // \brief Gets the calculated value to convert ADC to volts | 
  // NOTE: Depends on the ADC Voltage and ADC Bits
  // \param  none
  // \return  double value to convert ADC to volts
  double getADC2Voltage() const;

private:

  int m_envPin = 0;   // Arduino pin connected to ENV (Set to A3 for Wireless Shield)
  int m_rawPin = 0;   // Arduino pin connected to RAW (Set to A4 for Wireless Shield)
  int m_refPin = 0;   // Arduino pin connected to REF (Set to A5 for Wireless Shield)
  int m_rectPin = 0;  // Arduino pin connected to RECT (not connected for Wireless Shield)

  bool m_convertOutput = false;      // Flag to enable output conversion to millivolts
  
#ifdef LED_BUILTIN
  int m_statusLEDPin = LED_BUILTIN;
#else
  int m_statusLEDPin = 13;     		 // status LED pin (Wireless Shield uses pin 13)
#endif

  long m_ledMillis = 0;
  bool m_toggleLED = true;

  double m_adcBits = 12.;            // ADC bits (Wireless Shield is 12-bit)
  double m_adcVoltage = 3.3;         // ADC reference voltage (Wireless Shield is 3.3V)
  const double m_rawGain = 200.;     // RAW output has a set gain of 200
  double m_gainPotentiometer = 50.;  // gain potentiometer resistance in kOhms.
                                     // adjust the potentiometer setting such that the
                                     // max muscle reading is below 3.3V then update this
                                     // parameter to the measured value of the potentiometer
};

#endif
