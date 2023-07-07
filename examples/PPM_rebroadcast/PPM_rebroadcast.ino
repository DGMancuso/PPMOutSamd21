#include <ServoInput.h>
#include <PPMOutSamd21.h>


bool testingMode = true; // changes PPM output to have a testing loop setting values - no PWM in needed

//  pin definitions
  const uint8_t PPMOutputPin = 1;
  //byte PPMInputPin = 2; 

  const uint8_t PPMChannels = 7;
  ServoInputPin<3> PWM1;
  ServoInputPin<4> PWM2;
  ServoInputPin<5> PWM3;
  ServoInputPin<6> PWM4;
  ServoInputPin<7> PWM5;
  ServoInputPin<8> PWM6;
  ServoInputPin<9> PWM7;
  // ServoInputPin<10> PWM8;
  // ServoInputPin<11> PWM9;

// creates array for storing servo input references
ServoInputSignal* servoInputPtr[PPMChannels];

//        static void TimerHandler();

uint16_t PwmInRaw[PPMChannels];

ppmParamsClass ppmParams(PPMOutputPin, PPMChannels); // do this define next FIXME

PPMOut PPMOutObj(ppmParams, TC3_Handler, testingMode);

void setup() {
    // stores servo input references - comment out unneeded
  servoInputPtr[0] = &PWM1;
  servoInputPtr[1] = &PWM2;
  servoInputPtr[2] = &PWM3;
  servoInputPtr[3] = &PWM4;
  servoInputPtr[4] = &PWM5;
  servoInputPtr[5] = &PWM6;
  servoInputPtr[6] = &PWM7;
  // servoInputPtr[7] = &PWM8;
  // servoInputPtr[8] = &PWM9;
  
    // Sets PWM In endpoints if not default - add more for channels as needed
    // Allows extension of range if Xmiter cant reach all
  // ppmParams.setPwmChParam(0, pwmChParamsStruct{100,150,200}); 

    // initialize PwmInRaw at 1500
  for(uint16_t i : PwmInRaw){PwmInRaw[i] = 1500;}
}

void loop() {
  // sends PWM Signals to PPM Xmit trigger
  for(uint16_t i : PwmInRaw) // for each PWM Input
  {
    uint16_t rawValue = servoInputPtr[i]->getPulse(); // gets PWM but waits for processing
      
      // add any processing of raw value here
      switch (i) 
      {
        // case 0: break;
        // case 1: break;
        default:
        PwmInRaw[i] = rawValue;
      }

    PPMOutObj.setChannelValue(i, PwmInRaw[i]);  // sends PWM Signals to PPM Xmit trigger
  }
}

// Interrupt Service Routine for PPM
// Make sure it is named for the interrupt you are using.
// Preferably just point to library function.
void TC4_Handler()
{
  PPMOutObj.interruptServiceTC4(); // made library function so it doesnt need repeated
}
