#include "PpmParams.h"

ppmParamsClass::ppmParamsClass(uint8_t castPin, 
                               uint8_t channelCount, 
                               uint16_t frameLength, 
                               uint16_t pulseLowLength):
    castPin(castPin),
    channelCount(channelCount), 
    frameLength(frameLength),
    pulseLowLength(pulseLowLength),
    pwmChParams(new PWMParametersClass[channelCount])
{
}

// overwrites defaults to end point numbers for PWM in
bool ppmParamsClass::setPwmChParam(PWMParametersClass pwmChParamsOverwrite)
{
  if(pwmChParamsOverwrite.pin >= channelCount) return false;
  pwmChParams[pwmChParamsOverwrite.pin] = pwmChParamsOverwrite;
  return true;
}

ppmParamsClass::~ppmParamsClass() {
  delete pwmChParams;
}
