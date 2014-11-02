#include <Nexa.h>

#define SIGNAL_IN 2

void setup() {
    connectInput(SIGNAL_IN);
    pinMode(3, OUTPUT);
    digitalWrite(3,HIGH);
    Serial.begin(9600);
    Serial.println("Starting...");
}


void loop() {
  NexaMessage message = getMessage(SIGNAL_IN);
  if (message.lastBit == PAUSE && message.unique > 0) {
        printResult(message.unique , message.group, message.on, message.unit); 
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
