/*###########################################################################
        pq_Dali.h
 
        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
        see http://blog.perquin.com for latest, bugs and info
----------------------------------------------------------------------------
Changelog:
2014-02-07 Created & tested on ATMega328 @ 16Mhz
###########################################################################*/
#include <inttypes.h>

class Dali {
public:

  typedef void (*EventHandlerReceivedDataFuncPtr)(Dali *sender, uint8_t *data, uint8_t len);
  EventHandlerReceivedDataFuncPtr EventHandlerReceivedData;

  void begin(int8_t tx_pin, int8_t rx_pin);
  uint8_t send(uint8_t* tx_msg, uint8_t tx_len_bytes);
  uint8_t sendwait(uint8_t* tx_msg, uint8_t tx_len_bytes, uint32_t timeout_ms=500);
  uint8_t sendwait_int(uint16_t tx_msg, uint32_t timeout_ms=500);
  uint8_t sendwait_byte(uint8_t tx_msg, uint32_t timeout_ms=500);
  void ISR_timer();
  void ISR_pinchange();

  #define DALI_HOOK_COUNT 3


private:

  enum tx_stateEnum { IDLE=0,START,START_X,BIT,BIT_X,STOP1,STOP1_X,STOP2,STOP2_X,STOP3};
  uint8_t tx_pin; //transmitter pin
  uint8_t tx_msg[3]; //message to transmit
  uint8_t tx_len; //number of bits to transmit
  volatile uint8_t tx_pos; //current bit transmit position
  volatile tx_stateEnum tx_state; //current state
  volatile uint8_t tx_collision; //collistion occured
  volatile uint8_t tx_bus_low; //bus is low according to transmitter?

  enum rx_stateEnum { RX_IDLE,RX_START,RX_BIT};
  uint8_t rx_pin; //receiver pin
  volatile uint8_t rx_last_bus_low; //receiver as low at last pinchange
  volatile uint32_t rx_last_change_ts; //timestamp last pinchange
  volatile rx_stateEnum rx_state; //current state
  volatile uint8_t rx_msg[3]; //message received
  volatile int8_t rx_len; //number of half bits received
  volatile uint8_t rx_last_halfbit; //last halfbit received

  volatile uint8_t bus_idle_te_cnt; //number of Te since start of idle bus

  void push_halfbit(uint8_t bit);
}; 


