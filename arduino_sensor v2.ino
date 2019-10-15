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

const int NUM_SAMPLES_MEDIAN = 19;
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
  setupPID();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.setTimeout(50);
}

void setupSerialConnection() {
  Serial.begin(115200);
  while (!Serial);
}

//void loop() {
// delay(1000);
//  for(int i = 0;i<2;i++){
//    moveForward(1*10);
//    delay(1000);
//  }
//  rotLeft(180);
////  rotRight(180);
////  for(int i = 0;i<2;i++){
////  moveForward(2*10);
////  delay(1000);
////  }
//}

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

String commandString = "";
void loop() {
      while (commandString.length() > 0) {
        executeCommand(commandString[0]);
        delay(1);
        if (commandString.length() == 1) {
          commandString = "";
          break;
        }
        commandString = commandString.substring(1, -1);
      }

  //   printDistanceReading();
  //  calAngle();
  //calDistance();

//  for (int i = 0; i < 4; i++) {
//    moveForward(1 * 10);
//    delay(1000);
//  }
//    delay(3000);
}

void serialEvent()  {
  while (Serial.available()) {
    commandString = commandString + char(Serial.read());
  }
}

// ==================== COMMANDS ===========================

void executeCommand(char command) {
  switch (command) {

    // move forward
    case 'W':
      moveForward(10);
      
      if ((getFrontL() == 1) && (getFrontR() == 1)) {
        calDistance();
        calAngle();
        calDistance();

      }
      printSensorReading();
     // delay(20);
      break;

    // turn left
    case 'A':

      rotLeft(90);

      if ((getFrontL() == 1) && (getFrontR() == 1)) {
        calDistance();
        calAngle();
        calDistance();

      }
      printSensorReading();
      //delay(20);
      break;

    // move backward
    case 'S':
      moveBackwards(10);
      //delay(10);
      printSensorReading();
      break;

    // turn right
    case 'D':
      rotRight(90);
      if ((getFrontL() == 1) && (getFrontR() == 1)) {
          calDistance();
        calAngle();
        calDistance();

      }
      printSensorReading();
    //  delay(20);
      break;

    // move diagonally
    case 'R':
      moveForwardTick(800);
      printSensorReading();
      break;

    // brake
    case 'P':
      //calDistance();
      printDistanceReading();
     // moveBackwards(150);
      break;

    // turn left diagonally
    case 'Q':
      rotLeft(45);
      printSensorReading();
      break;

    // turn right diagonally
    case 'E':
      rotRight(45);
      printSensorReading();
      break;

    // turn 180 degrees
    case 'X':
      rotRight(180);
      printSensorReading();
      //delay(20);
      break;

    // print Sensor Reading
    case 'F':
        rotRight(90);
        calAngle();
        calDistance();

        //delay(10);
        rotLeft(90);
      break;

    case 'V':
      rotRight(90);

      
      
      calAngle();
      calDistance();
      calAngle();
     // delay(10);
      rotLeft(90);
      if ((getFrontL() == 1) && (getFrontR() == 1)) {
      calDistance();
      }
      //  alignFront();
      break;

    case 'L':
      //printDistanceReading();
      calAngle();
      break;


    case 'Z':
      if ((getFrontL() == 1) && (getFrontR() == 1) && (getLeft() == 1 )) {
        rotLeft(90);
        calAngle();
        calDistance();

        //delay(10);
        rotRight(90);
        calDistance();
      }
      else if ((getFrontL() == 1) && (getFrontR() == 1) && (getRightMiddle() == 1 )) {
        rotRight(90);
        calAngle();
        calDistance();

        //delay(10);
        rotLeft(90);
        calDistance();
      }

      break;
    case '1':
      moveForward(10);
      break;
    case '2':
      moveForward(10);
      break;
    case '3':
      moveForward(10);
      break;
    case '4':
      moveForward(10);
      break;
    case '5':
      moveForward(10);
      break;
    case '6':
      moveForward(10);
      break;
    case '7':
      moveForward(10);
      break;
  }
}

//read and return the median of (5*11) front left sensor values in grid distance
int getFrontL() {
  double median = readFrontSensor_FL();
  return (shortGrid(median, 5.25, 15.80, 33.00));
}

//read and return the median of (5*11) front center sensor values in grid distance
int getFrontC() {
  double median = readFrontSensor_FC();
  return (shortGrid(median, 6.80, 18.00, 33.00));
}

//read and return the median of (5*11) front left sensor values in grid distance
int getFrontR() {
  double median = readFrontSensor_FR();
  return (shortGrid(median, 6.80, 18.00, 30.00));
}

//read and return the median of (5*11) left sensor values in grid distance
int getLeft() {
  double median = readLeftSensor();
  return (shortGrid(median, 9.90, 15.80, 26.0));
}

//read and return the median of (5*11) right middle sensor values in grid distance
int getRightMiddle() {
  double median = readRightMiddleSensor();
  return (shortGrid(median, 7.40, 18.20, 33.00));
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
    double irDistance = LF.distance() - 7.5;
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
    double irDistance = CF.distance() - 6.3;
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
  Serial.flush();
  //  digitalWrite(LED_BUILTIN, HIGH);
  //  delay(10);
  //  digitalWrite(LED_BUILTIN, LOW);
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
  Serial.print("|R2:");
  Serial.print(readRightSensor()); // print right sensor distance
  Serial.print("|\n");
  // flush waits for transmission of outoing serial data to complete
  Serial.flush();
  //  delay(1);
}
