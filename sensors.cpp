#include <QTRSensors.h>

#define IR1 8

#define LFIR  A4 //2
#define RFIR  A5 //1
#define LRIR  A3 //4
#define RRIR  A2 //8

#define LFQTR  9
#define RFQTR  10
#define LRQTR  11
#define RRQTR  12

#define TLFQTR  4//1
#define TRFQTR  5//2
#define TLRQTR  7//8
#define TRRQTR  6//4

#define NUM_QTR   4     // number of sensors used
#define QTR_TIMEOUT       2500  // waits for 2500 microseconds for sensor outputs to go low

bool LFirSt;
bool RFirSt;
bool LRirSt;
bool RRirSt;

bool LFqtrSt;
bool RFqtrSt;
bool LRqtrSt;
bool RRqtrSt;

bool debug_mode = 0;

byte sensors = 0;
byte lastSensors = 0;
//ir gets the low bits
byte irSensor  = 0;
//qtr gets the hi bits
byte qtrSensor  = 0;
byte flipSensor = 0;
int counter = 0;

QTRSensorsRC qtrrc((unsigned char[]) {
  RFQTR, RRQTR, LRQTR, LFQTR}
, NUM_QTR, QTR_TIMEOUT);

QTRSensorsRC qtrrct((unsigned char[]) {
  TLRQTR, TLFQTR, TRFQTR, TRRQTR}
, NUM_QTR, QTR_TIMEOUT);

unsigned int sensorValues[NUM_QTR];

void setup()
{
  Serial.begin(115200);
  if (debug_mode) Serial.println("Sensors");

  pinMode(IR1, INPUT);

  pinMode(LFIR, INPUT);
  pinMode(RFIR, INPUT);
  pinMode(LRIR, INPUT);
  pinMode(RRIR, INPUT);
}
void loop()
{
  ReadIR();
  ReadFlipSensor();
  ReadQTR();
  if (debug_mode){
    //Serial.println("Sensors :" + sensors);
    //Serial.print("Hand Sensor: ");
    //Serial.print(handSensor, BIN);
    //Serial.println();
    //Serial.println("QTR Sensors: " + qtrSensor); 
    if (lastSensors != sensors){
     // Serial.print("Sensors: "); 
     // Serial.print(sensors, BIN);
     // Serial.println(); 
    }    
  } else {
    //if (Serial.available() == 0){
      Serial.write(sensors);
    //}

  }
  irSensor = sensors & 0xf;
  //handSensor = sensors & 0xf;
  qtrSensor = sensors >> 4;
  lastSensors = sensors;
  delay(100);
}
void ReadFlipSensor(){
  flipSensor = (digitalRead(IR1)); 
}
void ReadIR(){
  if (LFirSt == false){
    LFirSt = !digitalRead(LFIR);
  }
  if (RFirSt == false){
    RFirSt = !digitalRead(RFIR);
  }
  if (LRirSt == false){
    LRirSt = !digitalRead(LRIR);
  }
  if (RRirSt == false){
    RRirSt = !digitalRead(RRIR);
  }
//if we see a "1", keep it fo a few rounds
  //bit order RFirSt, LFirSt, LRirSt, RRirSt 
  if (counter == 5){
    if (flipSensor == 1){
      bitWrite(sensors,2,RFirSt);
      bitWrite(sensors,3,LFirSt);
      bitWrite(sensors,0,LRirSt);
      bitWrite(sensors,1,RRirSt);
    }else{
      bitWrite(sensors,0,RFirSt);
      bitWrite(sensors,1,LFirSt);
      bitWrite(sensors,2,LRirSt);
      bitWrite(sensors,3,RRirSt);
    }
    counter = 0;
    LFirSt = false;
    RFirSt = false;
    LRirSt = false;
    RRirSt = false;
  }
  counter++;
  if (debug_mode){
    Serial.print("IRSensors: ");
    Serial.print(sensors & 0xf, BIN);
    Serial.println();
    Serial.print("Counter: ");
    Serial.println(counter);
    //Serial.println("1 LF - " + LFirSt);
    //Serial.println("4 RF - " + RFirSt);
    //Serial.println("2 LR - " + LRirSt);
    //Serial.println("3 RR - " + RRirSt);
    
  }
}
void ReadQTR(){
  if (flipSensor){
    qtrrct.read(sensorValues);
  }else{
    qtrrc.read(sensorValues);
  }
  int bitVal = 0;
  if (debug_mode) Serial.print("QTR Values :");
  for (int i = 0; i < NUM_QTR; i++)
  {
    if (sensorValues[i] < QTR_TIMEOUT){
      //I saw the line!!!
      bitWrite(sensors,i + 4,1);
    }
    else{
      bitWrite(sensors,i + 4,0);
    }
    if (debug_mode){
      Serial.print(" ");
      Serial.print(sensorValues[i]);
      Serial.print(" ");
    }
  }
  if (debug_mode){  
    Serial.println();
    Serial.print("QTRSensors: ");
    Serial.print(sensors >> 4, BIN);
    Serial.println();
  }
}