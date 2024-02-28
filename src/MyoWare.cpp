/*
  MyoWare Class
  Advancer Technologies, LLC
  Brian Kaminski
  2/25/2024
  
  MIT license, all text here must be included in any redistribution.  
*/

#include "MyoWare.h"
#include "Arduino.h"

double MyoWare::getENVGain() const 
{
  return m_rawGain * (1. + m_gainPotentiometer);
}

double MyoWare::getADC2Voltage() const
{	
  return m_adcVoltage / (pow(2., m_adcBits) - 1.);
}

double MyoWare::readSensorOutput(const OutputType type) const 
{
  switch (type) 
  {
    case OutputType::ENVELOPE:
      {
        // read the sensor's analog pins
        const int envValue = analogRead(m_envPin);
        if (!m_convertOutput)
          return envValue;

        // convert the analog reading to a voltage (0 - 3.3V):
        const double envVolts = envValue * getADC2Voltage();

        // remove the sensor's gain and convert to millivolts
        return envVolts / getENVGain() * 1000.;
      }

    case OutputType::RAW:
      {
        // read the sensor's analog pins
        const int rawValue = analogRead(m_rawPin);
        const int refValue = analogRead(m_refPin);
        if (!m_convertOutput)
          return rawValue - refValue;

        // convert the analog reading to a voltage (0 - 3.3V):
        const double rawVolts = (rawValue - refValue) * getADC2Voltage();

        // remove the sensor's gain and convert to millivolts
        return rawVolts / m_rawGain * 1000.;
      }

    case OutputType::RECTIFIED:
      {
        // read the sensor's analog pins
        const int rectValue = analogRead(m_rectPin);
        if (!m_convertOutput)
          return rectValue;

        // convert the analog reading to a voltage (0 - 3.3V):
        const double rectVolts = rectValue * getADC2Voltage();

        // remove the sensor's gain and convert to millivolts
        return rectVolts / m_rawGain * 1000.;
      }

    default:
      return -9999.;
  }
  
  return -9999.;  
}

void MyoWare::blinkStatusLED(const int duration) 
{
  if (millis() - m_ledMillis > duration)  // duration in ms
  {
    if (m_toggleLED)
      digitalWrite(m_statusLEDPin, HIGH);
    else
      digitalWrite(m_statusLEDPin, LOW);
    m_ledMillis = millis();
    m_toggleLED = !m_toggleLED;
  }
}
