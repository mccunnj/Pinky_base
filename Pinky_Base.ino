// This does not run on MBED for arduino nano connect rp2040
#include "UltraSonic.h"
#include "LEDRing.h"
#include "IMUHandler.h"
#include "UARThandler.h"

// Union for float to uint32_t conversion
union FloatUnion {
    float f;
    uint32_t i;
};

// CORE 0
uint32_t receivedInt;
IMUHandler imuHandler;
// C0-UART
const uint TX_PIN = 0; // Replace with your TX pin
const uint RX_PIN = 1; // Replace with your RX pin
const uint BAUD_RATE = 115200;

UARThandler uart(TX_PIN, RX_PIN, BAUD_RATE);

// CORE 1
// D11 (7) == trigger, D12 (4) == echo USE GPIO# NOT D#
// Using pio0 and sm 0
UltraSonic sonar(20, 21, pio0, 0);
// Specify the PIO and state machine to use
LEDRing ledRing(4, pio0, 1); // Using pio0 and state machine 1

// For testing orientation changes
int ledTestTimer = 0;

void setup() {
  rp2040.enableDoubleResetBootloader();
  Serial.begin(115200);
  while (!Serial && (millis() < 10000)) {
    ; // Wait for serial port to connect or timeout after 10s
  }
  
  imuHandler.begin();

  uart.init();
}

void loop() {
  // Ultrasonic from core1
  
  if (rp2040.fifo.pop_nb(&receivedInt)) {
    // Convert uint32_t back to float
    FloatUnion dataReceived;
    dataReceived.i = receivedInt;
    float receivedFloatValue = dataReceived.f;
    // Serial.print("C0Dist: ");
    // Serial.println(receivedFloatValue);
    
    // Construct ultrasonic data message
    String ultrasonicMsg = "//U";
    ultrasonicMsg += receivedFloatValue;
    ultrasonicMsg += "\n";
    
    // Send ultrasonic data over UART
    uart.sendString(ultrasonicMsg.c_str());
    // Debug output
    Serial.print("Sending ultrasonic data: ");
    Serial.println(ultrasonicMsg);  
  }
  if(imuHandler.readIMUData()){
    Serial.print("Sending IMU data: ");
    Serial.println(imuHandler.accMsg);
    uart.sendString(imuHandler.accMsg.c_str());
    Serial.print("Sending gyro data: ");
    Serial.println(imuHandler.gyroMsg);
    uart.sendString(imuHandler.gyroMsg.c_str());
  }
 


  delay(50);
}

// Second core for ultrasonic and led ring
void setup1(){
  sonar.init();
  ledRing.begin();
  ledRing.setOrientation(1);
  ledRing.setSpeed(100);
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
  if(millis() - ledTestTimer > 2000){
    ledRing.setOrientation(random(0, 359));
    ledTestTimer = millis();
  }
}