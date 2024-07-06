#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"

#include "NanoConnectHcSr04.pio.h"
// Define the size of the rolling buffer
#define ROLLING_BUFFER_SIZE 20

class UltraSonic
{
public:
  /// @brief Constructor
  /// @param trigPin: trigger pin
  /// @param echoPin: echo pin
  /// @param pio: pio number 0=pio0, 1 = pio1
  /// @param sm: pio state machine  0-4
  UltraSonic(int trigPin, int echoPin, PIO pio, uint sm);

  /// @brief destructor
  virtual ~UltraSonic();

  void init();

  /// @brief Read the HCSR04 and return the distance in centimeters
  /// @return
  
  float getFilteredAverage();


private:
// Private members
  /// @brief selected pio
  PIO sonar_pio ;

  /// @brief selected state machine
  uint sonar_sm ;

  int triggerPin;
  int echoPin;

  // Static member for instance handling
  static UltraSonic *instance;

  uint sonar_offset;

  // Rolling buffer for sensor readings
  float rollingBuffer[ROLLING_BUFFER_SIZE];
  uint bufferIndex;

  // Private methods
  void handleInterrupt();
  static void PIOInterruptHandler();
  float readSonar();
  void addToRollingBuffer(float value);

};

#endif //ULTRASONIC_H