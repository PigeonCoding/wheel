
#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Gamepad.h>

Gamepad gp;

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

const int UpShift = 10;
const int DownShift = 16;
const int Handbrake = 14;

const int GasAnalog = A1;
const int maxGasAnalogRead = 22;

//int accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
//int gyro_x, gyro_y, gyro_z; // variables for gyro raw data
//int last_gyro_x, last_gyro_y, last_gyro_z;
//int g_gyro_x, g_gyro_y, g_gyro_z;
//int temperature; // variables for temperature data


int minVal=265; int maxVal=402;

int16_t AcX,AcY,AcZ;

struct value {
  double og, last; 
  double good;
};

typedef struct value Value;

Value x,y,z;

int chg = 0;

void init_value(Value &ex) {
  ex.og = 0;
  ex.last = 0;
  ex.good = 0;
}

void change(Value &ex) {
    if (ex.og - ex.last > chg || ex.og - ex.last < -chg) {
      ex.good = ex.og;
      ex.last = ex.og;
    } 
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  //init_value(x);
  //init_value(y);
  init_value(z);

  pinMode(UpShift, INPUT_PULLUP);
  pinMode(DownShift, INPUT_PULLUP);
  pinMode(Handbrake, INPUT_PULLUP);

  

}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 14, true); // request a total of 7*2=14 registers

  AcX = Wire.read()<<8 | Wire.read(); 
  AcY = Wire.read()<<8 | Wire.read(); 
  //AcZ = Wire.read()<<8 | Wire.read();

  int xAng = map(AcX,minVal,maxVal,-89,89);
  int yAng = map(AcY,minVal,maxVal,-89,89); 
  //int zAng = map(AcZ,minVal,maxVal,-89,89);

  //x.og = RAD_TO_DEG * (atan2(-yAng, -zAng)+PI); 
  //y.og = RAD_TO_DEG * (atan2(-xAng, -zAng)+PI); 
  z.og = RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

  //gyro_x.og = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  //gyro_y.og = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  //gyro_z.og = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

  //change(x);
  //change(y);
  change(z);

  float z_use = ((z.good / 360 - 0.5) * 2);
  if (z_use > 0) {
    z_use = abs( z_use - 1 );
  }else {
    z_use = - (-abs( z_use) + 1) ;
  }

 

  //Serial.print("gY = "); Serial.print(y);
  //Serial.print(" gX = "); Serial.print(x);
  //Serial.print(" gZ = "); Serial.print(z);
  

  gp.setLeftXaxis(z_use * 127);

  int UpShiftState = digitalRead(UpShift);
  if (UpShiftState != HIGH) {
    gp.setButtonState(1, true);
  }else {
    gp.setButtonState(1, false);
  }
  //Serial.println(UpShiftState);
  
  int DownShiftState = digitalRead(DownShift);
  if (DownShiftState != HIGH) {
    gp.setButtonState(2, true);
  }else {
    gp.setButtonState(2, false);
  }
  //Serial.println(DownShiftState);
  
  int HandbrakeState = digitalRead(Handbrake);
  if (HandbrakeState != HIGH) {
    //gp.setButtonState(3, true);
     gp.setLeftYaxis(127);
  }else {
    //gp.setButtonState(3, false);
     gp.setLeftYaxis(-127);
  }
  //Serial.println(HandbrakeState);
  

  float GasValue = 0;
  for (int x = 0; x <= 5; x++){
    GasValue += ((analogRead(GasAnalog) - 506.0) / 20.0 - 0.5 ) - 0.78;  
  }
  GasValue = GasValue / 5;
  if (GasValue >= 1) {
  //  GasValue = 1.0;  
  }
  Serial.println(GasValue);
  
  gp.setRightYaxis(GasValue * 127);
 
 
  
  delay(10);

}
