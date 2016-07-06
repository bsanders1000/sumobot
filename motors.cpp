#include "DualVNH5019MotorShield.h"

#define FAST 200
#define SLOW 100

//basic maneuvers
#define FWD   0  //straight forward
#define BKWD  1  //straight back
#define ROTLT 2  //swivel CCW
#define ROTRT 3  //swivel CW
#define CRVLTF 4  //curve left forward
#define CRVRTF 5 //curvw right forward
#define CRVLTB 6 //curve left backward
#define CRVRTB 7 //curve right backward
#define STOP 8

#define SRCH_MAX  10

int srchPattIdx = 0;
int m1Cur = 0;
int m2Cur = 0;
int currentState = 0;
unsigned long actionTime = 0;//time period an action should execute in milliseconds
unsigned long actionStartTime = 0; //start of timer in microseconds

byte irSensor  = 0;
byte qtrSensor  = 0;
byte lastQtrSensor  = 0;
byte allSensorSerial = 0;
byte lastAllSensorSerial = 0;

DualVNH5019MotorShield md;

//unsigned int SearchPattern[SRCH_MAX]  = {BKWD,ROTLT,FWD,FWD,ROTRT,FWD,ROTRT};
unsigned int SearchPattern[SRCH_MAX]  = {
  ROTLT,ROTLT,ROTLT,ROTLT,ROTLT,
  FWD,FWD,FWD,FWD,FWD};
int m1 = 0;//left side motors
int m2 = 0;//right side motors

void stopIfFault()
{
  if (md.getM1Fault())
  {
    Serial.println("M1 fault");
    delay(4000);
    //while(1);
  }
  if (md.getM2Fault())
  {
    Serial.println("M2 fault");
    delay(4000);
    //while(1);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Dual VNH5019 Motor Shield");
  md.init();

  delay(3000);
  //Lunge();
  GoFwd();
  //Brake();
  // GoBkwd();
  // Brake();
}

void loop()
{
  //check if there is a timer set
  if (actionStartTime && (millis() - actionStartTime < actionTime)){//keep going unless there is a new threat.
    //check for new threat
    if (!allSensorSerial || (lastAllSensorSerial == allSensorSerial)){
      //continue action, no new threat
      goto GetSensors;
    } 
  } 
  if (qtrSensor){
    AvoidTheLine();
  } 
  else if (irSensor){
    AttackTheEnemy();
  } 
  else {
    LookForEnemy();
  }
GetSensors:  
  GetSensorVals();
}
void AvoidTheLine(){
  Serial.println("Avoiding the line!");
  switch (qtrSensor){
  case 0://no lines, check ir sensors
    //goto IRSwitch;
    break;
  case 4://left rear QTR
    Avoid_Tawara_Left_Rear();
    break;
  case 3://front side QTRs
    Avoid_Tawara_Front();
    break;
    //case 13:      
  case 1://right front QTR
    Avoid_Tawara_Right_Front();
    break;
    //case 5:
  case 12://back side QTR
    Avoid_Tawara_Rear();
    break;
  case 8: //left front
    Avoid_Tawara_Left_Front();
    break;
  case 2://right rear
    Avoid_Tawara_Right_Rear();
    break;
  case 9://right side QTRs
    Avoid_Tawara_Right();
    break;
  case 6://left side QTRs
    Avoid_Tawara_Left();
    break;
  default:
    //TurnCCW();
    break;    
  }
  lastQtrSensor = qtrSensor;
}
void AttackTheEnemy(){
 Serial.println("Attacking the Enemy!");
 Serial.println(irSensor);
  switch (irSensor){
  case 0:
    break;
  case 1://right front IR    
  case 4://left rear IR
    TurnCW();
    break;
  case 3://both front IRs
    GoFwd();
    break;   
  case 12://both rear IRs
    GoBkwd();
    break;
  case 2://right rear    
  case 8: //left front
    TurnCCW();
    break;
  default:
    //Brake();
    break;    
  }
}
void LookForEnemy(){
  //LookAround
  Serial.println("Looking for the enemy!");
  switch(currentState){
  case 0://forward
    GoFwd();
    break;
  case 1://backward
    GoBkwd();
    break;
  case 2://rotate left
    GoFwd();
    break;
  case 3://rotate right
    GoFwd();
    break;
  case 4://curve left forward
    //GoFwd();
    CurveRtFw();
    break;
  case 5://curve right forward
    //GoFwd();
    CurveLtFw();
    break;
  case 6://curve left backward
    //GoBkwd();
    CurveRtBk();
    break;
  case 7://curve right backward
    //GoBkwd();
    CurveLtBk();
    break;
  case 8://stop
    GoBkwd();
    break;
  default:
    break;    
  }
}
void execute(int man){
  switch (man){
  case 0://forward
    m1 = m2 = FAST;
    break;
  case 1://backward
    m1 = m2 = -FAST;
    break;
  case 2://rotate left
    m1 = FAST;
    m2 = -FAST;
    break;
  case 3://rotate right
    m1 = -FAST;
    m2 = FAST;
    break;
  case 4://curve left forward
    m1 = FAST;
    m2 = SLOW;
    break;
  case 5://curve right forward
    m1 = SLOW;
    m2 = FAST;
    break;
  case 6://curve left backward
    m1 = -FAST;
    m2 = -SLOW;
    break;
  case 7://curve right backward
    m1 = -SLOW;
    m2 = -FAST;
    break;
  case 8://brake
    md.setBrakes(SLOW, SLOW);
    break;
  default:
    break;
  }
  RunWheels(m1, m2);
  currentState = man;
}

void RunWheels(int LSpeed, int RSpeed){
  //md.setBrakes(0, 0);
  //delay(5);
  /*
   int m1Diff = LSpeed - m1Cur;
   int m2Diff = RSpeed - m2Cur;
   int wheelStep = 10;
   int m1Step = m1Diff/wheelStep;
   int m2Step = m2Diff/wheelStep;
   
   for (int i = 0; i <= wheelStep; i++){
     md.setSpeeds(m1Cur + m1Step , m2Cur + m2Step);
     Serial.print("Left Wheel: ");
     Serial.print(m1Cur + m1Step);
     Serial.print("Left Wheel: ");
     Serial.print(m1Cur + m1Step);
     
     delay(2);
     stopIfFault();
   }
   */
  if ((m1Cur * LSpeed < 0) || (m2Cur * RSpeed < 0 )){
  //if (m1Cur != LSpeed || m2Cur != RSpeed){
    md.setBrakes(0, 0);
    delay(5);
    md.setBrakes(SLOW, SLOW);
    delay(5);
    md.setBrakes(0, 0);
    delay(5);    
  }
  md.setSpeeds(LSpeed , RSpeed);
 
  m1Cur = LSpeed;
  m2Cur = RSpeed;
  /*
      Serial.print("M1 current: ");
   Serial.println(md.getM1CurrentMilliamps());
   Serial.print("M2 current: ");
   Serial.println(md.getM2CurrentMilliamps());
   */
  //delay(5);
}
void Avoid_Tawara_Front(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (BKWD);
  delay(500);
  execute(ROTLT);
  Serial.println("Avoiding Tawara on rear left!");
} 
void Avoid_Tawara_Rear(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (FWD);
  delay(500);
  execute(ROTLT);
  Serial.println("Avoiding Tawara on rear left!");
}  

void Avoid_Tawara_Left(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute(ROTRT);
  Serial.println("Avoiding Tawara on left!");
} 

void Avoid_Tawara_Right(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute(ROTLT);
  Serial.println("Avoiding Tawara on right!");
} 
void Avoid_Tawara_Left_Rear(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVRTF);
  Serial.println("Avoiding Tawara on rear left!");
}

