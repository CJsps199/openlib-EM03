/*
  Emon.cpp - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
*/

// Proboscide99 10/08/2016 - Added ADMUX settings for ATmega1284 e 1284P (644 / 644P also, but not tested) in readVcc function

//#include "WProgram.h" un-comment for use on older versions of Arduino IDE
#include "EmonLibCJ.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


//--------------------------------------------------------------------------------------
// Sets the pins to be used for voltage and current sensors
//--------------------------------------------------------------------------------------
void EnergyMonitor3ph::voltage_l1(unsigned int _inPinV_l1, double _VCAL_l1, double _PHASECAL_l1)
{
  inPinV_l1 = _inPinV_l1;
  VCAL_l1 = _VCAL_l1;
  PHASECAL_l1 = _PHASECAL_l1;
  offsetV_l1 = ADC_COUNTS>>1;
}

void EnergyMonitor3ph::voltage_l2(unsigned int _inPinV_l2, double _VCAL_l2, double _PHASECAL_l2)
{
  inPinV_l2 = _inPinV_l2;
  VCAL_l2 = _VCAL_l2;
  PHASECAL_l2 = _PHASECAL_l2;
  offsetV_l2 = ADC_COUNTS>>1;
}

void EnergyMonitor3ph::voltage_l3(unsigned int _inPinV_l3, double _VCAL_l3, double _PHASECAL_l3)
{
  inPinV_l3 = _inPinV_l3;
  VCAL_l3 = _VCAL_l3;
  PHASECAL_l3 = _PHASECAL_l3;
  offsetV_l3 = ADC_COUNTS>>1;
}

void EnergyMonitor3ph::current_l1(unsigned int _inPinI_l1, double _ICAL_l1)
{
  inPinI_l1 = _inPinI_l1;
  ICAL_l1 = _ICAL_l1;
  offsetI_l1 = ADC_COUNTS>>1;
}

void EnergyMonitor3ph::current_l2(unsigned int _inPinI_l2, double _ICAL_l2)
{
  inPinI_l2 = _inPinI_l2;
  ICAL_l2 = _ICAL_l2;
  offsetI_l2 = ADC_COUNTS>>1;
}

void EnergyMonitor3ph::current_l3(unsigned int _inPinI_l3, double _ICAL_l3)
{
  inPinI_l3 = _inPinI_l3;
  ICAL_l3 = _ICAL_l3;
  offsetI_l3 = ADC_COUNTS>>1;
}

//--------------------------------------------------------------------------------------
// Sets the pins to be used for voltage and current sensors based on emontx pin map
//--------------------------------------------------------------------------------------
void EnergyMonitor3ph::voltageTX_l1(double _VCAL_l1, double _PHASECAL_l1)
{
  inPinV_l1 = 2;
  VCAL_l1 = _VCAL_l1;
  PHASECAL_l1 = _PHASECAL_l1;
  offsetV_l1 = ADC_COUNTS>>1;
}


void EnergyMonitor3ph::currentTX_l1(unsigned int _channel_l1, double _ICAL_l1)
{
  if (_channel_l1 == 1) inPinI_l1 = 3;
  if (_channel_l1 == 2) inPinI_l1 = 0;
  if (_channel_l1 == 3) inPinI_l1 = 1;
  ICAL_l1 = _ICAL_l1;
  offsetI_l1 = ADC_COUNTS>>1;
}

