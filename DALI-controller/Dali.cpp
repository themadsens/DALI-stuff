#include "Dali.h"

static const int delaytime = 20; //ms

Dali::Dali() //constructor
{
  applyWorkAround1Mhz = 0;
}

void Dali::setTxPin(uint8_t pin)
{
  TxPin = pin; // user sets the digital pin as output
  pinMode(TxPin, OUTPUT); 
  digitalWrite(TxPin, HIGH);
}

void Dali::setRxAnalogPin(uint8_t pin)
{
  RxAnalogPin = pin; // user sets the digital pin as output
}

void Dali::workAround1MhzTinyCore(uint8_t a)
{
  applyWorkAround1Mhz = a;
}

void Dali::setupAnalogReceive(uint8_t pin) 
{
  setRxAnalogPin(pin); // user sets the analog pin as input
}


void Dali::setupTransmit(uint8_t pin)
{
  setTxPin(pin);
  speedFactor = 2;
  //we don't use exact calculation of passed time spent outside of transmitter
  //because of high ovehead associated with it, instead we use this 
  //emprirically determined values to compensate for the time loss
  
  #if F_CPU == 1000000UL
    uint16_t compensationFactor = 88; //must be divisible by 8 for workaround
  #elif F_CPU == 8000000UL
    uint16_t compensationFactor = 12; 
  #else //16000000Mhz
    uint16_t compensationFactor = 4; 
  #endif  

#if (F_CPU == 80000000UL) || (F_CPU == 160000000)   // ESP8266 80MHz or 160 MHz
  delay1 = delay2 = (HALF_BIT_INTERVAL >> speedFactor) - 2;
#else
  delay1 = (HALF_BIT_INTERVAL >> speedFactor) - compensationFactor;
  delay2 = (HALF_BIT_INTERVAL >> speedFactor) - 2;
  period = delay1 + delay2;
  
  #if F_CPU == 1000000UL
    delay2 -= 22; //22+2 = 24 is divisible by 8
    if (applyWorkAround1Mhz) { //definition of micro delay is broken for 1MHz speed in tiny cores as of now (May 2013)
      //this is a workaround that will allow us to transmit on 1Mhz
      //divide the wait time by 8
      delay1 >>= 3;
      delay2 >>= 3;
    }
  #endif
#endif
}

void Dali::transmit(uint8_t cmd1, uint8_t cmd2) // transmit commands to DALI bus (address byte, command byte)
{
  sendBit(1);
  sendByte(cmd1);
  sendByte(cmd2);
  digitalWrite(TxPin, HIGH);
}

void Dali::sendByte(uint8_t b) 
{
  for (int i = 7; i >= 0; i--) 
  {
    sendBit((b >> i) & 1);
  }
}

void Dali::sendBit(int b) 
{   
 if (b) {
    sendOne();
  }
  else {
    sendZero();
  } 
}

void Dali::sendZero(void)
{
  digitalWrite(TxPin, HIGH);
  delayMicroseconds(delay2);
  digitalWrite(TxPin, LOW);
  delayMicroseconds(delay1);

}

void Dali::sendOne(void)
{
  digitalWrite(TxPin, LOW);
  delayMicroseconds(delay2);
  digitalWrite(TxPin, HIGH);
  delayMicroseconds(delay1);
}

void Dali::busInit() //DALI bus test
{
  int maxLevel;
  int minLevel;
  
  //Luminaries must turn on and turn off. If not, check connection.
  if (dali.msgMode) {
    delay(100);
    dali.transmit(BROADCAST_C, ON_C); //Broadcast OFF
    delay(500);
    dali.transmit(BROADCAST_C, OFF_C); //Broadcast ON
    delay(100);
  }
  
  //Receive response from luminaries: max and min level
  dali.transmit(BROADCAST_C, QUERY_STATUS);
  maxLevel = dali.maxResponseLevel();
  dali.transmit(BROADCAST_C, QUERY_STATUS);
  minLevel = dali.minResponseLevel();

  dali.analogLevel = (int)(maxLevel + minLevel) / 2;

  if (dali.msgMode)
    PN("Bus levels: %d, %d", maxLevel, minLevel);
  else
    Serial.println(dali.analogLevel);
}

void Dali::splitAdd(long input, uint8_t &highbyte, uint8_t &middlebyte, uint8_t &lowbyte) 
{
  highbyte   = (input >> 16) & 0xFF;
  middlebyte = (input >> 8) & 0xFF;
  lowbyte    = (input) & 0xFF;
}

