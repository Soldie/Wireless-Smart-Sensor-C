/*
 * Library for accelerometer ADXL355 pmdz
 *
 */

#include <ADXL355.h>

void ADXL355::setupADXL(){

  // Initialize SPI to interact with ADXL355
  SPI.begin();

  // Initalize the  data ready and chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

  //Configure ADXL355:
  writeRegister(RANGE, RANGE_2G); 
  writeRegister(POWER_CTL, MEASURE_MODE); 

  // Give the sensor time to set up:
  delay(100);

}



void ADXL355::getAxis(int *x, int *y, int *z){

	int axisAddresses[] = {XDATA1, XDATA2, XDATA3, YDATA1, YDATA2, YDATA3, ZDATA1, ZDATA2, ZDATA3};
    int axisMeasures[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dataSize = 9;

    // Read accelerometer data
    readMultipleData(axisAddresses, dataSize, axisMeasures);

    // Split data
    int xdata = (axisMeasures[0] >> 4) + (axisMeasures[1] << 4) + (axisMeasures[2] << 12);
    int ydata = (axisMeasures[3] >> 4) + (axisMeasures[4] << 4) + (axisMeasures[5] << 12);
    int zdata = (axisMeasures[6] >> 4) + (axisMeasures[7] << 4) + (axisMeasures[8] << 12);
  
    // Apply two complement
    if (xdata >= 0x80000) {
      xdata = ~xdata + 1;
    }
    if (ydata >= 0x80000) {
      ydata = ~ydata + 1;
    }
    if (zdata >= 0x80000) {
      zdata = ~zdata + 1;
    }

    *x = xdata;
    *y = ydata;
    *z = zdata;
}



float ADXL355::readTemperature() {

	unsigned int t1,t2;

	// Reading register TEMP1 and TEMP2
	t2 = readRegistry(TEMP2);
	t1 = readRegistry(TEMP1);

	unsigned int temp = (t2 << 8) | t1;

	// Scaling the value obtained
	float temperature = ((((float)temp - TEMP_BIAS)) / TEMP_SLOPE) + 25.0;

	return temperature;
}



void ADXL355::activateSelfTestMode() {

  writeRegister(SELF_TEST, ST1);
  delay(100);

  writeRegister(SELF_TEST, ST2);
  delay(100);
}



void ADXL355::resetDevice() {

  writeRegister(RESET_REG, RESET);
  writeRegister(RANGE, RANGE_2G); 
  writeRegister(POWER_CTL, MEASURE_MODE); 

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