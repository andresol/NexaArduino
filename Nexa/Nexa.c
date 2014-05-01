#include <Arduino.h>
#include <Nexa.h>

int getBitType(unsigned long t) {
	if (t > (NEXA_T - (SLACK * 2)) && t < (NEXA_T + (SLACK * 3))) {
     return ZERO;
   } else if (t > (NEXA_T * 3) && t < (NEXA_T * 7)) {
     return ONE;
   } else if (t > (NEXA_T * 30) && (t < NEXA_T * 50)) {
     return PAUSE;
   } else if (t > (NEXA_T * 7) && (t < NEXA_T * 13)) {
     return SYNC;
   } else {
     return ERR;
   }
}

int connectInput(const unsigned int pin) {
	 pinMode(pin, INPUT); 
}

NexaMessage getMessage(const unsigned int pin) {
	int i = 0;
    unsigned long t = 0;

    byte prevBit = 0;
    byte bit = 0;
	
	NexaMessage message;
	message.unique = 0;
	message.on = 0;
	message.group = 0;
	message.unit = 0;
	message.lastBit = -1;

    while (getBitType(t) != PAUSE) {
        t = pulseIn(pin, LOW, 1000000);
    }

    while (getBitType(t) != SYNC) {
        t = pulseIn(pin, LOW, 1000000);
    }

    while (i < PACKET_SIZE) {
        t = pulseIn(pin, LOW, 1000000);

        if (getBitType(t) == ZERO) {
            bit = ZERO;
        } else if (getBitType(t) == ONE) {
            bit = ONE;
        } else {
            i = 0;
            break;
        }

        if (i % 2 == 1) {
            if ((prevBit + bit) != 1) { // Error check. Must Be 01 or 10
                i = 0;
                break;
            }

            if (i <= UNIQUE) { 
                message.unique <<= 1;
                message.unique |= prevBit;
            } else if (i == GROUP) { 
                message.group = prevBit;
            } else if (i == ONOFF) { 
                message.on = prevBit;
            } else { 
                message.unit <<= 1;
                message.unit |= prevBit;
            }
        }
        prevBit = bit;
        ++i;
    }
    t = pulseIn(pin, LOW, 1000000);   
	message.lastBit = getBitType(t);
	return message;
}