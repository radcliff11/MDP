#include "DualVNH5019MotorShield.h"
#include <RunningMedian.h>
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

//Camera Declaration
SharpIR LF(A0, 1080);   //PS1
SharpIR RF(A1, 1080);   //PS2
SharpIR CF(A2, 1080);   //PS3
SharpIR LL(A3, 20150);   //PS4
SharpIR RR(A4, 1080);   //PS5
SharpIR RM(A5, 1080);   //PS6

const int NUM_SAMPLES_MEDIAN = 19;
double frontL_Value = 0;
double frontC_Value = 0;
double frontR_Value = 0;
double left_Value = 0;
double right_Value = 0;
double rightMiddle_Value = 0;

//End of Camera Declaration

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

  // Read 
  char choice;
  
  if (Serial.available() > 0)
  {
    choice = char(Serial.read());
    readCommands(choice);
  }
  readCommands(choice);
  
 printDistanceReading();
}

void readCommands(char choice){
  switch (choice){
    case 'W':
    printSensorReading();
    break;
  }
}
/////////////////////////////////////////////////// Sensor Readings /////////////////////////////////////////////////////////////////////////////////
 void printDistanceReading() {
  //print sensor reading to serial monitor
  Serial.print("FL:");
  Serial.print(readFrontSensor_FL()); // print front-left sensor distance
  Serial.print("|FC:");  
  Serial.print(readFrontSensor_FC()); // print front-center sensor distance
  Serial.print("|FR:");
  Serial.print(readFrontSensor_FR()); // print front-right sensor distance
  Serial.print("|LL:");
  Serial.print(readLeftSensor()); // print left sensor distance
  Serial.print("|RR:");
  Serial.print(readRightSensor()); // print right sensor distance
  Serial.print("|RM:");
  Serial.print(readRightMiddleSensor()); // print right-middle sensor distance
  Serial.print("|\n");
  // flush waits for transmission of outoing serial data to complete
  Serial.flush();
  delay(10);
}
void printSensorReading() {
//  print sensor reading to serial monitor
  Serial.print("FL:");
  Serial.print((int)getFrontL()); // print front-left sensor distance
  Serial.print("|FC:");  
  Serial.print((int)getFrontC()); // print front-center sensor distance
  Serial.print("|FR:");
  Serial.print((int)getFrontR()); // print front-right sensor distance
  Serial.print("|L:");
  Serial.print((int)getLeft()); // print front-right sensor distance
  Serial.print("|R:");
  Serial.print((int)getRight()); // print front-right sensor distance
  Serial.print("|RM:");
  Serial.print((int)getRightMiddle()); // print front-right sensor distance
  Serial.print("|\n");
  // flush waits for transmission of outoing serial data to complete
  Serial.flush();
  delay(10);
}
/////////////////////////////////////////////////////////////// return Median of Sensors in Grid Distance/////////////////////////////////////////////////////////////////////
//read and return the median of (5*11) front left sensor values in grid distance
int getFrontL() {
  double median = readFrontSensor_FL();
  return (shortGrid(median, 4.87, 17.25, 32.08));  
}

//read and return the median of (5*11) front center sensor values in grid distance
int getFrontC() {
  double median = readFrontSensor_FC();
  return (shortGrid(median, 5.74, 16.93, 32.91));  
}

//read and return the median of (5*11) front left sensor values in grid distance
int getFrontR() {
  double median = readFrontSensor_FR();
  return (shortGrid(median, 5.65, 16.50, 28.45));  
}

//read and return the median of (5*11) left sensor values in grid distance
int getLeft() {
  double median = readLeftSensor();
  return (shortGrid(median, 9.78, 14.95, 23.97));  
}

//read and return the median of (5*11) right sensor values in grid distance
int getRight() {
  double median = readRightSensor();
  return (shortGrid(median, 19.51, 32.17, 45.82));  
}

//read and return the median of (5*11) right middle sensor values in grid distance
int getRightMiddle() {
  double median = readRightMiddleSensor();
  return (shortGrid(median, 9.50, 20.86, 35.00));  
}
/////////////////////////////////////////// Read Sensor in terms of CM ////////////////////////////////////////////
// front left sensor
double readFrontSensor_FL() {
  RunningMedian frontL_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = LF.distance()-7.5;
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    frontL_Median.add(irDistance);    // add in the array  
    if (frontL_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (frontL_Median.getHighest() - frontL_Median.getLowest() > 15)
        return -10;
      frontL_Value = frontL_Median.getMedian();
    }
  }
  return frontL_Value;
}

// front centre sensor
double readFrontSensor_FC() {
  RunningMedian frontC_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = CF.distance()-6.3; //-6.3
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    frontC_Median.add(irDistance);    // add in the array  
    if (frontC_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (frontC_Median.getHighest() - frontC_Median.getLowest() > 50)
        return -10;
      frontC_Value = frontC_Median.getMedian();
    }
  }
  return frontC_Value;
}


// front right sensor
double readFrontSensor_FR() {
  RunningMedian frontR_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = RF.distance() - 6.5;
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    frontR_Median.add(irDistance);    // add in the array  
    if (frontR_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (frontR_Median.getHighest() - frontR_Median.getLowest() > 15)
        return -10;
      frontR_Value = frontR_Median.getMedian();
    }
  }
  return frontR_Value;
}

// left sensor
double readLeftSensor() {
  RunningMedian left_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = LL.distance() - 9.76;
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    left_Median.add(irDistance);    // add in the array  
    if (left_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (left_Median.getHighest() - left_Median.getLowest() > 15)
        return -10;
      left_Value = left_Median.getMedian();
    }
  }
  return left_Value;
}


// right sensor
double readRightSensor() {
  RunningMedian right_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = RR.distance();
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    right_Median.add(irDistance);    // add in the array  
    if (right_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (right_Median.getHighest() - right_Median.getLowest() > 15)
        return -10;
      right_Value = right_Median.getMedian();
    }
  }
  return right_Value;
}


// right sensor
double readRightMiddleSensor() {
  RunningMedian rightMiddle_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = RM.distance();
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    rightMiddle_Median.add(irDistance);    // add in the array  
    if (rightMiddle_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (rightMiddle_Median.getHighest() - rightMiddle_Median.getLowest() > 15)
        return -10;
      rightMiddle_Value = rightMiddle_Median.getMedian();
    }
  }
  return rightMiddle_Value;
}
///////////////////////////////////////////       Type of Sensor               ///////////////////////////////////
// determine which grid it belongs for long sensor
// determine which grid it belongs for short sensor
int shortGrid(double distance, double offset1, double offset2,  double offset3) {
  if (distance == -10) 
    return -1;
  else if (distance <= offset1)
    return 1;
  else if (distance <= offset2)
    return 2;
  else if (distance <= offset3)
    return 3;
  else
    return -1;
}
int longGrid(double distance, double offset1, double offset2,  double offset3, double offset4, double offset5, double offset6) {
  if (distance == -10) 
    return -1;
  else if (distance <= offset1)
    return 1;
  else if (distance <= offset2)
    return 2;
  else if (distance <= offset3)
    return 3;
  else if (distance <= offset4)
    return 4;
  else if (distance <= offset5)
    return 5;
  else if (distance <= offset5)
    return 6;
  else
    return -1;
}



///////////////////////////////////////////           ///////////////////////////////////////////////////


