#ifndef PPMPARAMS_H
#define PPMPARAMS_H
#include <Arduino.h>

// defines pwm endpoints
// numbers are x 10 nanoseconds (e-8 s) - max 2.55 microseconds
struct pwmChParamsStruct {

  uint8_t lowMicros = 100;
  uint8_t midMicros = 150;
  uint8_t highMicros= 200;
};
// class Classy
// {
//     private:
//         int arraysize;
//         std::unique_ptr<int[]> myarray;

//     public:
//         Classy(int parraysize);
//         void printarray();
// };

class ppmParamsClass{

  public:

    // The xmit pin
    const uint8_t castPin;

    // The number of channels to send in the PPM signal
    const uint8_t channelCount;

    const uint16_t frameLength; // in microseconds

    const uint16_t pulseLowLength; // in microseconds

  private:

    pwmChParamsStruct* pwmChParams;

  public:

    ppmParamsClass(uint8_t castPin = 5, 
                  uint8_t channelCount = 8, 
                  uint16_t frameLength = 22000, 
                  uint16_t pulseLowLength = 400);
    ~ppmParamsClass();

    bool setPwmChParam(uint8_t channel, pwmChParamsStruct pwmChParams);
};

#endif //PPMPARAMS_H
