#include "LEDRing.h"

LEDRing::LEDRing(uint8_t pin, PIO pio, uint8_t sm) : p(pin, LEDRING_NUM_NEOPIXELS, pio, sm) {}

void LEDRing::begin(){
  currentAnimation = 0; // Start with waiting animation
  adjDelay = 0;
  lastUpdate = 0;
  leadPixel = 0;
  oldFrontLED = 25;
  headingSpeed = 1; // Movement speed in mm/s
  
  // Start a repeating timer to call updateAnimation every 1 millisecond (adjust as needed)
  add_repeating_timer_ms(2, [](repeating_timer_t *rt) {
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
      orientationAnimation();
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

void LEDRing::setOrientation(int newHeading){
  // Ensure headings are within 0-359 degrees range
  newHeading = newHeading % 360;
  // Serial.print("New orientation assigned:");
  // Serial.println(newHeading);
  // Calculate the new front LED position
  uint8_t tempFrontLED = static_cast<uint8_t>(std::round(static_cast<double>(newHeading) / 15.0));
  // This only needs to be update if the new heading changes the frontLED
  if (tempFrontLED != oldFrontLED){
    newFrontLED = tempFrontLED;
    // Serial.print("newFrontLED assigned:");
    // Serial.println(newFrontLED);
  }    
}

void LEDRing::setSpeed(uint8_t newSpeed){
  if (newSpeed != headingSpeed){
    headingSpeed = newSpeed;
    if (headingSpeed > 0){
      LEDSpeed = constrain( 205 - headingSpeed * 1.5, 15, 500);
      currentAnimation = 1; // Switch to orientation animation
    } else {
      currentAnimation = 0; // Return to waiting
    }
  }
}

void LEDRing::orientationAnimation(){
  // Front LED: 0. Thanks old me!
  // Counting up moves CounterClockwise
  static int8_t frontLED = 0;
  static int8_t leftStart = 4; // 4,5,6,7,8,9
  static int8_t rightStart = 20; // 20, 19, 18, 17, 16, 15
  static int8_t rearLED = 12; // Rear LEDs 10, 11, 12, 13, 14
  static uint8_t headingFrame = 0;

  //frontLED = static_cast<int8_t>(std::round(static_cast<double>(heading) / 15.0));
  if (newFrontLED != oldFrontLED){
    if (millis() - lastUpdate >= 25){

      //Add Gliding to new heading
      if(headingTransition()){
        // Update pixel groupings
        frontLED = newFrontLED;
        leftStart = indexWrapper(frontLED + 4, 24);
        rightStart = indexWrapper(frontLED + 20, 24);
        rearLED = indexWrapper(frontLED + 12, 24);
        headingFrame = 0;
      }
    }
  }
  else{
    // Update Animation
    if (millis() - lastUpdate >= LEDSpeed){
      //updateSides
      // Serial.print("headingFrame:");
      // Serial.println(headingFrame);
      // Serial.print("leftStart:");
      // Serial.println(leftStart);
      for(uint8_t i = 0; i <= 5; i++){
        uint8_t travelIndex = indexWrapper(headingFrame + i, 6);
        // Calculate brightness with a more noticeable fall-off
        uint8_t travelBrightness = (i == 5) ? 255 : (5 + i * 25); 
        //Left: BLUE
        uint8_t leftIndex = indexWrapper(leftStart + 5 - travelIndex, 24);
        p.neoPixelSetValue(leftIndex, 0, 0, travelBrightness, false);
        //Right: Green
        uint8_t rightIndex = indexWrapper(rightStart - 5 + travelIndex, 24);
        p.neoPixelSetValue(rightIndex, 0, travelBrightness, 0, false);
        // Serial.print("i:");
        // Serial.print(i);
        // Serial.print("travelIndex:");
        // Serial.print(travelIndex);
        // Serial.print(" leftIndex:");
        // Serial.print(leftIndex);
        // Serial.print(" rightIndex:");
        // Serial.print(rightIndex);
        // Serial.print(" Brightness:");
        // Serial.println(travelBrightness);
        // Rear: RED - Center, mid, outer
        if (leftIndex == indexWrapper(leftStart + 2, 24)){
          p.neoPixelSetValue(rearLED, travelBrightness, 0, 0, false);
        }
        else if (leftIndex == indexWrapper(leftStart + 1, 24)){
          p.neoPixelSetValue(indexWrapper(rearLED + 1, 24), travelBrightness, 0, 0, false);
          p.neoPixelSetValue(indexWrapper(rearLED - 1, 24), travelBrightness, 0, 0, false);
        }
        else if( leftIndex == leftStart){
          p.neoPixelSetValue(indexWrapper(rearLED + 2, 24), travelBrightness, 0, 0, false);
          p.neoPixelSetValue(indexWrapper(rearLED - 2, 24), travelBrightness, 0, 0, false);
        }
      }
      p.neoPixelShow();
      headingFrame++;
      if(headingFrame > 5){
        headingFrame = 0;
      }
      lastUpdate = millis();
      // Serial.println(" ");
    }
  }
}

bool LEDRing::headingTransition(){
  int8_t LEDDistance = newFrontLED - oldFrontLED;

  // Serial.print("oldFrontLED: ");
  // Serial.print(oldFrontLED);
  // Serial.print(" newFrontLED: ");
  // Serial.print(newFrontLED);

  // Normalize pixel difference to range -12 to +12 pixels
  if (LEDDistance > 12){
    LEDDistance -= 24;
  } else if (LEDDistance <= -12){
    LEDDistance+= 24;
  }
  // Serial.print(" LEDDistance:");
  // Serial.println(LEDDistance);
  if (LEDDistance > 0){
    oldFrontLED = indexWrapper(oldFrontLED + 1, 24);

  } else {
    oldFrontLED = indexWrapper(oldFrontLED - 1, 24);
  }
  // Set Pixel Values
  p.neoPixelSetValue(oldFrontLED, 255,255,255, false);
  for (uint8_t i = 1; i <= 3; i++){
    uint8_t brightness = 255 - 50 * i;
    p.neoPixelSetValue(indexWrapper(oldFrontLED + i, 24), brightness, brightness, brightness, false);
    p.neoPixelSetValue(indexWrapper(oldFrontLED - i, 24), brightness, brightness, brightness, false);
  }
  p.neoPixelShow();
  lastUpdate = millis();
  return(newFrontLED == oldFrontLED);
}

uint8_t LEDRing::indexWrapper(int8_t index, uint8_t cap){
  // Use modulo operation and adjust for negative values
  int8_t wrappedIndex = index % cap;
  if (wrappedIndex < 0) {
    wrappedIndex += cap;
  }
  return static_cast<uint8_t>(wrappedIndex);
}


