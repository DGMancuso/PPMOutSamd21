#include "PpmParams.h"


// Classy::Classy(int parraysize)
//     : arraysize{parraysize}
//     , myarray{new int[arraysize]}
// {
//     for(int i = 0; i < arraysize; i++){
//         myarray[i] = i * i * 2;
//     }
// }

ppmParamsClass::ppmParamsClass(uint8_t castPin, 
                               uint8_t channelCount, 
                               uint16_t frameLength, 
                               uint16_t pulseLowLength):
    castPin(castPin),
    channelCount(channelCount), 
    frameLength(frameLength),
    pulseLowLength(pulseLowLength),
    pwmChParams(new pwmChParamsStruct[channelCount])
{
}

// overwrites defaults to end point numbers for PWM in
bool ppmParamsClass::setPwmChParam(uint8_t channel, pwmChParamsStruct pwmChParamsOverwrite)
{
  if(channel >= channelCount) return false;
  pwmChParams[channel] = pwmChParamsOverwrite;
  return true;
}

ppmParamsClass::~ppmParamsClass() {
  delete pwmChParams;
}

// void ppmParamsClass::loopValues() {
//     //gradually builds
//     ppmVal[0]+=100;
//     ppmVal[1]+=100;
//     ppmVal[2]+=100;
//     ppmVal[3]+=100;
//     ppmVal[4]+=100;
//     //resets to low-end
//     if (ppmVal[2]>1950){
//         ppmVal[0]=1000;
//         ppmVal[1]=1000;
//         ppmVal[2]=1000;
//         ppmVal[3]=1000;
//         ppmVal[4]=1000;
//     }
// }
