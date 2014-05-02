#ifndef HX711_h
#define HX711_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define NEXA_T 250 //Protocol length
#define SLACK 50
#define PACKET_SIZE 64
#define WAIT_TIME 100
#define TIMEOUT 100

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

typedef struct {
	unsigned long unique;
    int group;
    int on;
    unsigned int unit;
	unsigned int lastBit;
} NexaMessage;

#ifdef __cplusplus
extern "C"{
#endif
	int getBitType(unsigned long t);
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
extern "C"{
#endif
	int connectInput(const unsigned int pin);
#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
extern "C"{
#endif
	NexaMessage getMessage(const unsigned int pin);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* Nexa_h */