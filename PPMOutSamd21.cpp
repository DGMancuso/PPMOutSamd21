#include "PPMOutSamd21.h"

PPMOut::PPMOut(const ppmParamsClass ppmParams, void(*triggerFunction)(), bool testMode):
    ppmParams(ppmParams),
    looping(testMode),
    ppmVal({0,0,0,0,0,0,0,0}),
    triggerFunc(triggerFunction)
{
  Init();
}

void PPMOut::Init()
{
    pinMode(ppmParams.castPin, OUTPUT);
    for(uint16_t i = 0; i < 8 /*size(ppmVal)*/; i++) {ppmVal[i] = 1500;} // initializes each ppm channel to midpoint
    // chooses correct clock setup - only one implemented
    clockPeriodRatio = 1000.0 / clockSetupTC4();
}

float PPMOut::clockSetupTC4(uint8_t divisor48MHz)
{
/*In this example the timer overflows 1000 timer per second, (48MHz/3= 16MHz, 16MHz/64=256kHz, 256kHz/(255 + 1)=1kHz).

The PER (period) register only sets the timer's TOP value in 8-bit mode. In 16-bit mode the PER register has no effect,
therefore in this mode you lose the ability to choose the top counting point. However, you can still change the timer's 
clock frequency, by either altering the generic clock divisor (GCLK_GENDIV_DIV(x)), or the using the timer's own prescaler.*/

/*The example I provide uses generic clock 4 (GCLK4), this clock (unlike GCLK 0 to 3) is unused by the Arduino core code, 
so you can set its source to whatever you require, (in this case 48MHz) and divide it down further using the generic clock 
divisor, (in this case 48MHz/3=16MHz). It just gives you a little bit of extra control over your timer frequency. You can 
then use the timer prescaler to divide down the GCLK further.

Also, in the interrupt handler I've checked both the interrupt flag and the interrupt enable:
  // Check for match counter 0 (MC0) interrupt
  if (TC4->COUNT8.INTFLAG.bit.MC0 && TC4->COUNT8.INTENSET.bit.MC0)
If you don't do this and just use the interrupt flag only, the overflow (OVF), counter compare 0 (CC0), and counter compare 
(CC1) routines can interfere with each other.*/
  
  float returnTracker = 0;
   // Set up the generic clock (GCLK4) used to clock timers
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(divisor48MHz) | // Divide the 48MHz clock source by divisor (default = 3) -> 48MHz/3=16MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  returnTracker = 48000000.0/divisor48MHz;
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Feed GCLK4 to TC4 and TC5
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_TC4_CTRLA |= TC_CTRLA_MODE_COUNT16;          // Set the counter to 8-bit mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  // This is the number clock counter compares to - arbitrary just so it isnt 0 at start
  // Each interrupt will be incrementing this number for its next interrupt
  REG_TC4_COUNT16_CC0 = 0x55;                      // Set the TC4 CC0 register to some arbitary value
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  REG_TC4_COUNT16_CC1 = 0xAA;                      // Set the TC4 CC1 register to some arbitary value
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  // set to Count16 mode as cannot use period register - sets a max for rollover
    // REG_TC4_COUNT8_PER = 0xFF;                      // Set the PER (period) register to its maximum value
    // while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  //NVIC_DisableIRQ(TC4_IRQn);
  //NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  /* OVF is overflow interrupt in case something needs done at clock counter rollover.
     MC0 and MC1 are Match Counters. They are flags to interrupt when the clock counter reaches that value. 
     All interrupts trigger same Handler, here TC4_Handler. Use correct flag for WHAT to do.*/
  
  REG_TC4_INTFLAG |= TC_INTFLAG_MC1 | TC_INTFLAG_MC0 | TC_INTFLAG_OVF;        // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_MC1 /*| TC_INTENSET_MC0 | TC_INTENSET_OVF*/;     // Enable TC4 interrupts - ignoring rollover for everything
  if(looping) REG_TC4_INTENSET |= TC_INTENSET_MC0;                               // Adds testing mode flag to iterate array
  // REG_TC4_INTENCLR = TC_INTENCLR_MC1 | TC_INTENCLR_MC0 | TC_INTENCLR_OVF;     // Disable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_ENABLE |            // Enable TC4
                   TC_CTRLA_PRESCALER_DIV16;    // Set prescaler to 16 - default -> 16MHz/16 = 1MHz
  returnTracker = returnTracker/16;             // defaults: period   = 1000 nanosec <- 1/Hz
  returnTracker = 1000000000/returnTracker;      //           rollover = 65.5 millisec <- 65536 * period (16 bit ticks)              
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);     // Wait for synchronization
  return returnTracker;
}

PPMOut::~PPMOut() {
}