// define min response level
int Dali::minResponseLevel() 
{
  const uint8_t dalistep = 40; //us
  uint16_t rxmin = 1024;
  uint16_t dalidata;
  unsigned long idalistep;

  for (idalistep = 0; idalistep < dali.daliTimeout; idalistep = idalistep + dalistep) {
    dalidata = analogRead(RxAnalogPin);
    if (dalidata < rxmin) {
      rxmin = dalidata;
    };
    delayMicroseconds(dalistep);
  }
  return rxmin; 
}

// define max response level
int Dali::maxResponseLevel() 
{
  const uint8_t dalistep = 40; //us
  uint16_t rxmax = 0;
  uint16_t dalidata;
  unsigned long idalistep;
  
  for (idalistep = 0; idalistep < dali.daliTimeout; idalistep = idalistep + dalistep) {
    dalidata = analogRead(dali.RxAnalogPin);
    if (dalidata > rxmax) {
      rxmax = dalidata;
    };
    delayMicroseconds(dalistep);
  }
  return rxmax;
}

//scan for individual short address
void Dali::scanShortAdd()
{
  const int delayTime = 10;
  uint8_t add_byte;
  uint8_t device_short_add;
  uint8_t response;
    
  if (dali.getResponse)
    dali.transmit(BROADCAST_C, OFF_C); // Broadcast Off
  delay(delayTime);
  
  if (dali.msgMode) {
    PN("Short addresses:");
  }

  for (device_short_add = 0; device_short_add <= 63; device_short_add++) {
    add_byte = 1 + (device_short_add << 1); // convert short address to address byte
    
    dali.transmit(add_byte, GETMAX_C);
    response = dali.receive();
        
    if (dali.getResponse) {
      
      if (dali.msgMode) {
        dali.transmit(add_byte, ON_C); // switch on
        delay(1000);
        dali.transmit(add_byte, OFF_C); // switch off
        delay(1000);
      }
    }
    else {
      response = 0;
    }
    
    if (dali.msgMode) {
      P("ADDR: %02d", device_short_add);
      if (dali.getResponse) {
        PN(" Response: 0x%02x", response);
      }
      else {
        PN(" No response:%d", response);
      }
    }
    else {
      if (dali.getResponse) {
        Serial.print('1');
      }
      else {
        Serial.print('0');
      }
    }
  }
  Serial.println();
  delay(delayTime);
}

int Dali::readNumberString(const char *s, int len, boolean &test) 
{
  int result = 0;

  if (len == 8) {
    while (len-- > 0 && test) {
      if (isdigit(*s) && *s < 2)
        result = result * 2 + (*s++ - '0');
      else
        test = false;
    }
  }
  else if (len == 3) {
    while (len-- > 0 && test) {
      if (isdigit(*s))
        result = result * 10 + (*s++ - '0');
      else
        test = false;
    }
  }
  else if (len == 2) {
    while (len-- > 0 && test) {
      if (isdigit(*s))
        result = result * 16 + (*s++ - '0');
      else if (isxdigit(*s))
        result = result * 16 + (toupper(*s++) - 'A' + 10);
      else
        test = false;
    }
  }
  else
    test = false;
  return result;
}

bool Dali::cmdCheck(const char *input, int & cmd1, int & cmd2) 
{
  char *sep = strchr(input, ',');
  if (!sep)
    return false;

  boolean test = true;
  cmd1 = readNumberString(input, sep - input, test);
  cmd2 = readNumberString(sep+1, strlen(sep+1), test);

  return test;
}

void Dali::initialisation() {
  dali.randomise();
  dali.assignaddr();
}

void Dali::randomise() {
  delay(delaytime);
  dali.transmit(BROADCAST_C, RESET);
  delay(delaytime);
  dali.transmit(BROADCAST_C, RESET);
  delay(delaytime);
  dali.transmit(BROADCAST_C, OFF_C);
  delay(100);
  dali.transmit(0xA5, 0); //initialise
  delay(delaytime);
  dali.transmit(0xA5, 0); //initialise
  delay(delaytime);
  dali.transmit(0xA7, 0); //randomise
  delay(delaytime);
  dali.transmit(0xA7, 0); //randomise
}