/*  
SIGNAL CHARACTERISTICS
High Level: 9.5 to 22.5 V (Typical 16 V)
Low Level: -6.5 to + 6.5 V (Typical 0 V)
Te = half cycle = 416.67 �s +/- 10 %
10 �s <= tfall <= 100 �s 
10 �s <= trise <= 100 �s

BIT TIMING
msb send first
 logical 1 = 1Te Low 1Te High
 logical 0 = 1Te High 1Te Low
 Start bit = logical 1
 Stop bit = 2Te High

FRAME TIMING
FF: TX Forward Frame 2 bytes (38Te) = 2*(1start+16bits+2stop)
BF: RX Backward Frame 1 byte (22Te) = 2*(1start+8bits+2stop)
no reply: FF >22Te pause FF
with reply: FF >7Te <22Te pause BF >22Te pause FF


DALI commands
=============
In accordance with the DIN EN 60929 standard, addresses and commands are transmitted as numbers with a length of two bytes.

These commands take the form YAAA AAAS xxXXxx. Each letter here stands for one bit.

Y: type of address
     0bin:    short address
     1bin:    group address or collective call

A: significant address bit

S: selection bit (specifies the significance of the following eight bits):
     0bin:    the 8 xxXXxx bits contain a value for direct control of the lamp power
     1bin:    the 8 xxXXxx bits contain a command number.

x: a bit in the lamp power or in the command number


Type of Addresses
=================
Type of Addresses Byte Description
Short address 0AAAAAAS (AAAAAA = 0 to 63, S = 0/1)
Group address 100AAAAS (AAAA = 0 to 15, S = 0/1)
Broadcast address 1111111S (S = 0/1)
Special command 101CCCC1 (CCCC = command number)


Direct DALI commands for lamp power
===================================
These commands take the form YAAA AAA0 xxXXxx.

xxXXxx: the value representing the lamp power is transmitted in these 8 bits. It is calculated according to this formula: 

Pvalue = 10 ^ ((value-1) / (253/3)) * Pmax / 1000

253 values from 1dec to 254dec are available for transmission in accordance with this formula.

There are also 2 direct DALI commands with special meanings:

Command; Command No; Description; Answer
00hex; 0dec; The DALI device dims using the current fade time down to the parameterised MIN value, and then switches off.; - 
FFhex; 254dec; Mask (no change): this value is ignored in what follows, and is therefore not loaded into memory.; - 


Indirect DALI commands for lamp power
=====================================
These commands take the form YAAA AAA1 xxXXxx.

xxXXxx: These 8 bits transfer the command number. The available command numbers are listed and explained in the following tables in hexadecimal and decimal formats.

Command; Command No; Description; Answer
00hex 0dez Extinguish the lamp (without fading) - 
01hex 1dez Dim up 200 ms using the selected fade rate - 
02hex 2dez Dim down 200 ms using the selected fade rate - 
03hex 3dez Set the actual arc power level one step higher without fading. If the lamp is off, it will be not ignited. - 
04hex 4dez Set the actual arc power level one step lower without fading. If the lamp has already it's minimum value, it is not switched off. - 
05hex 5dez Set the actual arc power level to the maximum value. If the lamp is off, it will be ignited. - 
06hex 6dez Set the actual arc power level to the minimum value. If the lamp is off, it will be ignited. - 
07hex 7dez Set the actual arc power level one step lower without fading. If the lamp has already it's minimum value, it is switched off. - 
08hex 8dez Set the actual arc power level one step higher without fading. If the lamp is off, it will be ignited. - 
09hex ... 0Fhex 9dez ... 15dez reserved - 
1nhex
(n: 0hex ... Fhex) 16dez ... 31dez Set the light level to the value stored for the selected scene (n) - 


Configuration commands
======================
Command; Command No; Description; Answer
20hex 32dez Reset the parameters to default settings - 
21hex 33dez Store the current light level in the DTR (Data Transfer Register) - 
22hex ... 29hex 34dez ... 41dez reserved - 
2Ahex 42dez Store the value in the DTR as the maximum level - 
2Bhex 43dez Store the value in the DTR as the minimum level - 
2Chex 44dez Store the value in the DTR as the system failure level - 
2Dhex 45dez Store the value in the DTR as the power on level - 
2Ehex 46dez Store the value in the DTR as the fade time - 
2Fhex 47dez Store the value in the DTR as the fade rate - 
30hex ... 3Fhex 48dez ... 63dez reserved - 
4nhex
(n: 0hex ... Fhex) 64dez ... 79dez Store the value in the DTR as the selected scene (n) - 
5nhex
(n: 0hex ... Fhex) 80dez ... 95dez Remove the selected scene (n) from the DALI slave - 
6nhex
(n: 0hex ... Fhex) 96dez ... 111dez Add the DALI slave unit to the selected group (n) - 
7nhex
(n: 0hex ... Fhex) 112dez ... 127dez Remove the DALI slave unit from the selected group (n) - 
80hex 128dez Store the value in the DTR as a short address - 
81hex ... 8Fhex 129dez ... 143dez reserved - 
90hex 144dez Returns the status (XX) of the DALI slave XX 
91hex 145dez Check if the DALI slave is working yes/no 
92hex 146dez Check if there is a lamp failure yes/no 
93hex 147dez Check if the lamp is operating yes/no 
94hex 148dez Check if the slave has received a level out of limit yes/no 
95hex 149dez Check if the DALI slave is in reset state yes/no 
96hex 150dez Check if the DALI slave is missing a short address XX 
97hex 151dez Returns the version number as XX 
98hex 152dez Returns the content of the DTR as XX 
99hex 153dez Returns the device type as XX 
9Ahex 154dez Returns the physical minimum level as XX 
9Bhex 155dez Check if the DALI slave is in power failure mode yes/no 
9Chex ... 9Fhex 156dez ... 159dez reserved - 
A0hex 160dez Returns the current light level as XX 
A1hex 161dez Returns the maximum allowed light level as XX 
A2hex 162dez Returns the minimum allowed light level as XX 
A3hex 163dez Return the power up level as XX 
A4hex 164dez Returns the system failure level as XX 
A5hex 165dez Returns the fade time as X and the fade rate as Y XY 
A6hex ... AFhex 166dez ... 175dez reserved - 
Bnhex
(n: 0hex ... Fhex) 176dez ... 191dez Returns the light level XX for the selected scene (n) XX 
C0hex 192dez Returns a bit pattern XX indicating which group (0-7) the DALI slave belongs to XX 
C1hex 193dez Returns a bit pattern XX indicating which group (8-15) the DALI slave belongs to XX 
C2hex 194dez Returns the high bits of the random address as HH 
C3hex 195dez Return the middle bit of the random address as MM 
C4hex 196dez Returns the lower bits of the random address as LL 
C5hex ... DFhex 197dez ... 223dez reserved - 
E0hex ... FFhex 224dez ... 255dez Returns application specific extension commands   


Note Repeat of DALI commands 
============================
According to IEC 60929, a DALI Master has to repeat several commands within 100 ms, so that DALI-Slaves will execute them. 

The DALI Master Terminal KL6811 repeats the commands 32dez to 128dez, 258dez and 259dez (bold marked) automatically to make the the double call from the user program unnecessary.

The DALI Master Terminal KL6811 repeats also the commands 224dez to 255dez, if you have activated this with Bit 1 of the Control-Byte (CB.1) before.


DALI Control Device Type List
=============================
Type DEC Type HEX Name Comments
128 0x80 Unknown Device. If one of the devices below don�t apply
129 0x81 Switch Device A Wall-Switch based Controller including, but not limited to ON/OFF devices, Scene switches, dimming device.
130 0x82 Slide Dimmer An analog/positional dimming controller 
131 0x83 Motion/Occupancy Sensor. A device that indicates the presence of people within a control area.
132 0x84 Open-loop daylight Controller. A device that outputs current light level and/or sends control messages to actuators based on light passing a threshold.
133 0x85 Closed-loop daylight controller. A device that outputs current light level and/or sends control messages to actuators based on a change in light level.
134 0x86 Scheduler. A device that establishes the building mode based on time of day, or which provides control outputs.
135 0x87 Gateway. An interface to other control systems or communication busses
136 0x88 Sequencer. A device which sequences lights based on a triggering event
137 0x89 Power Supply *). A DALI Power Supply device which supplies power for the communication loop
138 0x8a Emergency Lighting Controller. A device, which is certified for use in control of emergency lighting, or, if not certified, for noncritical backup lighting.
139 0x8b Analog input unit. A general device with analog input.
140 0x8c Data Logger. A unit logging data (can be digital or analog data)


Flash Variables and Offset in Information
=========================================
Memory Name Offset
Power On Level [0]
System Failure Level [1]
Minimum Level [2]
Maximum Level [3]
Fade Rate [4]
Fade Time [5]
Short Address [6]
Group 0 through 7 [7]
Group 8 through 15 [8]
Scene 0 through 15 [9-24]
Random Address [25-27]
Fast Fade Time [28]
Failure Status [29]
Operating Mode [30]
Dimming Curve [31]
*/