// uses object's stored values to xmit one by one
void PPMOut::interruptServiceTC4() {     

  // Check for overflow (OVF) interrupt
  if (TC4->COUNT8.INTFLAG.bit.OVF && TC4->COUNT8.INTENSET.bit.OVF)             
  {
    // Resets frame - should never get here
    resetFrame();
    REG_TC4_INTFLAG |= TC_INTFLAG_OVF;         // Clear the OVF interrupt flag
  }

  // Check for match counter 0 (MC0) interrupt
  // When triggered, it changes the ppmVals by PPMOut::loopValues()
  if (TC4->COUNT8.INTFLAG.bit.MC0 && TC4->COUNT8.INTENSET.bit.MC0)             
  {
    Serial.println("AM I TRIGGERING?");
    loopCount++;
      if(loopCount >= 100) // every 100 triggers (5 sec)
      {
        loopCount = 0;
        loopValues();
      }
    REG_TC4_COUNT16_CC0 += (50000 * clockPeriodRatio); // next trigger 50ms later
    REG_TC4_INTFLAG |= TC_INTFLAG_MC0;         // Clear the MC0 interrupt flag
  }

  // Check for match counter 1 (MC1) interrupt
  if (TC4->COUNT8.INTFLAG.bit.MC1 && TC4->COUNT8.INTENSET.bit.MC1)           
  {
    //Serial.println(REG_TC4_COUNT16_CC1);
    if (lowState == false) // trigger means begin new pulse
    {
      digitalWrite(ppmParams.castPin,LOW); // pin high
      lowState = true;                   // track pin high
      REG_TC4_COUNT16_CC1 += (ppmParams.pulseLowLength * clockPeriodRatio); // sets next trigger to reverts signal after pulse length 
      ppm_sum += ppmParams.pulseLowLength; // overall time at next trigger - adds one pulseLowLength
    }
    else // if lowState = true
    {
      if (ppm_ch >= ppmParams.channelCount - 1) // resets state after last channel
      {
        resetFrame();
      }
      else // there are more channels to send
      {
        digitalWrite(ppmParams.castPin,HIGH); // pin low
        lowState = false;                     // track pin low
        REG_TC4_COUNT16_CC1=REG_TC4_COUNT16_CC1 += ((ppmVal[ppm_ch] - ppmParams.pulseLowLength) * clockPeriodRatio); // sets next trigger at channel value - note pulseLowLength subtracted back out
        ppm_sum += (ppmVal[ppm_ch] - ppmParams.pulseLowLength); // overall time at next trigger - signal compensating for pulse length
        ppm_ch++; // sets next channel

 
           
    // loopCount++;
    //   if(loopCount >= 50) // every triggers 
    //   {
    //     loopCount = 0;
    //     loopValues();
    //    }

      }
    }
    REG_TC4_INTFLAG |= TC_INTFLAG_MC1;        // Clear the MC1 interrupt flag
  } //end MC1 Interrupt
}

bool PPMOut::resetFrame(){
    Serial.println(ppm_sum); // debugging, can remove
    Serial.print("__________");
    digitalWrite(ppmParams.castPin,HIGH); // pin low
    lowState = false;                     // track pin low
    ppm_ch = 0; //next pulse will start frame
    if (ppm_sum > ppmParams.frameLength) ppm_sum = 0; // if value is somehow out of range, limits downtime
    uint16_t remain = ppmParams.frameLength - ppm_sum; // calculates remaining time left in current frame
    if (remain < 500) remain = 500; // if time is too short, extend frame to min 500 microsec
    Serial.println(remain); // debugging, can remove
    Serial.print("__________");
    REG_TC4_COUNT16_CC1 += (remain * clockPeriodRatio);
    ppm_sum = 0; // reset pulse timer
    REG_TC4_INTFLAG |= TC_INTFLAG_MC1;        // Clear the MC1 interrupt flag
    Serial.print(micros());
    Serial.print("__________");
    for(uint16_t i = 0; i < 8 /*size(ppmVal)*/; i++) {Serial.print(ppmVal[i]);Serial.print("_____");}
    Serial.print(clockPeriodRatio);
    Serial.println("Reset Frame Complete");
  return true;
}

void PPMOut::loopValues() {
    //gradually builds
    ppmVal[0]+=100;
    ppmVal[1]+=100;
    ppmVal[2]+=100;
    ppmVal[3]+=100;
    ppmVal[4]+=100;
    //resets to low-end
    if (ppmVal[2]>1950){
        ppmVal[0]=1000;
        ppmVal[1]=1000;
        ppmVal[2]=1000;
        ppmVal[3]=1000;
        ppmVal[4]=1000;
    }
    Serial.println("Loop increment run.");

}

bool PPMOut::setChannelValue(byte channelRequest, uint16_t value){
    if (channelRequest > ppmParams.channelCount) return false; // can't set a channel that doesnt exist
    if (value > 2000 || value < 1000) return false; // dont do anything if value is weird - FIXME change to variables later
    ppmVal[channelRequest] = value;
    return true;
}

uint16_t PPMOut::getChannelValue(byte channelRequest){
    if (channelRequest > ppmParams.channelCount) return UINT16_MAX; // can't set a channel that doesnt exist
    return ppmVal[channelRequest]; // the obvious answer
    return UINT16_MAX; // shouldn't get here but in case function changes
}
