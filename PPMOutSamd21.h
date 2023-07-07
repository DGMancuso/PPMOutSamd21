#ifndef PPMOUTSAMD21_H
#define PPMOUTSAMD21_H
#include <PpmParams.h>

typedef void (*timerCallback)();
static timerCallback TC3Callback;

class PPMOut {
    
    public:

    timerCallback triggerFunc;

    private:

    ppmParamsClass ppmParams;

    uint8_t clockType = 0; // 0 for TC4
                           // 1 for ...

    float clockPeriodRatio = 1.0; // 1000 microsec / (period in nanos) 
    
    long prev;

    bool highState = false; // true false

    bool looping = false;    // flag for looping test mode
    uint8_t loopCount = 0; // counter for loop triggers to extend timer rollover

    unsigned long lastLoopTime; // time of last increment of testing loop
    unsigned long loopTimeDelay = 500; // delay for each test loop (.5 s)

    uint16_t ppmVal[8]; // array of pwm legths for each channel (in microsec)

    uint8_t ppm_ch = 0; // a counter for which channel is getting sent next

    uint8_t ppm_sum = 0;

    public:

    PPMOut(const ppmParamsClass ppmParams,
          void(*triggerFunction)(), 
          uint8_t clockType = 0,
          bool testMode = false);
    ~PPMOut();

    // An interrupt service routine for handling the interrupts activated by PPM pulses
    void interruptServiceTC4();
    void interruptServiceTC4(uint16_t[]);

    // allows interaction with ppm array, main input to change ppm signals
    bool setChannelValue(byte channelRequest, uint16_t value);

    // returns value on channelRequest
    // error returns UINT16_MAX
    uint16_t getChannelValue(byte channelRequest);

    private:

    // Possibility of multiple constructors
    void Init(); 

    // Sets up the clock and interrupt
    // Returns value is one clock period in nanoseconds
    uint16_t clockSetupTC4(uint8_t divisor48MHz = 3); 

    // Preps everything needed to start a new frame
    bool resetFrame();
    
    // Changes values for testing
    void loopValues();

};

#endif //PPMOUTSAMD21_H
