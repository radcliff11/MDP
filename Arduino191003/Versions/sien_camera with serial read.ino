#include "DualVNH5019MotorShield.h"
#include "PinChangeInt.h"
#include "SharpIR.h"

DualVNH5019MotorShield md;

#define motor_R_encoder 11  //Define pins for motor encoder input
#define motor_L_encoder 3

// SR is short range 
// LR is long range
#define SRmodel 1080
#define LRmodel 20150

#define MAX_SPEED 400
#define SPEED 200

#define PS1 A0   //PS1
#define PS2  A1   //PS2
#define PS3 A2  //PS3
#define PS4  A3   //PS4
#define PS5 A4   //PS5

#define PS6 A5    //PS6
    
SharpIR L =  SharpIR(PS1, SRmodel);
SharpIR R =  SharpIR(PS2, SRmodel);
SharpIR FR =  SharpIR(PS3, SRmodel);
SharpIR FL =  SharpIR(PS4, SRmodel);
SharpIR bL =  SharpIR(PS5, LRmodel);
//SharpIR Far =  SharpIR(PS6, LRmodel);

boolean printFlag = false;
int right_encoder_val = 0, left_encoder_val = 0;
void RightEncoderInc(){right_encoder_val++;}
void LeftEncoderInc(){left_encoder_val++;}

void setup()
{
  Serial.begin(115200);
  md.init();
  PCintPort::attachInterrupt(motor_R_encoder, RightEncoderInc, CHANGE);
  PCintPort::attachInterrupt(motor_L_encoder, LeftEncoderInc, CHANGE);
}

void loop()
{
char choice;
  
  if (Serial.available() > 0)
  {
    choice = char(Serial.read());
    readCommands(choice);
  }
  readCommands(choice);
}

void readCommands(char choice){
  switch (choice){
    case 'a':
    calibrate();
    break;

    case 'b':
    printDistance();
    break;
  }
}
/////////////////////////////////////////////////// Sends Sensor readings in form of grids in a String /////////////////////////////////////////////////////////////////////////////////
void sendSensorReadings(){
  int L,R,CL,CR,bL,bR;     // left, right, center left, center right, big left, big right
  L = getDistance(1);
  R = getDistance(2);
  CL = getDistance(3);
  CR = getDistance(4);
  bL = getDistance(5);
  bR = getDistance(6);

  Serial.print(gridDistance(L,1));
  Serial.print(":");

  Serial.print(gridDistance(R,1));
  Serial.print(":");

  Serial.print(gridDistance(CL,1));
  Serial.print(":");

  Serial.print(gridDistance(CR,1));
  Serial.print(":");

  Serial.print(gridDistance(bL,2));
  Serial.print(":");
  
  Serial.print(gridDistance(bR,2));
  Serial.print(":");
  Serial.print("\n");
}
///////////////////////////////////////////////////////////// returns distance in form of CM ////////////////////////////////////////////////////////////////////////////
 double getDistance(int sensor){
  double sum = 0;
  double average = 0;

  // LEFT
  if (sensor == 1) {
    // Get the sum of 10 values
    for (int i = 0; i < 10; i++) {
      sum = sum + L.distance();
    }
    average = sum / 10;
    return average;
  }

  // RIGHT
  if (sensor == 2) {
    for (int i = 0; i < 10; i++) {
      sum = sum + R.distance();
    }
    average = sum / 10;
    return average;
  }

 // FRONT RIGHT
  if (sensor == 3) {
    for (int i = 0; i < 10; i++) {
      sum = sum + FR.distance();
    }
    average = sum / 10;
    return average;
  }
 
   // FRONT LEFT
  if (sensor == 4) {
    for (int i = 0; i < 10; i++) {
      sum = sum + FL.distance();
    }
    average = sum / 10;
    return average;
  }
  
  // WALL HUGGER
  if (sensor == 5) {
    for (int i = 0; i < 10; i++) {
      sum = sum + bL.distance();
    }
    average = (sum / 10) + 5;
    return average;
  }

  // Center Far
  if (sensor == 6) {
    for (int i = 0; i < 10; i++) {
     // sum = sum + Far.distance();
    }
    average = sum / 10;
    return average;
  }
 }
/////////////////////////////////////////////////////////////// return distance in form of grids/////////////////////////////////////////////////////////////////////
int gridDistance(int dis, int sensorType){
    
    int grids = 0;
   

    // sensorType 1 is for SR
    //2 is for LR
        
    if(sensorType == 1){
      if(dis>30) grids = 3;
      else if(dis >=12 && dis <= 22) grids = 1;
      else if (dis > 22 && dis <= 27) grids = 2;
      else grids = -1;
    }
6:17 PM
if(sensorType == 2){
      if(dis>58) grids = 6;
      else if(dis >= 49 && dis <= 58) grids = 5;
       else if (dis >= 39 && dis <= 48) grids = 4;
       else if (dis >= 30 && dis <= 37) grids = 3;
       else if (dis > 22 && dis <= 27) grids = 2;
       else if (dis>= 12 && dis <= 22) grids = 1;
    }
    return grids;
  }
/////////////////////////////////////////// Pre calibration to get error //////////////////////////////
  void calibrate(){
   double L,R;
   double error = 0;
// 3 = right
// 4 = left
    L = getDistance(4);
    R = getDistance(3);
    error = L-R;
    Serial.println(error);

    if(error>0) // left further than right
      {
        moveRight(error);
      }
   else if(error<0)
   {
    moveLeft(error);
   }

  else{
  md.setBrakes(400,400);
  
  }
   }
   /////////////////////////////////// Prints Left and Right Distance to check if it is perpendicular//////////////////////////////////
  void printDistance(){
  
  double L,R,FL;

    L = getDistance(4);
    R = getDistance(3);
    FL = getDistance(5);
    Serial.print("Left: ");
    Serial.println(L);
    Serial.print("Right: ");
    Serial.println(R);
    Serial.print("Front Left: ");
    Serial.println(FL);
  }

  /////////////////// Move Right ///////////////////////////

  void moveRight(double error){
    md.setSpeeds(-400,400);
    delay(abs(error*10));
    md.setBrakes(400,400);
    delay(1000);
  }
////////////////////// Move Left ////////////////////////////
  void moveLeft(double error){
    md.setSpeeds(400,-400);
    delay(abs(error*10));
    md.setBrakes(400,400);
    delay(1000);
  }