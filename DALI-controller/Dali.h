#ifndef dali_h
#define dali_h

/*
Timer 2 in the ATMega328 and Timer 1 in a ATtiny85 is used to find the time between
each transition coming from the demodulation circuit.
Their setup is for sampling the input in regular intervals.
For practical reasons we use power of 2 timer prescaller for sampling, 
for best timing we use pulse lenght as integer multiple of sampling speed.
We chose to sample every 8 ticks, and pulse lenght of 48 ticks 
thats 6 samples per pulse, lower sampling rate (3) will not work well for 
innacurate clocks (like internal oscilator) higher sampling rate (12) will
cause too much overhead and will not work at higher transmission speeds.
This gives us 16000000Hz/48/256 = 1302 pulses per second (so it's not really 1200) 
At different transmission speeds or on different microcontroller frequencies, clock prescaller is adjusted 
to be compatible with those values. We allow about 50% clock speed difference both ways
allowing us to transmit even with up to 100% in clock speed difference
*/

// DALI coomands
#define BROADCAST_DP  0xFE
#define BROADCAST_C   0xFF
#define ON_C          0x05
#define OFF_C         0x00
#define QUERY_STATUS  0x90
#define GETMAX_C      0xA1
#define RESET         0x20

//setup timing for transmitter
#define HALF_BIT_INTERVAL 1666 

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include <pins_arduino.h>
#endif
#include "P.h"


class Dali
{
  public:
	Dali(); //the constructor
    void setTxPin(uint8_t pin); //set the arduino digital pin for transmit. 
    void setRxAnalogPin(uint8_t pin); //set the arduino digital pin for receive.
    void workAround1MhzTinyCore(uint8_t a = 1); //apply workaround for defect in tiny Core library for 1Mhz
    void setupTransmit(uint8_t pin); //set up transmission
	void setupAnalogReceive(uint8_t pin);
    void transmit(uint8_t cmd1, uint8_t cmd2); //transmit 16 bits of data
	void scanShortAdd(); //scan for short address
	void busInit(int addr); // bus test
	void initialisation(); //initialization of new luminaries
    void randomise();
    void searchaddr(long longaddr);
    void assignaddr();
	uint8_t receive(); //get response
    bool readNumberString(const char *s, int len, int &test);

	int minResponseLevel(); 
	int maxResponseLevel();
    
    uint8_t speedFactor;
    uint16_t delay1;
    uint16_t delay2;
	uint16_t period;

	bool msgMode; //0 - get only response from dali bus to COM; 1 - response with text (comments)
	bool getResponse;
	uint8_t RxAnalogPin;

	unsigned long daliTimeout = 40000; //us, DALI response timeout
	
  private:
	
	void sendByte(uint8_t b); //transmit 8 bits of data
	void sendBit(int b); //transmit 1 bit of data
	void sendZero(void); //transmit "0"
    void sendOne(void); //transmit "1"
   	void splitAdd(long input, uint8_t &highbyte, uint8_t &middlebyte, uint8_t &lowbyte); //split random address 

	uint8_t TxPin;
	
    uint8_t applyWorkAround1Mhz;
	uint8_t rxAnalogPin = 0;
	int analogLevelHi = 999; //analog border level (less - "0"; more - "1")
	int analogLevelLo = 750; //analog border level (less - "0"; more - "1")

};//end of class Dali

extern Dali dali;

#endif
// vim: set sw=2 sts=2 et:
