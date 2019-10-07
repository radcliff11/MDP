#include <RunningMedian.h>
#include "SharpIR.h"

#define motor_R_encoder 5  //Define pins for motor encoder input
#define motor_L_encoder 3

SharpIR LF(A0, 1080);   //PS1
SharpIR RF(A1, 1080);   //PS2
SharpIR CF(A2, 1080);   //PS3
SharpIR LL(A3, 20150);   //PS4
SharpIR RM(A4, 1080);   //PS6
SharpIR RR(A5, 1080);   //PS5

const int NUM_SAMPLES_MEDIAN = 8;
double frontL_Value = 0;
double frontC_Value = 0;
double frontR_Value = 0;
double left_Value = 0;
double right_Value = 0;
double rightMiddle_Value = 0;

// the setup routine runs once when you press reset
void setup() {
  // initialize serial communication
  setupSerialConnection();
  setupMotorEncoder();
//  setupSensorInterrupt();
  setupPID();

//  while(Serial.available() > 0) {
//    char clearBuffer = Serial.read();
//  }
//  delay(20);  
}
 void setupSerialConnection() {
  Serial.begin(115200);
  while (!Serial);
}
//void loop() {
//////moveForward(5*10);
////
////
////moveForward(1*10);
//////    
//  //  delay(1000);
////printDistanceReading();
//
//alignFront();
// }
//
//void loop() {
//  delay(1000);
//for(int i = 0;i<5;i++){
//  moveForward(1*10);
//  delay(1000);
//}
//  rotLeft(180);
//  for(int i = 0;i<5;i++){
//  moveForward(1*10);
//  delay(1000);
//  }
//}
//CAMERA
//
/* FOR CALIBRATION
 MOVE GRID 1, DIAGONAL, 
moveForward(1*10);
TURN LEFT, RIGHT, 
turnLeft(90);turnLeft(90);delay(2000);
turnRight(90);turnRight(90);delay(2000);
turnLeft(45);turnRight(45);delay(2000);

DIAGONAL
turnLeft(45);moveForward(2*10);delay(2000);
turnRight(45);moveForward(2*10);delay(2000);

ALIGN FRONT
turnRight(90);alignFront();turnLeft(90);alignFront();delay(2000);

printDistanceReading();
 */

///////////////////////////////////////////////////////////////////////////// END OF FOREVER LOOP////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

// the loop routine runs over and over again forever
void loop() {

  // if not connected
  if (!Serial) {
    //Serial.println("Waiting for connection");
  }
  //for rceiving string from rpi 
  String dummy;

  // while data is available in the serial buffer
  while (Serial.available() > 0){
    //dummy = Serial.peek();
    /// Instruction Example "A1:W"
    dummy = Serial.readString();
    String commands = dummy.substring(3);
    int num_commands = commands.length();
 
    for (int i = 0; i <= num_commands-1; i++){
      if(commands[i] == '\n'){
        break;
      }

      char command = commands[i];//get the value of the command from the 3rd char
      switch (command) {
        // move forward 
        case 'W':
          moveForward(10);
          if((LF.distance()-7.5 <=6.5 )&& (RF.distance() - 6.5 <=7.5)){
            calAngle();
            calDistance();
            
          }
          printSensorReading();
          break;
        // turn left
        case 'A':
        
          rotLeft(90);
        
           if((LF.distance()-7.5 <=6.5 )&& (RF.distance() - 6.5 <=7.5)){
            calAngle();
            calDistance();
            
          }
          printSensorReading();
          
          break;
        // move backward
        case 'S':
        
          moveBackwards(10);delay(10);printSensorReading();
        
          break;
        // turn right
        case 'D':
          rotRight(90);
        
           if((LF.distance()-7.5 <=6.5 )&& (RF.distance() - 6.5 <=7.5)){
            calAngle();
            calDistance();
            
          }
          printSensorReading();
       
          break;
        // move diagonally
        case '2':
          moveForwardTick(800);printSensorReading();
           
          break;        
        // brake
        case 'P':
          calDistance();
          //printDistanceReading();
         
          break;
        // turn left diagonally
        case 'Q':
          rotLeft(45);printSensorReading();
          
          break;
        // turn right diagonally
        case 'E':
          rotRight(45);printSensorReading();
          
          break;
        // turn 180 degrees
        case 'X':
          rotRight(180);
          
          printSensorReading();
          
          break; 
         // print Sensor Reading
        case 'F':
       // calDistance();
         //calAngle();
         calDistance();
         
        calAngle();
       
          //printSensorReading();
          
          break;
        case 'V':
        rotRight(90);
        calDistance();
        calAngle();
        delay(10);
        rotLeft(90);
        //  alignFront();       
        
          break; 
        case 'C':
          rotRight(90);
          calDistance();
          calAngle();
          delay(10);
          rotLeft(90);
           calDistance();
//          alignFront();
          break;  
        case 'Z':
          rotLeft(90);
          calDistance();
        calAngle();
          delay(10);
          rotRight(90); 
          calDistance();
          break;
        case 'M':
          calDistance();
        calAngle();
          delay(10);
          calDistance();
          rotLeft(90);
          
          break;      
        }
      } 
  }
}


