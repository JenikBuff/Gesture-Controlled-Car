#include <Wire.h>
#include <math.h>
#include <SoftwareSerial.h>
SoftwareSerial master(10,9); //rx,tx

float ax, ay, az, gx, gy, gz, accAnglex, accAngley;

float angleX=0, angleY=0, angleZ=0;

float offsetx,offsety,offsetz;
float mountedOffsetx,mountedOffsety;

unsigned long now,then;
float dt; //this is for gyro integration
unsigned long prev;

int sample = 500;

#define PI 3.14159265

void setup() {

  pinMode(12,OUTPUT); 
  digitalWrite(12,HIGH); //VCC

  pinMode(11,OUTPUT); 
  digitalWrite(11,LOW); //GND

  Serial.begin(9600);
  master.begin(9600);
  ///////////////////////////////////////////////

  Wire.begin();  
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0b00000000);
  Wire.endTransmission(true);
  
  Serial.println("MPU STARTS NOW");
  delay(3000); //i need to be stable first
  gyroCalibration();
  accCalibration();
  Serial.println("Offsets-G");
  Serial.print("X = ");Serial.println(offsetx);
  Serial.print("Y = ");Serial.println(offsety);
  Serial.print("Z = ");Serial.println(offsetz);
  Serial.println("Offsets-A");
  Serial.print("X = ");Serial.println(mountedOffsetx);
  Serial.print("Y = ");Serial.println(mountedOffsety);

  then = millis();
  prev = millis();
}

void loop() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false); 
  Wire.requestFrom(0x68, 14);

    ax = (readHighLow() / 16384.0)-mountedOffsetx;
    ay = (readHighLow() / 16384.0)-mountedOffsety;
    az = readHighLow() / 16384.0;

    readHighLow(); // ignore temperature

    gx = (readHighLow() / 131.072)-offsetx;
    gy = (readHighLow() / 131.072)-offsety;
    gz = (readHighLow() / 131.072)-offsetz;

    accAnglex = atan2(ay,az) * (180/PI);
    accAngley = atan2(-ax,sqrt(ay*ay+az*az)) * (180/PI);

    now = millis();
    dt = (now - then)/1000.0;
    then = now;

    if (abs(gx)<0.50) gx = 0;
    if (abs(gy)<0.50) gy = 0;
    if (abs(gz)<0.50) gz = 0;

    angleX = 0.96*(angleX + (gx * dt)) + 0.04 * accAnglex;
    angleY = 0.96*(angleY + (gy * dt)) + 0.04 * accAngley;
    angleZ = angleZ + (gz * dt);

    if ((millis()-prev)>50)
    {
    Serial.print(" || Gyro ||");
    Serial.print(" X: "); Serial.print(angleX);
    Serial.print(" | Y: "); Serial.print(angleY);
    Serial.print(" | Z: "); Serial.println(angleZ);

    master.print("<");
    master.print(angleX);
    master.print(",");
    master.print(angleY);
    master.print(",");
    master.print(angleZ);
    master.print(">");

    prev = millis();
    }
    
}

void gyroCalibration()
{

  float sumx=0,sumy=0,sumz=0,rawx,rawy,rawz;

  for (int i=1;i<=sample;i++)
  {
    Wire.beginTransmission(0x68);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68,6);

    rawx = readHighLow();
    rawy = readHighLow();
    rawz = readHighLow();

    sumx = sumx + rawx;
    sumy = sumy + rawy;
    sumz = sumz + rawz;

    delay(2);

  }

  offsetx = (sumx/sample)/131.072;
  offsety = (sumy/sample)/131.072;
  offsetz = (sumz/sample)/131.072;
}

void accCalibration()
{

  float sumx=0,sumy=0,rawx,rawy;

  for (int i=1;i<=sample;i++)
  {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68,4);

    rawx = readHighLow();
    rawy = readHighLow();

    sumx = sumx + rawx;
    sumy = sumy + rawy;

    delay(2);

  }

  mountedOffsetx = (sumx/sample)/16384.0;
  mountedOffsety = (sumy/sample)/16384.0;
}

int16_t readHighLow() {
  uint8_t high = Wire.read();
  uint8_t low = Wire.read();
  return (int16_t)(high << 8 | low);
}