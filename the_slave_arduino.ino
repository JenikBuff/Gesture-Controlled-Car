#include <SoftwareSerial.h>
#include <stdlib.h>
#include <AFMotor.h>

AF_DCMotor m1(1); 
AF_DCMotor m2(2); 
AF_DCMotor m3(3); 
AF_DCMotor m4(4); 

SoftwareSerial slave(A0,A1); //rx,tx
char info[32];
int p;
bool inArray=false;
unsigned long packetStart;

float angleX,angleY,angleZ;
unsigned long prev;
int speed =100;

void setup() {
  m1.setSpeed(speed);
  m2.setSpeed(speed);
  m3.setSpeed(speed);
  m4.setSpeed(speed);

  Serial.begin(9600);
  slave.begin(9600);
  prev = millis();
}

void loop() {
  while (slave.available())
  {
    char data = slave.read();

    if (inArray && (millis()-packetStart>150)) // if we got '<' but not '>' and goes to next iteration directly and searches for the next opener '<'
    {
      inArray = false;
      p=0;
      continue;
    }
    if (data == '<')
    {
      p=0;
      inArray = true;
      packetStart = millis();
    }

    if (inArray && ((data != '<') && (data != '>'))) {
      if (p<31)
      {
        info[p] = data;
        p++;
      }
    }
    if (data == '>')
    {
      info[p]='\0';
      p=0;
      inArray = false;

      if (Parse())
      {
      //UseWhenYouNeedToSeeValues();
      if (abs(angleY)>abs(angleX))
      {
        if (angleY>20) Backward();
        else if (angleY<-20) Forward();
        else Stop();
      }
      else
      {
        if (angleX > 50) Right();
        else if (angleX <-50) Left();
        else Stop();
      }
      }
    }
  }
}

bool Parse()
{
  char *d;
  float tx, ty, tz;
  d = strtok(info,",");
  if (d==NULL) return false;
  tx = atof(d);

  d = strtok(NULL,",");
  if (d==NULL) return false;
  ty = atof(d);  
  
  d = strtok(NULL,",");
  if (d==NULL) return false;
  tz = atof(d);

  angleX = tx;
  angleY = ty;
  angleZ = tz;
  return true;
}

void Forward()
{
  m1.run(FORWARD);
  m2.run(FORWARD);
  m3.run(FORWARD);
  m4.run(FORWARD);
}

void Backward()
{
  m1.run(BACKWARD);
  m2.run(BACKWARD);
  m3.run(BACKWARD);
  m4.run(BACKWARD);
}

void Right()
{
  m1.run(FORWARD);
  m2.run(FORWARD);
  m3.run(BACKWARD);
  m4.run(BACKWARD);
}

void Left()
{
  m1.run(BACKWARD);
  m2.run(BACKWARD);
  m3.run(FORWARD);
  m4.run(FORWARD);
}

void Stop()
{
  m1.run(RELEASE);
  m2.run(RELEASE);
  m3.run(RELEASE);
  m4.run(RELEASE);
}

void UseWhenYouNeedToSeeValues()
{
  if ((millis()-prev)>10)
    {
    Serial.print(" X: "); Serial.print(angleX);
    Serial.print(" | Y: "); Serial.print(angleY);
    Serial.print(" | Z: "); Serial.println(angleZ);
    prev = millis();
    }
}