//read and return the median of (5*11) front left sensor values in grid distance
int getFrontL() {
  double median = readFrontSensor_FL();
  return (shortGrid(median, 6.50, 17.9, 33.00));  
}

//read and return the median of (5*11) front center sensor values in grid distance
int getFrontC() {
  double median = readFrontSensor_FC();
  return (shortGrid(median, 8.00, 20.10, 33.00));  
}

//read and return the median of (5*11) front left sensor values in grid distance
int getFrontR() {
  double median = readFrontSensor_FR();
  return (shortGrid(median, 7.5, 19.00, 30.00));  
}

//read and return the median of (5*11) left sensor values in grid distance
int getLeft() {
  double median = readLeftSensor();
  return (shortGrid(median, 10.4, 16.60, 26.0));  
}

//read and return the median of (5*11) right middle sensor values in grid distance
int getRightMiddle() {
  double median = readRightMiddleSensor();
  return (shortGrid(median, 8.20, 19.00, 33.00));  
}

//read and return the median of (5*11) right sensor values in grid distance
int getRight() {
  double median = readRightSensor();
  return (shortGrid(median, 4.06, 16.30, 45.82));  
}


// front left sensor
double readFrontSensor_FL() {
  RunningMedian frontL_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = LF.distance()-7.5;
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    frontL_Median.add(irDistance);    // add in the array  
    if (frontL_Median.getCount() == NUM_SAMPLES_MEDIAN) {
      if (frontL_Median.getHighest() - frontL_Median.getLowest() > 50)
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
    double irDistance = CF.distance()-6.3;
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


// right middle sensor
double readRightMiddleSensor() {
  RunningMedian rightMiddle_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = RM.distance() - 10.21;
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


//// right sensor
double readRightSensor() {
  RunningMedian right_Median = RunningMedian(NUM_SAMPLES_MEDIAN);
  for (int n = 0; n < NUM_SAMPLES_MEDIAN; n++) {
    double irDistance = RR.distance() - 7.18;
    //reference point at 3x3 grid boundary (30cmx30cm) is 0cm
    
    right_Median.add(irDistance);    // add in the array  
    if (right_Median.getCount() == NUM_SAMPLES_MEDIAN) {
//      if (right_Median.getHighest() - right_Median.getLowest() > 15)
//        return -10;
      right_Value = right_Median.getMedian();
    }
  }
  return  abs(right_Value);
}


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

// determine which grid it belongs for long sensor
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

void printSensorReading() {
//  print sensor reading to serial monitor
  Serial.print("A2:"); //Serial.print("FL:");
  Serial.print((int)getFrontL()); // print front-left sensor distance
  Serial.print(",");//Serial.print("|FC:");  
  Serial.print((int)getFrontC()); // print front-center sensor distance
  Serial.print(",");//Serial.print("|FR:");
  Serial.print((int)getFrontR()); // print front-right sensor distance
  Serial.print(",");//Serial.print("|L:");
  Serial.print((int)getLeft()); // print front-right sensor distance
  Serial.print(",");//Serial.print("|R1:");
  Serial.print((int)getRightMiddle()); // print front-right sensor distanced
//  Serial.print("|R2:");
//  Serial.print((int)getRight()); // print front-right sensor distance
  Serial.print("\n");
  // flush waits for transmission of outoing serial data to complete
  //Serial.flush();
  delay(10);
}

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
  Serial.print("|R1:");
  Serial.print(readRightMiddleSensor()); // print right-middle sensor distance
//  Serial.print("|R2:");
//  Serial.print(readRighIItSensor()); // print right sensor distance
  Serial.print("|\n");
  // flush waits for transmission of outoing serial data to complete
 // Serial.flush();
//  delay(1);
}