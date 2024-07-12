#ifndef LEDRING_H
#define LEDRING_H

#include <NeoPixelConnect.h>
#define LEDRING_NUM_NEOPIXELS 24

class LEDRing {
public:
  LEDRing(uint8_t pin, PIO pio, uint8_t sm);
  void begin();
  void setOrientation(int heading); // From the controller' ROS node IN DEGREES 0-359
  void setSpeed(uint8_t newSpeed);
  void mapDistanceToDelay(float distance);
  
private:
  // Methods
  void updateAnimation(); // Method to update LED animations
  void waiting();
  void orientationAnimation();
  bool headingTransition();
  uint8_t indexWrapper(int8_t index, uint8_t cap);
  // Members
  NeoPixelConnect p;
  uint8_t waiting_pixel_setting[4];
  uint8_t currentAnimation;  // 0 for waiting, 1 for orientation, etc. Add more as needed
  uint8_t leadPixel; // For waiting()
  uint8_t adjDelay; // For waiting()
  unsigned long lastUpdate; // Last time animation was updated
  repeating_timer_t animationTimer;  // Repeating timer object for animation updates
  uint8_t newFrontLED;// Holds the front LED# of the heading 
  uint8_t oldFrontLED;
  uint8_t headingSpeed; // Probably 0-100
  int LEDSpeed; // Timer for navLites

};

#endif // LEDRING_H