void Avoid_Tawara_Right_Rear(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVLTF);
  Serial.println("Avoiding Tawara on rear right!");
}

void Avoid_Tawara_Right_Front(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVLTB);
  Serial.println("Avoiding Tawara on front right!");
}  

void Avoid_Tawara_Left_Front(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVRTB);
  Serial.println("Avoiding Tawara on left front!");
}  

void TurnCW(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (ROTRT);
  Serial.println("Rotating right!");
}

void TurnCCW(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (ROTLT);
  Serial.println("Rotating left!");
}
void CurveRtFw(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVRTF);
  Serial.println("Curving right forward!");
}

void CurveLtFw(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVLTF);
  Serial.println("Curving left forward!");
}
void CurveRtBk(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVRTB);
  Serial.println("Curving right back!");
}

void CurveLtBk(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (CRVLTB);
  Serial.println("Curving back left!");
}
void GoFwd(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (FWD);
  Serial.println("Going forward!");
}
void GoBkwd(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (BKWD);
  Serial.println("Going backward!");
}
void AttackFwd(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (FWD);
  delay(1000);
  Serial.println("Going forward!");
}
void AttackBkwd(){
  actionTime = 1500;
  actionStartTime  = millis();
  execute (BKWD);
  delay(1000);
  Serial.println("Going backward!");
}

void Lunge(){
  actionTime = 2000;
  actionStartTime  = millis();
  execute (FWD);
}

void Brake(){
  execute (STOP);
}
void Search(){
  /*
    for (int i = 0; i <= 5; i++){
   execute (BKWD);
   delay(2);
   } 
   */
  for (int i = 0; i <= 30; i++){
    execute (SearchPattern[srchPattIdx]);
    delay(2);
  } 
  srchPattIdx++;
  if (srchPattIdx >= SRCH_MAX){
    srchPattIdx = 0;
  }
  Serial.print("srchPattIdx = ");
  Serial.print(srchPattIdx);
  Serial.println();
}

void GetSensorVals(){
  lastAllSensorSerial = allSensorSerial;
  //read the bits out of the read val and update sensor vars
  if (Serial.available() > 0){
    allSensorSerial = Serial.read();
    //if (irSensor == 0) {
      irSensor = allSensorSerial & 0xf;
    //}
    qtrSensor = allSensorSerial >> 4;

    Serial.print("irSensor : ");
    Serial.print(irSensor, BIN);
   // Serial.println();
   //   Serial.println("Read from serial port : ");
   // Serial.print(allSensorSerial, BIN);
     Serial.print("  qtrSensor : ");
    Serial.print(qtrSensor, BIN);
    Serial.println();   
  }
  if (0 && allSensorSerial != lastAllSensorSerial){
    Serial.print("Hand Sensor: ");
    Serial.print(irSensor, BIN);
    Serial.println();
    Serial.print("QTR Sensors: "); 
    Serial.print(qtrSensor);
    Serial.println(); 
  }

}


