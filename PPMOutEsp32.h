#ifndef PPMOUTSAMD21_H
#define PPMOUTSAMD21_H
#include <PpmParams.h>

typedef void (*timerCallback)();

enum ppmState_e
{
    PPM_STATE_IDLE,
    PPM_STATE_PULSE,
    PPM_STATE_FILL,
    PPM_STATE_SYNC
};

class PPMOut {
    
    public:

    timerCallback triggerFunc;

    private:

    ppmParamsClass ppmParams;

    float clockPeriodRatio = 1.0; // 1000 microsec / (period in nanos) 
    
    long prev;

    bool lowState = false; // true false

    bool looping = false;    // flag for looping test mode
    uint8_t loopCount = 0; // counter for loop triggers to extend timer rollover

    unsigned long lastLoopTime; // time of last increment of testing loop
    unsigned long loopTimeDelay = 500; // delay for each test loop (.5 s)

    uint16_t ppmVal[8]; // array of pwm legths for each channel (in microsec)

    uint8_t ppm_ch = 0; // a counter for which channel is getting sent next

    uint16_t ppm_sum = 0;

    int lastMicros = 0;

    uint8_t ppmState = PPM_STATE_IDLE;
    uint8_t ppmChannel = 0;
    uint8_t ppmOutput = LOW;
    int usedFrameLength = 0;
    int currentChannelValue;

    public:

    PPMOut(const ppmParamsClass ppmParams,
          void(*triggerFunction)(), 
          bool testMode = false);
    ~PPMOut();

    // An interrupt service routine for handling the interrupts activated by PPM pulses
    void IRAM_ATTR onPpmTimer();

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
    float clockSetupTC4(uint8_t divisor48MHz = 3); 

    // Preps everything needed to start a new frame
    bool resetFrame();
    
    // Changes values for testing
    void loopValues();

};

#endif //PPMOUTSAMD21_H
