// This does not run on MBED for arduino nano connect rp2040
#include "UltraSonic.h"
#include "LEDRing.h"

// D11 (7) == trigger, D12 (4) == echo USE GPIO# NOT D#
// Using pio0 and sm 0
UltraSonic sonar(20, 21, pio0, 0);
// Specify the PIO and state machine to use
LEDRing ledRing(4, pio0, 1); // Using pio0 and state machine 1

// Union for float to uint32_t conversion
union FloatUnion {
    float f;
    uint32_t i;
};

void setup() {
  Serial.begin(115200);
  rp2040.enableDoubleResetBootloader();
}

void loop() {
  // Ultrasonic from core1
  uint32_t receivedInt;
  if (rp2040.fifo.pop_nb(&receivedInt)) {
    // Convert uint32_t back to float
    FloatUnion dataReceived;
    dataReceived.i = receivedInt;
    float receivedFloatValue = dataReceived.f;
    // Serial.print("C0Dist: ");
    // Serial.println(receivedFloatValue);
  }
  delay(500);
}

// Second core for ultrasonic and led ring
void  setup1(){
  sonar.init();
  ledRing.begin();
}
void loop1(){
  float distanceC1 = sonar.getFilteredAverage();
  if (distanceC1 >=0){
    // Serial.print("C1dist:");
    // Serial.println(distanceC1);
    FloatUnion dataToSend;
    dataToSend.f = distanceC1;
    uint32_t intRepresentation = dataToSend.i;
    rp2040.fifo.push_nb(intRepresentation);
    ledRing.mapDistanceToDelay(distanceC1);
  }
}