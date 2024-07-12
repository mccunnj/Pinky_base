#include "IMUHandler.h"
#include <Arduino.h>

IMUHandler::IMUHandler() : initialized(false), gyroOffsetX(0), gyroOffsetY(0), gyroOffsetZ(0) {}

IMUHandler::~IMUHandler() {
    // Destructor
}

bool IMUHandler::begin() {
    initialized = IMU.begin();
    if (!initialized) {
        Serial.println("Failed to initialize IMU!");
    } else {
        Serial.println("IMU initialized!");

        Serial.print("Accelerometer sample rate = ");
        Serial.print(getAccelerationSampleRate());
        Serial.println(" Hz");

        Serial.print("Gyroscope sample rate = ");
        Serial.print(getGyroscopeSampleRate());
        Serial.println(" Hz");

        calibrateGyroscope();
        calculateCovariance();
    }
    return initialized;
}

bool IMUHandler::readIMUData(){
  readAccelerometer();
  readGyroscope();
  wrapIMUData();
  return true;
}

float IMUHandler::getAccelerationSampleRate() {
    return IMU.accelerationSampleRate();
}

float IMUHandler::getGyroscopeSampleRate() {
    return IMU.gyroscopeSampleRate();
}

bool IMUHandler::isAccelerationAvailable() {
    return IMU.accelerationAvailable();
}

bool IMUHandler::isGyroscopeAvailable() {
    return IMU.gyroscopeAvailable();
}

void IMUHandler::readAccelerometer() {
    if (initialized && isAccelerationAvailable()) {
        IMU.readAcceleration(ax, ay, az);
        // Convert from g to m/s²
        ax *= 9.81;
        ay *= 9.81;
        az *= 9.81;
    }
}

void IMUHandler::readGyroscope() {
    if (initialized && isGyroscopeAvailable()) {
        IMU.readGyroscope(gx, gy, gz);
        gx -= gyroOffsetX;
        gy -= gyroOffsetY;
        gz -= gyroOffsetZ;
        // Apply threshold to filter out noise
        const float threshold = 0.05;
        gx = (abs(gx) < threshold) ? 0 : gx;
        gy = (abs(gy) < threshold) ? 0 : gy;
        gz = (abs(gz) < threshold) ? 0 : gz;
        // Convert from degrees/second to rad/s
        gx *= PI / 180.0;  // Convert to rad/s
        gy *= PI / 180.0;
        gz *= PI / 180.0;
    }
}

void IMUHandler::calibrateGyroscope() {
    if (!initialized) return;

    float sumX = 0, sumY = 0, sumZ = 0;
    const int numSamples = 100;

    Serial.println("Calibrating gyroscope...");
    for (int i = 0; i < numSamples; i++) {
        while (!isGyroscopeAvailable());
        float x, y, z;
        IMU.readGyroscope(x, y, z);
        sumX += x;
        sumY += y;
        sumZ += z;
        delay(10);
    }

    gyroOffsetX = sumX / numSamples;
    gyroOffsetY = sumY / numSamples;
    gyroOffsetZ = sumZ / numSamples;

    Serial.println("Gyroscope calibrated.");
    Serial.print("Offsets: ");
    Serial.print(gyroOffsetX);
    Serial.print(", ");
    Serial.print(gyroOffsetY);
    Serial.print(", ");
    Serial.println(gyroOffsetZ);
}

void IMUHandler::calculateCovariance() {
    // Constants from datasheet for noise density
    // Accelerometer noise density at 104 Hz
    float acc_noise_density = 75.0e-6; // µg/√Hz

    // Gyroscope noise density at 104 Hz
    float gyro_noise_density = 3.8e-3; // mdps/√Hz

    // Convert noise density to RMS noise (multiply by square root of bandwidth)
    float acc_rms_noise = acc_noise_density * sqrt(104.0); // in µg
    // Assuming bandwidth or effective noise bandwidth considering ODR and high-performance mode
    float gyro_rms_noise = gyro_noise_density * sqrt(16.0e6); // in mdps

    // Convert RMS noise to covariance (assuming independent axes)
    // Accelerometer covariance matrix
    acc_covariance[0] = sq(acc_rms_noise / 1000.0); // Convert from µg to m/s^2 (1000 µg = 9.81 m/s^2)
    acc_covariance[4] = sq(acc_rms_noise / 1000.0);
    acc_covariance[8] = sq(acc_rms_noise / 1000.0);

    // Gyroscope covariance matrix
    gyro_covariance[0] = sq(gyro_rms_noise / 1000.0); // Convert from mdps to rad/s (1000 mdps = π/180 rad/s)
    gyro_covariance[4] = sq(gyro_rms_noise / 1000.0);
    gyro_covariance[8] = sq(gyro_rms_noise / 1000.0);
}

void IMUHandler::printIMUData() {
  Serial.println("Accelerometer data in m/s^2: ");
  Serial.print(ax);
  Serial.print('\t');
  Serial.print(ay);
  Serial.print('\t');
  Serial.println(az);
  Serial.println();

  Serial.println("Gyroscope data in rad/sec: ");
  Serial.print(gx);
  Serial.print('\t');
  Serial.print(gy);
  Serial.print('\t');
  Serial.println(gz);
  Serial.println();
}

void IMUHandler::wrapIMUData(){
  //Accelerometer
  accMsg = "//A";
  accMsg += ax;
  accMsg += ",";
  accMsg += ay;
  accMsg += ",";
  accMsg += az;
  accMsg += "\n";
  //Gyroscope
  gyroMsg = "//G";
  gyroMsg += gx;
  gyroMsg += ",";
  gyroMsg += gy;
  gyroMsg += ",";
  gyroMsg += gz;
  gyroMsg += "\n";
}