//--------------------------------------------------------------------------------------
// emon_calc procedure
// Calculates realPower,apparentPower,powerFactor,Vrms,Irms,kWh increment
// From a sample window of the mains AC voltage and current.
// The Sample window length is defined by the number of half wavelengths or crossings we choose to measure.
//--------------------------------------------------------------------------------------
void EnergyMonitor3ph::calcVI_l1(unsigned int crossings_l1, unsigned int timeout_l1)
{
  
  int SupplyVoltage=3300;
  

  unsigned int crossCount_l1 = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples_l1 = 0;                        //This is now incremented

  //-------------------------------------------------------------------------------------------------------------------------
  // 1) Waits for the waveform to be close to 'zero' (mid-scale adc) part in sin curve.
  //-------------------------------------------------------------------------------------------------------------------------
  boolean st_l1=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st_l1==false)                                   //the while loop...
  {
    startV_l1 = analogRead(inPinV_l1);                    //using the voltage waveform
    if ((startV_l1 < (ADC_COUNTS*0.55)) && (startV_l1 > (ADC_COUNTS*0.45))) st_l1=true;  //check its within range
    if ((millis()-start)>timeout_l1) st_l1 = true;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //-------------------------------------------------------------------------------------------------------------------------
  start = millis();

  while ((crossCount_l1 < crossings_l1) && ((millis()-start)<timeout_l1))
  {
    numberOfSamples_l1++;                       //Count number of times looped.
    lastFilteredV_l1 = filteredV_l1;               //Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    sampleV_l1 = analogRead(inPinV_l1);                 //Read in raw voltage signal
    sampleI_l1 = analogRead(inPinI_l1);                 //Read in raw current signal

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
    offsetV_l1 = offsetV_l1 + ((sampleV_l1-offsetV_l1)/1024);
    filteredV_l1 = sampleV_l1 - offsetV_l1;
    offsetI_l1 = offsetI_l1 + ((sampleI_l1-offsetI_l1)/1024);
    filteredI_l1 = sampleI_l1 - offsetI_l1;

    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------
    sqV_l1= filteredV_l1 * filteredV_l1;                 //1) square voltage values
    sumV_l1 += sqV_l1;                                //2) sum

    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------
    sqI_l1 = filteredI_l1 * filteredI_l1;                //1) square current values
    sumI_l1 += sqI_l1;                                //2) sum

    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV_l1 = lastFilteredV_l1 + PHASECAL_l1 * (filteredV_l1 - lastFilteredV_l1);

    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------
    instP_l1 = phaseShiftedV_l1 * filteredI_l1;          //Instantaneous Power
    sumP_l1 +=instP_l1;                               //Sum

    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------
    lastVCross_l1 = checkVCross_l1;
    if (sampleV_l1 > startV_l1) checkVCross_l1 = true;
                     else checkVCross_l1 = false;
    if (numberOfSamples_l1==1) lastVCross_l1 = checkVCross_l1;

    if (lastVCross_l1 != checkVCross_l1) crossCount_l1++;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied.

  double V_RATIO_l1 = VCAL_l1 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Vrms_l1 = V_RATIO_l1 * sqrt(sumV_l1 / numberOfSamples_l1);

  double I_RATIO_l1 = ICAL_l1 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l1 = I_RATIO_l1 * sqrt(sumI_l1 / numberOfSamples_l1);

  //Calculation power values
  realPower_l1 = V_RATIO_l1 * I_RATIO_l1 * sumP_l1 / numberOfSamples_l1;
  apparentPower_l1 = Vrms_l1 * Irms_l1;
  powerFactor_l1=realPower_l1 / apparentPower_l1;

  //Reset accumulators
  sumV_l1 = 0;
  sumI_l1 = 0;
  sumP_l1 = 0;
//--------------------------------------------------------------------------------------
}

void EnergyMonitor3ph::calcVI_l2(unsigned int crossings_l2, unsigned int timeout_l2)
{
  
  int SupplyVoltage=3300;
  

  unsigned int crossCount_l2 = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples_l2 = 0;                        //This is now incremented

  //-------------------------------------------------------------------------------------------------------------------------
  // 1) Waits for the waveform to be close to 'zero' (mid-scale adc) part in sin curve.
  //-------------------------------------------------------------------------------------------------------------------------
  boolean st_l2=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st_l2==false)                                   //the while loop...
  {
    startV_l2 = analogRead(inPinV_l2);                    //using the voltage waveform
    if ((startV_l2 < (ADC_COUNTS*0.55)) && (startV_l2 > (ADC_COUNTS*0.45))) st_l2=true;  //check its within range
    if ((millis()-start)>timeout_l2) st_l2 = true;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //-------------------------------------------------------------------------------------------------------------------------
  start = millis();

  while ((crossCount_l2 < crossings_l2) && ((millis()-start)<timeout_l2))
  {
    numberOfSamples_l2++;                       //Count number of times looped.
    lastFilteredV_l2 = filteredV_l2;               //Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    sampleV_l2 = analogRead(inPinV_l2);                 //Read in raw voltage signal
    sampleI_l2 = analogRead(inPinI_l2);                 //Read in raw current signal

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
    offsetV_l2 = offsetV_l2 + ((sampleV_l2-offsetV_l2)/1024);
    filteredV_l2 = sampleV_l2 - offsetV_l2;
    offsetI_l2 = offsetI_l2 + ((sampleI_l2-offsetI_l2)/1024);
    filteredI_l2 = sampleI_l2 - offsetI_l2;

    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------
    sqV_l2= filteredV_l2 * filteredV_l2;                 //1) square voltage values
    sumV_l2 += sqV_l2;                                //2) sum

    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------
    sqI_l2 = filteredI_l2 * filteredI_l2;                //1) square current values
    sumI_l2 += sqI_l2;                                //2) sum

    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV_l2 = lastFilteredV_l2 + PHASECAL_l2 * (filteredV_l2 - lastFilteredV_l2);

    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------
    instP_l2 = phaseShiftedV_l2 * filteredI_l2;          //Instantaneous Power
    sumP_l2 +=instP_l2;                               //Sum

    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------
    lastVCross_l2 = checkVCross_l2;
    if (sampleV_l2 > startV_l2) checkVCross_l2 = true;
                     else checkVCross_l2 = false;
    if (numberOfSamples_l2==1) lastVCross_l2 = checkVCross_l2;

    if (lastVCross_l2 != checkVCross_l2) crossCount_l2++;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied.

  double V_RATIO_l2 = VCAL_l2 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Vrms_l2 = V_RATIO_l2 * sqrt(sumV_l2 / numberOfSamples_l2);

  double I_RATIO_l2 = ICAL_l2 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l2 = I_RATIO_l2 * sqrt(sumI_l2 / numberOfSamples_l2);

  //Calculation power values
  realPower_l2 = V_RATIO_l2 * I_RATIO_l2 * sumP_l2 / numberOfSamples_l2;
  apparentPower_l2 = Vrms_l2 * Irms_l2;
  powerFactor_l2 = realPower_l2 / apparentPower_l2;

  //Reset accumulators
  sumV_l2 = 0;
  sumI_l2 = 0;
  sumP_l2 = 0;
//--------------------------------------------------------------------------------------
}

void EnergyMonitor3ph::calcVI_l3(unsigned int crossings_l3, unsigned int timeout_l3)
{
  
  int SupplyVoltage=3300;
  
  

  unsigned int crossCount_l3 = 0;                             //Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples_l3 = 0;                        //This is now incremented

  //-------------------------------------------------------------------------------------------------------------------------
  // 1) Waits for the waveform to be close to 'zero' (mid-scale adc) part in sin curve.
  //-------------------------------------------------------------------------------------------------------------------------
  boolean st_l3=false;                                  //an indicator to exit the while loop

  unsigned long start = millis();    //millis()-start makes sure it doesnt get stuck in the loop if there is an error.

  while(st_l3==false)                                   //the while loop...
  {
    startV_l3 = analogRead(inPinV_l3);                    //using the voltage waveform
    if ((startV_l3 < (ADC_COUNTS*0.55)) && (startV_l3 > (ADC_COUNTS*0.45))) st_l3=true;  //check its within range
    if ((millis()-start)>timeout_l3) st_l3 = true;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 2) Main measurement loop
  //-------------------------------------------------------------------------------------------------------------------------
  start = millis();

  while ((crossCount_l3 < crossings_l3) && ((millis()-start)<timeout_l3))
  {
    numberOfSamples_l3++;                       //Count number of times looped.
    lastFilteredV_l3 = filteredV_l3;               //Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in raw voltage and current samples
    //-----------------------------------------------------------------------------
    sampleV_l3 = analogRead(inPinV_l3);                 //Read in raw voltage signal
    sampleI_l3 = analogRead(inPinI_l3);                 //Read in raw current signal

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
    offsetV_l3 = offsetV_l3 + ((sampleV_l3-offsetV_l3)/1024);
    filteredV_l3 = sampleV_l3 - offsetV_l3;
    offsetI_l3 = offsetI_l3 + ((sampleI_l3-offsetI_l3)/1024);
    filteredI_l3 = sampleI_l3 - offsetI_l3;

    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------
    sqV_l3= filteredV_l3 * filteredV_l3;                 //1) square voltage values
    sumV_l3 += sqV_l3;                                //2) sum

    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------
    sqI_l3 = filteredI_l3 * filteredI_l3;                //1) square current values
    sumI_l3 += sqI_l3;                                //2) sum

    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV_l3 = lastFilteredV_l3 + PHASECAL_l3 * (filteredV_l3 - lastFilteredV_l3);

    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------
    instP_l3 = phaseShiftedV_l3 * filteredI_l3;          //Instantaneous Power
    sumP_l3 +=instP_l3;                               //Sum

    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------
    lastVCross_l3 = checkVCross_l3;
    if (sampleV_l3 > startV_l3) checkVCross_l3 = true;
                     else checkVCross_l3 = false;
    if (numberOfSamples_l3==1) lastVCross_l3 = checkVCross_l3;

    if (lastVCross_l3 != checkVCross_l3) crossCount_l3++;
  }

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  //Calculation of the root of the mean of the voltage and current squared (rms)
  //Calibration coefficients applied.

  double V_RATIO_l3 = VCAL_l3 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Vrms_l3 = V_RATIO_l3 * sqrt(sumV_l3 / numberOfSamples_l3);

  double I_RATIO_l3 = ICAL_l3 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l3 = I_RATIO_l3 * sqrt(sumI_l3 / numberOfSamples_l3);

  //Calculation power values
  realPower_l3 = V_RATIO_l3 * I_RATIO_l3 * sumP_l3 / numberOfSamples_l3;
  apparentPower_l3 = Vrms_l3 * Irms_l3;
  powerFactor_l1=realPower_l3 / apparentPower_l3;

  //Reset accumulators
  sumV_l3 = 0;
  sumI_l3 = 0;
  sumP_l3 = 0;
//--------------------------------------------------------------------------------------
}


//--------------------------------------------------------------------------------------
double EnergyMonitor3ph::calcIrms_l1(unsigned int Number_of_Samples_l1)
{

  
    int SupplyVoltage=3300;
  
  


  for (unsigned int n = 0; n < Number_of_Samples_l1; n++)
  {
    sampleI_l1 = analogRead(inPinI_l1);

    // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
    //  then subtract this - signal is now centered on 0 counts.
    offsetI_l1 = (offsetI_l1 + (sampleI_l1-offsetI_l1)/1024);
    filteredI_l1 = sampleI_l1 - offsetI_l1;

    // Root-mean-square method current
    // 1) square current values
    sqI_l1 = filteredI_l1 * filteredI_l1;
    // 2) sum
    sumI_l1 += sqI_l1;
  }

  double I_RATIO_l1 = ICAL_l1 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l1 = I_RATIO_l1 * sqrt(sumI_l1 / Number_of_Samples_l1);

  //Reset accumulators
  sumI_l1 = 0;
  //--------------------------------------------------------------------------------------

  return Irms_l1;
}

double EnergyMonitor3ph::calcIrms_l2(unsigned int Number_of_Samples_l2)
{

  
    int SupplyVoltage=3300;
 


  for (unsigned int n = 0; n < Number_of_Samples_l2; n++)
  {
    sampleI_l2 = analogRead(inPinI_l2);

    // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
    //  then subtract this - signal is now centered on 0 counts.
    offsetI_l2 = (offsetI_l2 + (sampleI_l2-offsetI_l2)/1024);
    filteredI_l2 = sampleI_l2 - offsetI_l2;

    // Root-mean-square method current
    // 1) square current values
    sqI_l2 = filteredI_l2 * filteredI_l2;
    // 2) sum
    sumI_l2 += sqI_l2;
  }

  double I_RATIO_l2 = ICAL_l2 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l2 = I_RATIO_l2 * sqrt(sumI_l2 / Number_of_Samples_l2);

  //Reset accumulators
  sumI_l2 = 0;
  //--------------------------------------------------------------------------------------

  return Irms_l2;
}

double EnergyMonitor3ph::calcIrms_l3(unsigned int Number_of_Samples_l3)
{


    int SupplyVoltage=3300;
 


  for (unsigned int n = 0; n < Number_of_Samples_l3; n++)
  {
    sampleI_l3 = analogRead(inPinI_l3);

    // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
    //  then subtract this - signal is now centered on 0 counts.
    offsetI_l3 = (offsetI_l3 + (sampleI_l3-offsetI_l3)/1024);
    filteredI_l3 = sampleI_l3 - offsetI_l3;

    // Root-mean-square method current
    // 1) square current values
    sqI_l3 = filteredI_l3 * filteredI_l3;
    // 2) sum
    sumI_l3 += sqI_l3;
  }

  double I_RATIO_l3 = ICAL_l3 *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms_l3 = I_RATIO_l3 * sqrt(sumI_l3 / Number_of_Samples_l3);

  //Reset accumulators
  sumI_l3 = 0;
  //--------------------------------------------------------------------------------------

  return Irms_l3;
}

void EnergyMonitor3ph::serialprint_l1()
{

  Serial.print("P: ");  
  Serial.print(realPower_l1);
  Serial.print(' ');
  Serial.print("S: ");
  Serial.print(apparentPower_l1);
  Serial.print(' ');
  Serial.print("Vrms: ");
  Serial.print(Vrms_l1);
  Serial.print(' ');
  Serial.print("Irms: ");
  Serial.print(Irms_l1);
  Serial.print(' ');
  Serial.print("PF: ");
  Serial.print(powerFactor_l1);
  Serial.println(' ');
  delay(100);
}

void EnergyMonitor3ph::serialprint_l2()
{

  Serial.print("P: ");  
  Serial.print(realPower_l2);
  Serial.print(' ');
  Serial.print("S: ");
  Serial.print(apparentPower_l2);
  Serial.print(' ');
  Serial.print("Vrms: ");
  Serial.print(Vrms_l2);
  Serial.print(' ');
  Serial.print("Irms: ");
  Serial.print(Irms_l2);
  Serial.print(' ');
  Serial.print("PF: ");
  Serial.print(powerFactor_l2);
  Serial.println(' ');
  delay(100);
}

void EnergyMonitor3ph::serialprint_l3()
{

  Serial.print("P: ");  
  Serial.print(realPower_l3);
  Serial.print(' ');
  Serial.print("S: ");
  Serial.print(apparentPower_l3);
  Serial.print(' ');
  Serial.print("Vrms: ");
  Serial.print(Vrms_l3);
  Serial.print(' ');
  Serial.print("Irms: ");
  Serial.print(Irms_l3);
  Serial.print(' ');
  Serial.print("PF: ");
  Serial.print(powerFactor_l3);
  Serial.println(' ');
  delay(100);
}



//thanks to http://hacking.majenko.co.uk/making-accurate-adc-readings-on-arduino
//and Jérôme who alerted us to http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/

long EnergyMonitor3ph::readVcc_l1() {
  long result;

  //not used on emonTx V3 - as Vcc is always 3.3V - eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/

  #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_AT90USB1286__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRB &= ~_BV(MUX5);   // Without this the function always returns -1 on the ATmega2560 http://openenergymonitor.org/emon/node/2253#comment-11432
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);

  #endif


  #if defined(__AVR__)
  delay(2);                                        // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);                             // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = READVCC_CALIBRATION_CONST / result;  //1100mV*1024 ADC steps http://openenergymonitor.org/emon/node/1186
  return result;
  #elif defined(__arm__)
  return (3300);                                  //Arduino Due
  #else
  return (3300);                                  //Guess that other un-supported architectures will be running a 3.3V!
  #endif
}

