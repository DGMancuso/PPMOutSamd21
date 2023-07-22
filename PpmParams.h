#ifndef PPMPARAMS_H
#define PPMPARAMS_H
#include <PWMParametersClass.h>

class ppmParamsClass{

  public:

    // The xmit pin
    const uint8_t castPin;

    // The number of channels to send in the PPM signal
    const uint8_t channelCount;

    const uint16_t frameLength; // in microseconds

    const uint16_t pulseLowLength; // in microseconds

  private:

    PWMParametersClass* pwmChParams;

  public:

    ppmParamsClass(uint8_t castPin = 5, 
                  uint8_t channelCount = 8, 
                  uint16_t frameLength = 22000, 
                  uint16_t pulseLowLength = 500);
    ~ppmParamsClass();

    bool setPwmChParam(PWMParametersClass pwmChParams);
};

#endif //PPMPARAMS_H