void Dali::searchaddr(long longadd) {
  uint8_t highbyte;
  uint8_t middlebyte;
  uint8_t lowbyte;

  dali.splitAdd(longadd, highbyte, middlebyte, lowbyte); //divide 24bit adress into three 8bit adresses
  delay(delaytime);
  dali.transmit(0xB1, highbyte);   // search HB
  delay(delaytime);
  dali.transmit(0xB3, middlebyte); // search MB
  delay(delaytime);
  dali.transmit(0xB5, lowbyte);    // search LB
  delay(delaytime);
  dali.transmit(0xA9, 0);          // compare
  int response = dali.receive();

  if (dali.msgMode) {
    PN("LONG: 0x%06lX - 1 => 0x%02x(%d)", longadd + 1, response, dali.getResponse?1:0);
  }
}

void Dali::assignaddr() {

  long low_longadd = 0x000000;
  long high_longadd = 0xFFFFFF;
  long longadd = (long)(low_longadd + high_longadd) / 2;
  uint8_t short_add = 0;

  if (dali.msgMode) {
    PN("Searching for long addresses:");
    delay(500);
  }

  while (longadd <= 0xFFFFFF - 2 and short_add < 64) {
    while ((high_longadd - low_longadd) > 1) {

      dali.searchaddr(longadd);
      
      if (!dali.getResponse) {
        low_longadd = longadd;
      }
      else {
        high_longadd = longadd;
      }
      
      longadd = (low_longadd + high_longadd) / 2; //center

    } // inner while

    if (high_longadd != 0xFFFFFF) {
      dali.searchaddr(longadd + 1);
      dali.transmit(0xB7, 1 | (short_add << 1)); //program short adress
      delay(delaytime);
      dali.transmit(0xAB, 0);                    //withdraw
      delay(delaytime);
      dali.transmit(1 + (short_add << 1), ON_C);
      delay(1000);
      dali.transmit(1 + (short_add << 1), OFF_C);
      delay(delaytime);

      if (dali.msgMode) { 
        PN("SHORT: %02d", short_add);
      }
      else {
        Serial.println(longadd + 1);
      }
      short_add++;

      high_longadd = 0xFFFFFF;
      longadd = (low_longadd + high_longadd) / 2;

    }
    else {
      if (dali.msgMode) { 
        PN("END"); 
      }
    }
  } // first while


  dali.transmit(0xA1, 0);  //terminate
}

uint8_t Dali::receive() {
  unsigned long startFuncTime = 0;
  bool previousLogicLevel = 1;
  bool currentLogicLevel = 1;
  uint8_t arrLength = 20;
  int  timeArray[arrLength];
  int i = 0;
  int k = 0;
  bool logicLevelArray[arrLength];
  int response = 0;

  dali.getResponse = false;
  startFuncTime = micros();
  
  // add check for micros overlap here!!!

  while (micros() - startFuncTime < dali.daliTimeout && i < arrLength)
  {
    // geting response
    if (analogRead(dali.RxAnalogPin) > dali.analogLevel) {
      currentLogicLevel = 1;
    }
    else {
      currentLogicLevel = 0;
    }

    if (previousLogicLevel != currentLogicLevel) {
      timeArray[i] = micros() - startFuncTime;
      logicLevelArray[i] = currentLogicLevel;
      previousLogicLevel = currentLogicLevel;
      i++;
    }
  }
  arrLength = i;

  if (arrLength < 10) { // 8 data bits + a "1" start bit => >10 shifts
    dali.getResponse = false;
    return arrLength;
  }
  dali.getResponse = true;

  //decoding to manchester
  for (i = 0; i < arrLength - 1; i++) {
    if ((timeArray[i + 1] - timeArray[i]) > 0.75 * dali.period) {
      for (k = arrLength; k > i; k--) {
        timeArray[k] = timeArray[k - 1];
        logicLevelArray[k] = logicLevelArray[k - 1];
      }
      arrLength++;
      timeArray[i + 1] = (timeArray[i] + timeArray[i + 2]) / 2;
      logicLevelArray[i + 1] = logicLevelArray[i];
    }
  }

  k = 8;
  for (i = 1; i < arrLength; i++) {
    if (logicLevelArray[i] == 1) {
      if ((int)round((timeArray[i] - timeArray[0]) / (0.5 * dali.period)) & 1) {
        response = response + (1 << k);
      }
      k--;
    }
  }
  
  //remove start bit
  return response & 0xFF;
}

Dali dali;
// vim: set sw=2 sts=2 et:
