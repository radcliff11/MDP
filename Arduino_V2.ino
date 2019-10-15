#include <MsTimer2.h>
#include <EnableInterrupt.h>
#include "DualVNH5019MotorShield.h"
#include <PID_v1.h>

const int LEFT_PULSE = 3;           // M1 - LEFT motor pin number on Arduino board
const int RIGHT_PULSE = 11;         // M2 - RIGHT motor pin number on Arduino board
const int MOVE_FAST_SPEED = 200;    //if target distance more than 30cm
const int MOVE_MAX_SPEED = 200;     //if target distance more than 60cm 350

const int TURN_MAX_SPEED = 400;     //change this value to calibrate turning. If the rotation overshoots, decrease the speed
const int ROTATE_MAX_SPEED = 150;   //used in rotateLeft() and rotateRight()

//TICKS[0] for general cm -> ticks calibration.
//TICKS[1-9] with specific distance (by grids) e.g. distance=5, TICKS[5]
//Grids           1     2     3     4     5     6     7     8    9  //2 is used for diagonal move
int TICKS[9] = {532, 1110, 1725, 2380, 3000, 3580, 4240, 4695, 5320};  // for movement of each grid
float POW[9] = {0.44, 0.435, 0.442, 0.430, 0.430, 0.425, 0.435, 0.430, 0.430,}; // Motor Power per grid swlab3 0.45 GRID 1


//DO FOR SPECIFIC TURN 45, 90 180 degree
const int LEFTTURNS[3] = {313, 725, 1530};
const int RIGHTTURNS[3] = {305, 700, 1480};

const double kp = 18.7, ki = 0.1, kd = 2.00;

// for PID
double tick_R = 0;
double tick_L = 0;
double speed_O = 0;

//motor declaration as 'md' e.g. to set motor speed ==> md.setSpeeds(Motor1, Motor2)
DualVNH5019MotorShield md;  //our motor is Pololu Dual VNH5019 Motor Driver Shield
PID myPID(&tick_R, &speed_O, &tick_L, kp, ki, kd, REVERSE);


//////////////////////////////////////////////////////////////////////////SETUP AND INITIALIZE////////////////////////////////////////////////////////////////////////////////////////
void setupMotorEncoder() {
  md.init();
  pinMode(LEFT_PULSE, INPUT);
  pinMode(RIGHT_PULSE, INPUT);
  enableInterrupt(LEFT_PULSE, leftMotorTime, CHANGE); //Enables interrupt on a left motor (M1) - enable interrupt basically enables the interrupt flag and enables Interrupt service routines to run
  enableInterrupt(RIGHT_PULSE, rightMotorTime, CHANGE); //Enables interrupt on a left motor (M1)
}

void setupPID() {
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-400, 400);   // change this value for PID calibration. This is the maximum speed PID sets to
  myPID.SetSampleTime(5);
}

// when forward command is received, taking in the parameter of how many cm it should move
void moveForward(int distancee) {
  initializeTick();   // set all tick to 0
  initializeMotor_Start();  // set motor and brake to 0
  int distance = cmToTicks(distancee); // convert grid movement to tick value
  float power = cmtoPow(distancee);
  double currentSpeed = 0;
  boolean initialStatus = true;

  if (distancee == 10) {    // if number of tick to move < 60, move at a slower speed of 200
    currentSpeed = MOVE_MAX_SPEED;
  } else {                // if number of tick to move >= 60, move at the max speed of 350
    currentSpeed = MOVE_FAST_SPEED;
  }

  //error checking feedback in a while loop
  while (tick_R <= distance || tick_L + 30 <= distance) {
    if (distancee == 10)
    {
      if (myPID.Compute()) {
        if (initialStatus) {
          md.setSpeeds(currentSpeed + speed_O, 0);
          // delay(5);
          initialStatus = false;
        }
        md.setSpeeds(currentSpeed + speed_O * power, currentSpeed + speed_O); //*1
      }
    }
    else    // for distancee >= 20
    {
      if (myPID.Compute()) {
        if (initialStatus) {
          md.setSpeeds(currentSpeed + speed_O, 0);
          // delay(7);
          initialStatus = false;
        }

        md.setSpeeds(currentSpeed +  speed_O * power, currentSpeed + speed_O); //*0.43
      }
    }
  }
  if (distancee == 10)
  {
    initializeMotor2_End();   //brakes the motor
  }
  else
  {
    initializeMotor3_End();   //brakes the motor
  }
}

// when backward command is received, taking in the parameter of how many cm it should move
void moveBackwards(int distance) {
  initializeTick();
  initializeMotor_Start();
   //cmToTicks(distance);
  double currentSpeed = MOVE_MAX_SPEED;
  boolean initialStatus = true;

  //error checking feedback in a while loop
  while (tick_R <= distance || tick_L + 30 <= distance) {
    if (myPID.Compute()) {
      if (initialStatus) {
        //md.setSpeeds(-(currentSpeed + speed_O), 0);
        md.setSpeeds(0, -(currentSpeed + speed_O));
        //delay(5);
        initialStatus = false;
      }
      md.setSpeeds(-(currentSpeed + 0.1275 * speed_O), -(currentSpeed - speed_O));
    }
  }
  initializeMotorBack_End();  //brakes the motor
}


//for enableInterrupt() function, to add the tick for counting
void leftMotorTime() {
  tick_L++;
}

//for enableInterrupt() function, to add the tick for counting
void rightMotorTime() {
  tick_R++;
}

// to reset/initialize the ticks and speed for PID
void initializeTick() {
  tick_R = 0;
  tick_L = 0;
  speed_O = 0;
}

// to reset/initialize the motor speed and brakes for PID
void initializeMotor_Start() {
  md.setSpeeds(0, 0);
  md.setBrakes(0, 0);
}

// brakes when moving forward
void initializeMotor_End() {
  md.setSpeeds(0, 0);
  //md.setBrakes(400, 400);
  for (int i = 200; i < 400; i += 50) {
    md.setBrakes(i, i);
    delay(10);
  }
  delay(10);

}

// brakes when moving forward
void initializeMotor2_End() {
  md.setSpeeds(0, 0);
  //md.setBrakes(375, 350);
  for (int i = 200; i < 400; i += 50) {
    md.setBrakes(i * 1.05, i);
    delay(10);
  }
  delay(10);

}

// brakes when moving forward
void initializeMotor3_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(400, 400);
  delay(20);

}

// brakes when moving backward
void initializeMotorBack_End() {
  md.setSpeeds(0, 0);
  //md.setBrakes(400, 400);
  for (int i = 200; i < 400; i += 50) {
    md.setBrakes(i * 1.07, i);
    delay(10);
  }
  delay(20);
}

// brakes when turning left/right
void initializeMotorTurnR_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(375, 350);
  delay(20);
}
//For LEFT TURN
void initializeMotorTurnL_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(375, 350);
  delay(20);
}
/////////////////////////////////////////////////////////////////END SETUP AND INITIALIZE//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////CONVERSION AND MANEUVERING//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// converting distance (cm) to ticks
int cmToTicks(int cm) {
  int dist = (cm / 10) - 1; //dist is the index no. of the TICKS array of size 10
  if (dist < 10)
    return TICKS[dist]; //TICKS[10] = {545, 1155, 1760, 2380, 2985, 3615, 4195, 4775, 5370};
  return 0;
}

float cmtoPow(int cm) {
  int dist = (cm / 10) - 1;
  if (dist < 10)
    return POW[dist];
  return 0;

}

//use this function to check RPM of the motors
void testRPM(int M1Speed, int M2Speed) {
  md.setSpeeds(M1Speed, M2Speed);  //setSpeeds(Motor1, Motor2)
  delay(1000);
  Serial.println(tick_R / 562.25 * 60 );
  Serial.println(tick_L / 562.25 * 60);
  tick_R = 0;
  tick_L = 0;
}

void rotLeft(int angle) {

  int tick;
  initializeTick();
  initializeMotor_Start();
  if (angle == 45) {
    tick = LEFTTURNS[0];
  }
  else if (angle == 90) {
    tick = LEFTTURNS[1];
  }
  else if (angle == 180) {
    tick = LEFTTURNS[2];
  }
  else {
    tick = 0;
  }


  double currentSpeed = ROTATE_MAX_SPEED;
  while (tick_L < tick) {
    if (myPID.Compute())
      md.setSpeeds(-(currentSpeed + speed_O), currentSpeed + speed_O);
  }
  initializeMotor_End();
}

void rotRight(int angle) {//
  initializeTick();
  initializeMotor_Start();
  int tick;
  if (angle == 45) {
    tick = RIGHTTURNS[0];
  }
  else if (angle == 90) {
    tick = RIGHTTURNS[1];
  }
  else if (angle == 180) {
    tick = RIGHTTURNS[2];
  }
  else {
    tick = 0;
  }

  double currentSpeed = ROTATE_MAX_SPEED;
  while (tick_R < tick) {
    if (myPID.Compute())
      md.setSpeeds((currentSpeed + speed_O), -(currentSpeed + speed_O));
  }
  initializeMotor_End();
}

/////////////////////////////////////////////////////////////////END CONVERSION AND MANEUVERING//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION = 5;       // calAngle() and calDistance()
 int NUM_OF_TRIES_ANGLE_CALIBRATION = 5;                  // calAngle()
float CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE = 19;           // calAngle() and calDistance()
float ANGLE_CALIBRATION_THRESHOLD = 0.1;                  // calAngle()
float LEFT_WALL_OFFSET_CALIBRATION = 1;
float RIGHT_WALL_OFFSET_CALIBRATION = 0.0;
float LEFT_TO_WALL_DISTANCE_THRESHOLD[2] = {0.1, 2.0};  // calDistance()
float RIGHT_TO_WALL_DISTANCE_THRESHOLD[2] = {1.80, 3.2}; // calDistance()

//=======================================Pre Calibration to get Error====================================
void calRight(float error){
  delay(1);
  if(error>0.5)
  {
    md.setSpeeds(-250,250);
    delay(abs(error*30));
    md.setBrakes(400,400);
  }

  else if (error<0.5)
  {
    md.setSpeeds(-250,250);
    delay(abs(error*50));
    md.setBrakes(400,400);
  }
}

void calLeft(float error){
  delay(1);
  if(error>0.5)
  {
    md.setSpeeds(250,-250);
    delay(abs(error*30));
    md.setBrakes(400,400);
  }
  else if(error<0.5)
  {
    md.setSpeeds(250,-250);
    delay(abs(error*50));
    md.setBrakes(400,400);
  }
}

void calibrate()  {
    delay(1);
    calAngle();
    delay(1);
    calDistance();
}

void calAngle() {
  double L, R;
  int count = 0;
  //   desiredLeft , desiredRight = 12.0;
  double error = 0;

  while (1) {
    L = readFrontSensor_FL() + 1.36;
    R = readFrontSensor_FR();
    delay(10);
    error = L - R;
    tick_L = 0;
    if((R>0.5 && R< 0.8) || (L< 2.5-1.36 && L > -0.3)){
      while (tick_L < 75) {
//        if (myPID.Compute())
      md.setSpeeds(-(200 + speed_O), -(200 + speed_O));
     }    
    }

    if (count >= 5)
      break;

    if (error > 0) // left further than right
      calLeft(error);
    else if (error < 0)
      calRight(error);
    else
      md.setBrakes(400, 400);

    count++;
  }
  delay(10);
}

void calDistance() {
  float leftToWallDistance = 99.0, rightToWallDistance = 99.0;
  float error;
  
  while (1)  {
      
    leftToWallDistance = readFrontSensor_FL();
    rightToWallDistance = readFrontSensor_FR();
    error = leftToWallDistance - rightToWallDistance;

    if(leftToWallDistance > CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE || 
      rightToWallDistance > CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE)
      break;
//    Serial.print(leftToWallDistance);
//    Serial.print(":");
//    Serial.print(rightToWallDistance);
//    Serial.print("\n");
    if((leftToWallDistance >= LEFT_TO_WALL_DISTANCE_THRESHOLD[0] && 
    leftToWallDistance < LEFT_TO_WALL_DISTANCE_THRESHOLD[1]
    ) || (rightToWallDistance >= RIGHT_TO_WALL_DISTANCE_THRESHOLD[0] && 
    rightToWallDistance < RIGHT_TO_WALL_DISTANCE_THRESHOLD[1]))
      break;
    
    if (rightToWallDistance < RIGHT_TO_WALL_DISTANCE_THRESHOLD[0] || leftToWallDistance < LEFT_TO_WALL_DISTANCE_THRESHOLD[0])
    {
//      moveForward(-100,0.9);
      md.setSpeeds(-100, -100);
      delay(abs(error)*250);
      md.setBrakes(400, 400);
    }
    else {
//      moveForward(100,0.9);
      md.setSpeeds(100, 100);
      delay(abs(error)*250);
      md.setBrakes(400, 400);
    }
  }
}

/////////////////////////////////////////////////////////////////END OF CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////  Checklist ///////////////////////

//to avoid 1x1 obstacle
void avoid() {

  while (1) {
    moveForward(1 * 10);

    int frontIR1 = (int)getFrontL();
    int frontIR2 = (int)getFrontC();
    int frontIR3 = (int)getFrontR();

    int flag = 0;

    if (frontIR2 == 1) { //Obstacle is in front of Front Center sensor
      Serial.println("Obstacle Detected by Front Center Sensor");
      delay(500); rotLeft(90);
      delay(500); moveForward(2 * 10);
      delay(500); rotRight(90);
      delay(500); moveForward(4 * 10);
      delay(500); rotRight(90);
      delay(500); moveForward(2 * 10);
      delay(500); rotLeft(90);
    }
    else if (frontIR1 == 1 && frontIR2 != 1) { //Obstacle is in front of Front Left sensor
      Serial.println("Obstacle Detected by Front Left Sensor");
      delay(500); rotRight(90);
      delay(500); moveForward(1 * 10);
      delay(500); rotLeft(90);
      delay(500); moveForward(4 * 10);
      delay(500); rotLeft(90);
      delay(500); moveForward(1 * 10);
      delay(500); rotRight(90);
    }
    else if (frontIR3 == 1 && frontIR2 != 1) { //Obstacle is in front of Front Right sensor
      Serial.println("Obstacle Detected by Front Right Sensor");
      delay(500); rotRight(90);
      delay(500); moveForward(3 * 10);
      delay(500); rotLeft(90);
      delay(500); moveForward(4 * 10);
      delay(500); rotLeft(90);
      delay(50000); moveForward(3 * 10);
      delay(5000); rotRight(90);
    }
    delay(500);
  }
}

//to avoid 1x1 obstacle diagonally
void avoidDiagonal() {

  while (1) {
    moveForward(1 * 10);

    int frontIR1 = (int)getFrontL();
    int frontIR2 = (int)getFrontC();
    int frontIR3 = (int)getFrontR();

    int flag = 0;

    if (frontIR2 == 2) { //Obstacle is in front of Front Center sensor
      Serial.println("Obstacle Detected by Front Center Sensor");
      // moveForwardTick(346);
      delay(500);
      rotLeft(45);
      delay(500);
      moveForwardTick(2546);//2546
      delay(500);
      rotRight(45);
      rotRight(45);
      delay(500);
      moveForwardTick(2100);
      delay(500);
      rotLeft(50);
    }
    else if (frontIR3 == 1 && frontIR2 != 1) { //Obstacle is in front of Front Left sensor
      Serial.println("Obstacle Detected by Front Right Sensor");
      delay(500);
      rotLeft(50);
      delay(500);
      moveForwardTick(1000);
      delay(500);
      rotRight(45);
      delay(500);
      moveForwardTick(850);
      delay(500);
      rotRight(45);
      delay(500);
      moveForwardTick(1220);
      delay(500);
      rotLeft(55);
    }
    else if (frontIR1 == 1 && frontIR2 != 1) { //Obstacle is in front of Front Right sensor
      Serial.println("Obstacle Detected by Front Left Sensor");
      delay(500);
      rotRight(45);
      delay(500);
      moveForwardTick(1000);
      delay(500);
      rotLeft(55);
      delay(500);
      moveForwardTick(1000);
      delay(500);
      rotLeft(55);
      delay(500);
      moveForwardTick(1020);
      delay(500);
      rotRight(50);
    }
    delay(500);
  }
}

// for moving diagonal
void moveForwardTick(int distance) {
  initializeTick();   // set all tick to 0
  initializeMotor_Start();  // set motor and brake to 0
  double currentSpeed = MOVE_MAX_SPEED;

  //error checking feedback in a while loop
  while (tick_R <= distance || tick_L <= distance) {
    if (myPID.Compute()) {
      md.setSpeeds(currentSpeed + speed_O, currentSpeed - speed_O);
    }
  }
  initializeMotor_End();  //brakes the motor
}
