#ifndef LEDRING_H
#define LEDRING_H

#include <NeoPixelConnect.h>
#define LEDRING_NUM_NEOPIXELS 24

class LEDRing {
public:
  LEDRing(uint8_t pin, PIO pio, uint8_t sm);
  void begin();
  void setOrientation(int heading);
  void mapDistanceToDelay(float distance);
  

private:
  NeoPixelConnect p;
  void waiting();
  void updateAnimation(); // Method to update LED animations
  uint8_t waiting_pixel_setting[4];
  uint8_t currentAnimation;  // 0 for waiting, 1 for orientation, etc. Add more as needed
  uint8_t leadPixel;
  uint8_t adjDelay;
  unsigned long lastUpdate; // Last time animation was updated
  repeating_timer_t animationTimer;  // Repeating timer object for animation updates

};

#endif // LEDRING_H
