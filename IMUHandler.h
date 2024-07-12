#ifndef IMUHANDLER_H
#define IMUHANDLER_H

#include <Arduino_LSM6DSOX.h>

class IMUHandler {
public:
  IMUHandler();
  ~IMUHandler();
  bool begin();
  void printIMUData();
  
  bool readIMUData();
  float ax, ay, az; // Acceleration variables
  float gx, gy, gz; // Gyroscope variables
  // Covariance matrices
  float acc_covariance[9]; // Acceleration covariance matrix
  float gyro_covariance[9]; // Gyroscope covariance matrix
  //Strings for UART TX
  String accMsg;
  String gyroMsg;

private:
  bool initialized;
  float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
  // Helper functions
  void calibrateGyroscope();
  float getAccelerationSampleRate();
  float getGyroscopeSampleRate();
  bool isAccelerationAvailable();
  bool isGyroscopeAvailable();
  void readAccelerometer();
  void readGyroscope();
  void calculateCovariance();
  void wrapIMUData();
};

#endif // IMUHANDLER_H
