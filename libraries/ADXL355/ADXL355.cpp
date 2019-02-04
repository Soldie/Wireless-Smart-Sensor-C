/*
 * ADXL355.cpp contains functions to use the sensor ADXL 355
 */

#include <ADXL355.h>

void ADXL355::setupADXL(){

  // Initialize SPI to interact with ADXL355
  SPI.begin();

  // Initalize the  data ready and chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

  //Configure ADXL355:
  writeRegister(RANGE, RANGE_2G); 
  writeRegister(POWER_CTL, STAND_BY_MODE); 

  // Give the sensor time to set up:
  delay(100);
}

void ADXL355::resetDevice() {

  writeRegister(RESET_REG, RESET);
  delay(100);
}

void ADXL355::activateStandByMode() {

  writeRegister(POWER_CTL, STAND_BY_MODE); 
  delay(100);
}

void ADXL355::activateMeasurementMode() {

  writeRegister(POWER_CTL, MEASURE_MODE); 
  delay(100);
}

void ADXL355::writeRegister(byte thisRegister, byte thisValue) {
  byte dataToSend = (thisRegister << 1) | WRITE_BYTE;
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  SPI.transfer(thisValue);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}

unsigned int ADXL355::readRegistry(byte thisRegister) {
  unsigned int result = 0;
  byte dataToSend = (thisRegister << 1) | READ_BYTE;

  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  return result;
}

void ADXL355::readMultipleData(int *addresses, int dataSize, int *readedData) {
  digitalWrite(CHIP_SELECT_PIN, LOW);
  for(int i = 0; i < dataSize; i = i + 1) {
    byte dataToSend = (addresses[i] << 1) | READ_BYTE;
    SPI.transfer(dataToSend);
    readedData[i] = SPI.transfer(0x00);
  }
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}