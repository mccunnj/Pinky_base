//This does not work on MBED
#include "ultraSonic.h"

UltraSonic* UltraSonic::instance = nullptr;

/// @brief Constructor
/// @param trigPin: trigger pin
/// @param echoPin: echo pin
/// @param pio: pio number 0=pio0, 1 = pio1
/// @param sm: pio state machine  0-4
UltraSonic::UltraSonic(int triggerPin, int echoPin, PIO pio=pio0, uint sm=0)
  : sonar_sm(sm), sonar_pio(pio), triggerPin(triggerPin), echoPin(echoPin)
{
  instance = this; // Set the current instance
  // this->sonar_sm = sm;
  // this->sonar_pio = pio;
  sonar_offset = pio_add_program(pio, &hc_sr04_program);
  // hc_sr04_init(pio, sm, sonar_offset, triggerPin, echoPin);
  // // Set up the interrupt handler
  // irq_set_exclusive_handler(PIO0_IRQ_0, UltraSonic::PIOInterruptHandler);
  // irq_set_enabled(PIO0_IRQ_0, true);
  // pio_set_irq0_source_enabled(sonar_pio, pis_interrupt0, true);

  // Initialize rolling buffer
  bufferIndex = 0;
  memset(rollingBuffer, 0, sizeof(rollingBuffer));

}

/// @brief Destructor
UltraSonic::~UltraSonic(){}

void UltraSonic::init(){
  hc_sr04_init(sonar_pio, sonar_sm, sonar_offset, triggerPin, echoPin);
  // Set up the interrupt handler
  irq_set_exclusive_handler(PIO0_IRQ_0, UltraSonic::PIOInterruptHandler);
  irq_set_enabled(PIO0_IRQ_0, true);
  pio_set_irq0_source_enabled(sonar_pio, pis_interrupt0, true);
}

void UltraSonic::PIOInterruptHandler() {
    if (instance) {
        instance->handleInterrupt();
    }
}

void UltraSonic::handleInterrupt() {
  if (pio_interrupt_get(instance->sonar_pio, 0)) {
    // Serial.print("CPUID:");
    // Serial.print(rp2040.cpuid());
    float distance = readSonar();
    // Add distance to rolling buffer
    addToRollingBuffer(distance);
    // Clear the interrupt flag
    pio_interrupt_clear(instance->sonar_pio, 0); 
    //Serial.println("pong");
  }
}

/// @brief Read the HCSR04 and return the distance in centimeters
/// @return
float UltraSonic::readSonar() {
    // value is used to read from the sm RX FIFO
    uint32_t clock_cycles;

    // read one data item from the FIFO
    // Note: every test for the end of the echo pulse takes 2 pio clock ticks,
    //       but changes the 'timer' by only one
    clock_cycles = 2 * pio_sm_get(this->sonar_pio, this->sonar_sm);
    // using
    // - the time for 1 pio clock tick (1/125000000 s)
    // - speed of sound in air is about 340 m/s
    // - the sound travels from the HCS-R04 to the object and back (twice the distance)
    // we can calculate the distance in cm by multiplying with 0.000136
    float cm = (float) clock_cycles * 0.000136;

    // clear the FIFO: do a new measurement
    pio_sm_clear_fifos(this->sonar_pio, this->sonar_sm);
    //Serial.println(cm);
    return cm;
}

void UltraSonic::addToRollingBuffer(float value) {
    rollingBuffer[bufferIndex] = value;
    bufferIndex = (bufferIndex + 1) % ROLLING_BUFFER_SIZE; // Update index, wrap around if necessary
}

float UltraSonic::getFilteredAverage() {
  // Copy the rolling buffer safely
  float localBuffer[ROLLING_BUFFER_SIZE];
  {
    // Lock interrupts during buffer copy
    __disable_irq();  // Disable interrupts

    memcpy(localBuffer, rollingBuffer, sizeof(rollingBuffer));

    // Unlock interrupts after buffer copy
    __enable_irq();  // Enable interrupts
  }

  // 1. SORTING THE ARRAY IN ASCENDING ORDER (Bubble Sort)
  for (int i = 0; i < ROLLING_BUFFER_SIZE - 1; i++) {
    for (int j = 0; j < ROLLING_BUFFER_SIZE - i - 1; j++) {
      if (localBuffer[j] > localBuffer[j + 1]) {
        float temp = localBuffer[j];
        localBuffer[j] = localBuffer[j + 1];
        localBuffer[j + 1] = temp;
      }
    }
  }

  // Filter noise (example: take median of the sorted buffer)
  // For simplicity, taking the median here
  float medianValue = localBuffer[ROLLING_BUFFER_SIZE / 2];

  // Calculate average excluding outliers (e.g., values too different from median)
  float sum = 0;
  uint32_t count = 0;
  for (int i = 0; i < ROLLING_BUFFER_SIZE; ++i) {
    if (abs(localBuffer[i] - medianValue) < 0.5) {  // Adjust threshold as needed
      sum += localBuffer[i];
      count++;
    }
  }

  if (count == 0) {
    return -1;  // Avoid division by zero
  }

  return sum / count;
}
