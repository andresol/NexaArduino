
// T Hight T Low = 0
// T Hight 5T Low = 1
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
#define DEBUG 0
#define SLACK 25
#define PACKET_SIZE 65

// Defining the 4 types of bit format nexa uses.
#define NEXA_T_SYNC_LOW (NEXA_T * 10) - (SLACK * 10)
#define NEXA_T_SYNC_HIGH (NEXA_T * 10) + (SLACK * 10)

#define NEXA_T_PAUSE_LOW (NEXA_T * 40) - (SLACK * 40)
#define NEXA_T_PAUSE_HIGH (NEXA_T * 40) + (SLACK * 40)

#define NEXA_T_1_LOW (NEXA_T * 5) + (SLACK * 5)
#define NEXA_T_1_HIGH (NEXA_T * 5) - (SLACK * 5)

#define NEXA_T_0_LOW NEXA_T - SLACK
#define NEXA_T_0_HIGH NEXA_T + SLACK

// Defining bit size and start
#define UNIQUE_CODE_SIZE 26
#define UNIQUE_CODE_START 0

#define GROUP_CODE_SIZE 1
#define GROUP_CODE_START (UNIQUE_CODE_SIZE * 2) + 1 //53

#define ONOFF_CODE_SIZE 1
#define ONOFF_CODE_START GROUP_CODE_START + (GROUP_CODE_SIZE * 2) //55

#define UNIT_CODE_SIZE 2
#define UNIT_CODE_START ONOFF_CODE_START + (ONOFF_CODE_SIZE * 2) //57

#define BUTTON_CODE_SIZE 2
#define BUTTON_CODE_START UNIT_CODE_START + (UNIT_CODE_SIZE * 2) //61

#define DIM_LEVEL_CODE_SIZE 4
#define DIM_LEVEL_CODE_START BUTTON_CODE_START + (BUTTON_CODE_SIZE * 2) //65

#define PACKET_SIZE DIM_LEVEL_CODE_START + (DIM_LEVEL_CODE_SIZE * 2)

const int rxPin = 2; // Input of 433 MHz receiver


void setup() {
    pinMode(rxPin, INPUT); //Input of 433 MHz receiver
    Serial.begin(9600);
    Serial.println("Nexa Receiver");
}

void detectSyncBit() {
  unsigned long t1 = 0; // Latch 1 time.
  unsigned long t2 = 0; //  Latch 2 time.
  // Find the first Sync bit.
  while (true) {
    t1 = pulseIn(rxPin, LOW, 1000000); // SYNC
    t2 = pulseIn(rxPin, LOW, 1000000); // PAUSE
    
    if (t1 > NEXA_T_SYNC_LOW && t1 < NEXA_T_SYNC_HIGH && t2 > NEXA_T_PAUSE_LOW && t2 < NEXA_T_PAUSE_HIGH) {
      #if defined(DEBUG)
        Serial.print("Start bit detected.");
        Serial.print("t1:" + t1);
        Serial.println("t2:" + t2);
      #endif
      break;
    }
  }
}


void loop() {
    int i = 0;
    
    unsigned long t = 0;

    byte prevBit = 0;
    byte bit = 0;

    unsigned long sender = 0;
    bool group = false;
    bool on = false;
    unsigned int unit = 0;
    unsigned int button = 0;
    unsigned int dim = 0;
    unsigned int rest;

    detectSyncBit();

    // data collection from reciever circuit
    while (i < PACKET_SIZE) {
        t = pulseIn(rxPin, LOW, 1000000);

        if (t > NEXA_T_0_LOW && t < NEXA_T_0_HIGH) {
            bit = 0;
        } else if (t > NEXA_T_1_LOW && t < NEXA_T_1_HIGH) {
            bit = 1;
        } else {
          #if defined(DEBUG)
            Serial.println("Not a 0 or 1 bit. Don't care. Just quit. Pulse is: " + t);
          #endif
          i = 0;
          break;
        }

        if (i % 2 == 1) {
            if ((prevBit ^ bit) == 0) { //Check for redundant XOR = 1 ok.
                if ( i >= GROUP_CODE_START && i < UNIT_CODE_START) { // 53-56 is 0 if using dim.
                   rest <<= 1;
                   rest |= prevBit;
                } else {
                  #if defined(DEBUG)
                    Serial.println("Redundant check failed. ");
                  #endif
                  i = 0;
                  break;
                }
            }

            if (i < GROUP_CODE_START) { // Get the unique code.
              sender <<= 1;
              sender |= prevBit;
            } else if (i == GROUP_CODE_START) { // Group code
              group = prevBit;
            } else if (i == ONOFF_CODE_START) { // On/Off bit
              on = prevBit;
            } else if (i >= UNIT_CODE_START && i < BUTTON_CODE_START){ // Unit code
              unit <<= 1;
              unit |= prevBit;
            } else if (i >= BUTTON_CODE_START && i < DIM_LEVEL_CODE_SIZE) { // Button code
              button <<= 1;
              button |= prevBit;
            } else if (i >= DIM_LEVEL_CODE_START && rest == 0) { // Rest. Dim if bit 53-56 is 0
              #if defined(DEBUG)
                Serial.println("Using dim.");
              #endif
              dim <<= 1;
              dim |= prevBit;
            } else if (i >= DIM_LEVEL_CODE_START && rest != 0) { 
              break; //Not dim.
            } else {
              //rest <<= 1;
              //rest |= prevBit;
            }
        }

        prevBit = bit;
        ++i;
    }
    // Unit and button is inverted
    unit = ~unit;
    button = ~button;   
    
    // interpret message
    if (i > 0) {
        printResult(sender, group, on, unit, button, dim); // Print the result on Serial Monitor. Use this to identify your transmitter code.
    }

}

void printResult(unsigned long sender, bool group, bool on, unsigned int unit, unsigned int button, unsigned int dim) {
    Serial.print("sender ");
    Serial.println(sender);

    if (group) {
        Serial.println("group command");
    } else {
        Serial.println("no group");
    }

    if (on) {
        Serial.println("on");
    } else {
        Serial.println("off");
    }

    Serial.print("Unit: ");
    Serial.println(unit);
    Serial.print("Button: ");
    Serial.println(button);
    Serial.print("Dim: ");
    Serial.println(dim);
    
    Serial.println();


}
