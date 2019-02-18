/*
 * Header file for ADXL355.cpp
 *
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>

class ADXL355{
  public:
    /*
     * Starting ADXL355
     */
    void setupADXL();

    /* 
     * ADXL in low power mode
     */
    void activateStandByMode();
    
    /* 
     * ADXL in measurement mode
     */
    void activateMeasurementMode();
    
    /* 
     * Diagnosis of the sensor
     */
    void activateSelfTestMode();

    /* 
     * Resetting ADXL
     */
    void resetDevice();

    /*
     * Return data from temperature sensor
     */
    float readTemperature();

    /*
     * Get values of acceleration of each axis 
     */
    void getAxis(int *x, int *y, int *z);
    

  private:
    // Memory register addresses:
    const int XDATA3 = 0x08;
    const int XDATA2 = 0x09;
    const int XDATA1 = 0x0A;
    const int YDATA3 = 0x0B;
    const int YDATA2 = 0x0C;
    const int YDATA1 = 0x0D;
    const int ZDATA3 = 0x0E;
    const int ZDATA2 = 0x0F;
    const int ZDATA1 = 0x10;
    const int RANGE = 0x2C;
    const int POWER_CTL = 0x2D;
    const int RESET_REG = 0x2F;
    const int SELF_TEST = 0x2E;
    const int TEMP2 = 0x06;
    const int TEMP1 = 0x07;

    // Device values
    const int RANGE_2G = 0x01;
    const int RANGE_4G = 0x02;
    const int RANGE_8G = 0x03;
    const int MEASURE_MODE = 0x06;  // Only accelerometer
    const int STAND_BY_MODE = 0x07; // Low power standby 
    const int RESET = 0x52;
    const int ST1 = 0x01;
    const int ST2 = 0x02;
    const float TEMP_BIAS = 1852.0; //Accelerometer temperature bias(in ADC codes) at 25 Deg C 
    const float TEMP_SLOPE = -9.05; //Accelerometer temperature change from datasheet (LSB/degC) 

    // Operations
    const int READ_BYTE = 0x01;
    const int WRITE_BYTE = 0x00;

    // Pins used for the connection with the sensor
    const int CHIP_SELECT_PIN = 7;

    /* 
     * Read multiple registries
     */
    void readMultipleData(int *addresses, int dataSize, int *readedData);

    /* 
     * Write registry in specific device address
     */
    void writeRegister(byte thisRegister, byte thisValue);

    /* 
     * Read registry in specific device address
     */
    unsigned int readRegistry(byte thisRegister);

};

