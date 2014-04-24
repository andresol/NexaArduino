
// T Hight T Low = 0
// T Hight 5T Low = 1
// T Hight 10T Low = SYNC
// T Hight 40T Low = PAUSE
// A packet is in total 65 bits.
// Every other bit is the is redundant data, just the inverted value of the former bit.
// ’1′ -> 10
// ’0′ -> 01

// A packet starts with a Sync bit, and is resent four times with a Paus bit in between.
// Every packet is ended with a zero bit.

// SHHHH HHHH HHHH HHHH HHHH HHHH HHGO EE BB DDDD 0 P

// S = Sync bit.
// H = The first 26 bits are transmitter unique codes, and it is this code that the reciever “learns” to recognize.
// G = Group code, set to one for the whole group.
// O = On/Off bit. Set to 1 for on, 0 for off.
// E = Unit to be turned on or off. The code is inverted, i.e. ’11′ equals 1, ’00′ equals 4.
// B = Button code. The code is inverted, i.e. ’11′ equals 1, ’00′ equals 4.
// D = Dim level bits.
// 0 = packet always ends with a zero.
// P = Pause, a 10 ms pause in between re-send.

// SHHHH HHHH HHHH HHHH HHHH HHHH HHGO EE BB DDDD 0 P
// GO was replaced by ’0000′ (four zeroes in a row) if using Dim.

#define NEXA_T 250
//#define DEBUG 0
#define SLACK 50
#define PACKET_SIZE 64
#define RX_PIN 2// Input of 433 MHz receiver

//BITS
#define ZERO 0
#define ONE 1
#define SYNC 2
#define PAUSE 3
#define ERR 4

//DATA
#define UNIQUE 52
#define GROUP 53
#define ONOFF 55


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
#if defined(DEBUG)
     if (t > 100) {
       Serial.print("Cannot convert ");
       Serial.print(t);
       Serial.println("to bit");
     }
#endif
     return ERR;
   }
}

void setup() {
    pinMode(RX_PIN, INPUT); // Input of 433 MHz

    Serial.begin(9600);
    Serial.println("Starting...");
}


void loop() {
    int i = 0;
    unsigned long t = 0;

    byte prevBit = 0;
    byte bit = 0;

    unsigned long unique = 0;
    bool group = false;
    bool on = false;
    unsigned int unit = 0;

    while (getBitType(t) != PAUSE) {
        t = pulseIn(RX_PIN, LOW, 1000000);
    }

    while (getBitType(t) != SYNC) {
        t = pulseIn(RX_PIN, LOW, 1000000);
    }

    // data collection from reciever circuit
    while (i < PACKET_SIZE) {
        t = pulseIn(RX_PIN, LOW, 1000000);

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
                unique <<= 1;
                unique |= prevBit;
            } else if (i == GROUP) { 
                group = prevBit;
            } else if (i == ONOFF) { 
                on = prevBit;
            } else { 
                unit <<= 1;
                unit |= prevBit;
            }
        }
        prevBit = bit;
        ++i;
    }
     t = pulseIn(RX_PIN, LOW, 1000000);   
    //Print message
    if (i > 0 && getBitType(t) == PAUSE) {
        printResult(unique , group, on, unit); 
        delay(100);
    }
}


void printResult(unsigned long unique , bool group, bool on, unsigned int unit) {
   
    Serial.print("Unique key ");
    Serial.println(unique);

    if (group) {
        Serial.println("All commands");
    } else {
      Serial.print("Unit ");
      Serial.println(unit);
    }
    
    if (on) {
        Serial.println("On");
    } else {
        Serial.println("Off");
    }

  Serial.println();

}
