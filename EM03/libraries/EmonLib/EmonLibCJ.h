/*
  Emon.h - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
*/

#ifndef EmonLibCJ_h
#define EmonLibCJ_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

// define theoretical vref calibration constant for use in readvcc()
// 1100mV*1024 ADC steps http://openenergymonitor.org/emon/node/1186
// override in your code with value for your specific AVR chip
// determined by procedure described under "Calibrating the internal reference voltage" at
// http://openenergymonitor.org/emon/buildingblocks/calibration
#ifndef READVCC_CALIBRATION_CONST
#define READVCC_CALIBRATION_CONST 1126400L
#endif

// to enable 12-bit ADC resolution on Arduino Due,
// include the following line in main sketch inside setup() function:
//  analogReadResolution(ADC_BITS);
// otherwise will default to 10 bits, as in regular Arduino-based boards.
#if defined(__arm__)
#define ADC_BITS    12
#else
#define ADC_BITS    10
#endif

#define ADC_COUNTS  (1<<ADC_BITS)


class EnergyMonitor3ph
{
  public:

    void voltage_l1(unsigned int _inPinV_l1, double _VCAL_l1, double _PHASECAL_l1);
    void current_l1(unsigned int _inPinI_l1, double _ICAL_l1);

    void voltage_l2(unsigned int _inPinV_l2, double _VCAL_l2, double _PHASECAL_l2);
    void current_l2(unsigned int _inPinI_l2, double _ICAL_l2);

    void voltage_l3(unsigned int _inPinV_l3, double _VCAL_l3, double _PHASECAL_l3);
    void current_l3(unsigned int _inPinI_l3, double _ICAL_l3);

    void voltageTX_l1(double _VCAL_l1, double _PHASECAL_l1);
    void currentTX_l1(unsigned int _channel_l1, double _ICAL__l1);

    void calcVI_l1(unsigned int crossings_l1, unsigned int timeout_l1);
    void calcVI_l2(unsigned int crossings_l2, unsigned int timeout_l2);
    void calcVI_l3(unsigned int crossings_l3, unsigned int timeout_l3);

    double calcIrms_l1(unsigned int NUMBER_OF_SAMPLES_l1);
    double calcIrms_l2(unsigned int NUMBER_OF_SAMPLES_l2);
    double calcIrms_l3(unsigned int NUMBER_OF_SAMPLES_l3);

    void serialprint_l1();
    void serialprint_l2();
    void serialprint_l3();

    long readVcc_l1();
    //Useful value variables
    double realPower_l1,
      apparentPower_l1,
      powerFactor_l1,
      Vrms_l1,
      Irms_l1;
    double realPower_l2,
      apparentPower_l2,
      powerFactor_l2,
      Vrms_l2,
      Irms_l2;
    double realPower_l3,
      apparentPower_l3,
      powerFactor_l3,
      Vrms_l3,
      Irms_l3;

  private:

    //Set Voltage and current input pins
    unsigned int inPinV_l1;
    unsigned int inPinI_l1;

    unsigned int inPinV_l2;
    unsigned int inPinI_l2;

    unsigned int inPinV_l3;
    unsigned int inPinI_l3;
    //Calibration coefficients
    //These need to be set in order to obtain accurate results
    double VCAL_l1;
    double ICAL_l1;
    double PHASECAL_l1;

    double VCAL_l2;
    double ICAL_l2;
    double PHASECAL_l2;

    double VCAL_l3;
    double ICAL_l3;
    double PHASECAL_l3;

    //--------------------------------------------------------------------------------------
    // Variable declaration for emon_calc procedure
    //--------------------------------------------------------------------------------------
    int sampleV_l1;                        //sample_ holds the raw analog read value
    int sampleI_l1;

    int sampleV_l2;                        //sample_ holds the raw analog read value
    int sampleI_l2;

    int sampleV_l3;                        //sample_ holds the raw analog read value
    int sampleI_l3;

    double lastFilteredV_l1,filteredV_l1;          //Filtered_ is the raw analog value minus the DC offset
    double filteredI_l1;
    double offsetV_l1;                          //Low-pass filter output
    double offsetI_l1;

    double lastFilteredV_l2,filteredV_l2;          //Filtered_ is the raw analog value minus the DC offset
    double filteredI_l2;
    double offsetV_l2;                          //Low-pass filter output
    double offsetI_l2;

    double lastFilteredV_l3,filteredV_l3;          //Filtered_ is the raw analog value minus the DC offset
    double filteredI_l3;
    double offsetV_l3;                          //Low-pass filter output
    double offsetI_l3;

                          //Low-pass filter output

    double phaseShiftedV_l1; 
    double phaseShiftedV_l2; 
    double phaseShiftedV_l3;                             //Holds the calibrated phase shifted voltage.

    double sqV_l1,sumV_l1,sqI_l1,sumI_l1,instP_l1,sumP_l1; 
    double sqV_l2,sumV_l2,sqI_l2,sumI_l2,instP_l2,sumP_l2; 
    double sqV_l3,sumV_l3,sqI_l3,sumI_l3,instP_l3,sumP_l3;              //sq = squared, sum = Sum, inst = instantaneous

    int startV_l1;                                       //Instantaneous voltage at start of sample window.

    boolean lastVCross_l1, checkVCross_l1;  

    int startV_l2;                                       //Instantaneous voltage at start of sample window.

    boolean lastVCross_l2, checkVCross_l2;

    int startV_l3;                                       //Instantaneous voltage at start of sample window.

    boolean lastVCross_l3, checkVCross_l3;                //Used to measure number of times threshold is crossed.


};

#endif
