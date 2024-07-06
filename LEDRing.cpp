#include "LEDRing.h"

LEDRing::LEDRing(uint8_t pin, PIO pio, uint8_t sm) : p(pin, LEDRING_NUM_NEOPIXELS, pio, sm) {}

void LEDRing::begin(){
  currentAnimation = 0; // Start with waiting animation
  adjDelay = 0;
  lastUpdate = 0;
  leadPixel = 0; 
  
  // Start a repeating timer to call updateAnimation every 1 millisecond (adjust as needed)
    add_repeating_timer_ms(1, [](repeating_timer_t *rt) {
        LEDRing *ledRingInstance = static_cast<LEDRing *>(rt->user_data);
        ledRingInstance->updateAnimation();
        return true;  // Continue repeating
    }, this, &animationTimer);
}

void LEDRing::updateAnimation(){
  switch (currentAnimation) {
    case 0:  // Default animation (waiting)
      waiting();
      break;
    case 1:  // Orientation animation
      // Implement orientation animation logic here
      // Example:
      // setOrientation(heading);
      break;
    // Add more cases for additional animations as needed
    default:
      waiting();  // Default to waiting if animation type is not recognized
      break;
    }
}
  
void LEDRing::waiting(){
  if( millis() - lastUpdate > adjDelay){
    // this array will hold a pixel number and the rgb values 
    uint8_t waiting_pixel_setting[4];
    for(int j = 0; j < LEDRING_NUM_NEOPIXELS; j++){//pixel update
      //Lead pixel will be i + j
      uint8_t pixel = leadPixel + j;
      while(pixel >= LEDRING_NUM_NEOPIXELS){
        pixel -= LEDRING_NUM_NEOPIXELS;
      }
      waiting_pixel_setting[0] = pixel;
      
      waiting_pixel_setting[3] = random( 255 - ((255 / LEDRING_NUM_NEOPIXELS) * j) - (255 / LEDRING_NUM_NEOPIXELS) ,255 - ((255 / LEDRING_NUM_NEOPIXELS) * j));
      // Serial.print("B:");
      // Serial.println(waiting_pixel_setting[3]);
      
      waiting_pixel_setting[1] = waiting_pixel_setting[3] / random(3, 5);
      // Serial.print("R:");
      // Serial.println(waiting_pixel_setting[1]);
      
      waiting_pixel_setting[2] = waiting_pixel_setting[3] / random(5, 10);
      // Serial.print("G:");
      // Serial.println(waiting_pixel_setting[2]);
      
      p.neoPixelSetValue(pixel, waiting_pixel_setting[1], waiting_pixel_setting[2], waiting_pixel_setting[3], false);
    }
    p.neoPixelShow();
    // if(adjDelay == 120){
    // adjDelay = 0;
    // } else {
    //   adjDelay++;
    // }
    Serial.println(adjDelay);
    if(leadPixel == 0){
      leadPixel = LEDRING_NUM_NEOPIXELS - 1;
    } else {
      leadPixel--;
    }    
    lastUpdate = millis();
  }
}

void LEDRing::mapDistanceToDelay(float distance){
  if (distance < 15.5){
    adjDelay = map((distance * 10), 30, 155, 125, 3);
  }
  else {
    adjDelay = 1;
  }
}
