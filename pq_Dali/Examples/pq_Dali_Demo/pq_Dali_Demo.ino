/*###########################################################################
DALI Interface Demo

On the Arduino connect pin18 and pin19 together

Dali interface1 transmit on pin18, interface2 receives on pin19

----------------------------------------------------------------------------
Changelog:
2014-02-07 Created & tested on ATMega328 @ 16Mhz
----------------------------------------------------------------------------
		pq_Dali_Demo.ino
		
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
###########################################################################*/

#include "pq_Dali.h"

//create the DALI interfaces
Dali dali1; 
Dali dali2; 

//callback to handle received data on dali2 interface
void dali2_receiver(Dali *d, uint8_t *data, uint8_t len) {
  Serial.print("RX");
  if(len>=2) Serial.println((int)(data[0]<<8) + data[1]); else  Serial.println((int)data[0]);
}

void setup() {
  Serial.begin(115200);
  Serial.println("DALI Master/Slave Demo");
  
  //start the DALI interfaces
  //arguments: tx_pin, rx_pin (negative values disable transmitter / receiver)
  //Note: the receiver pins should be on different PCINT groups, e.g one 
  //receiver on pin0-7 (PCINT2) one on pin8-13 (PCINT0) and one on pin14-19 (PCINT1)
  dali1.begin(18,-3); 
  dali2.begin(-2,19);
  
  //attach a received data handler
  dali2.EventHandlerReceivedData = dali2_receiver;
}

uint16_t i=0;

void loop() {
  Serial.print("tx");
  Serial.println(i);
  dali1.sendwait_int(i);
  //delay(200);
  i+=1;
}

