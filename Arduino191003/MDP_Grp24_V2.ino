#include <MsTimer2.h>
#include <EnableInterrupt.h>
#include "DualVNH5019MotorShield.h"
#include <PID_v1.h>

const int LEFT_PULSE = 3;           // M1 - LEFT motor pin number on Arduino board
const int RIGHT_PULSE = 11;         // M2 - RIGHT motor pin number on Arduino board
const int MOVE_FAST_SPEED = 200;    //if target distance more than 30cm 
const int MOVE_MAX_SPEED = 200;     //if target distance more than 60cm 350

const int TURN_MAX_SPEED = 400;     //change this value to calibrate turning. If the rotation overshoots, decrease the speed 
const int ROTATE_MAX_SPEED = 275;   //used in rotateLeft() and rotateRight()

int TURN_TICKS_L = 630;//689,620       //change this left encoder ticks value to calibrate left turn 
int TURN_TICKS_R = 622;//692       //change this right encoder ticks value to calibrate right turn 
//int TURN_TICKS_L_180 = 1500;
//int TURN_TICKS_R_180 = 1493;

//TICKS[0] for general cm -> ticks calibration. 
//TICKS[1-9] with specific distance (by grids) e.g. distance=5, TICKS[5] 
//Grids           1     2     3     4     5     6     7     8    9  //2 is used for diagonal move
int TICKS[9] = {490, 755, 1672, 2290, 2890, 3495, 4095, 4685, 5295};  // for movement of each grid
                                //         
const int LEFTTICK[14] = {20, 720, 30, 35, 40, 233, 393, 343, 489, 65, 70, 75, 80, 85};//adjust LEFTICK[5] for 45 Degree, LEFTICK[5] for right diagonal movement
const int RIGHTTICK[14] = {20, 725, 30, 35, 40, 223, 283, 333, 450, 65, 70, 75, 80, 85};//adjust RIGHTTICK[5] for 45 Degree,RIGHTTICK[6]for right diagonal movement


//DO FOR SPECIFIC TURN 45, 90 180 degree
const int LEFTTURNS[3] = {320,728,1565};
const int RIGHTTURNS[3] = {305,700,1505};

const double kp = 18.7, ki = 0.1, kd = 2.00;

// for PID
double tick_R = 0;
double tick_L = 0;
double speed_O = 0;
// for PID2
int right_encoder_val = 0, left_encoder_val = 0;
void RightEncoderInc(){right_encoder_val++;}
void LeftEncoderInc(){left_encoder_val++;}

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

// not used
void stopMotorEncoder() {
  disableInterrupt(LEFT_PULSE);
  disableInterrupt(RIGHT_PULSE);
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
  double currentSpeed = 0;
  boolean initialStatus = true;

  if (distancee == 10) {    // if number of tick to move < 60, move at a slower speed of 200
    currentSpeed = MOVE_MAX_SPEED;
  } else {                // if number of tick to move >= 60, move at the max speed of 350
    currentSpeed = MOVE_FAST_SPEED;
  }

  //error checking feedback in a while loop
  while (tick_R <= distance || tick_L+30 <= distance) {
    if (distancee == 10)
    {
      if (myPID.Compute()) {
        if (initialStatus) {
          md.setSpeeds(0, currentSpeed + speed_O);
        //  delay(5);
          initialStatus = false;
        }
        md.setSpeeds(currentSpeed+ speed_O*0.43, currentSpeed + speed_O);//*0.43
      }
    }
    else    // for distancee >= 20
    {
      if (myPID.Compute()) {
        if (initialStatus) {
          md.setSpeeds(0, currentSpeed + speed_O);
         // delay(7);         
          initialStatus = false;
        }
      md.setSpeeds(currentSpeed+  speed_O*0.43, currentSpeed + speed_O);//*0.43
      }
    }
  }
  if (distancee == 10)
  {/*
    initializeMotor2_End();  //brakes the motor
    initializeTick();   // set all tick to 0
    initializeMotor_Start();  // set motor and brake to 0
    while (tick_R < 3) { // -15
        if (myPID.Compute())
        {
          md.setSpeeds(0, currentSpeed - speed_O);
        }
      }*/
      initializeMotor2_End();   //brakes the motor
  }
  else
  {/*
    initializeMotorFront_End();  //brakes the motor
    initializeTick();   // set all tick to 0
    initializeMotor_Start();  // set motor and brake to 0
    while (tick_R < 5) { // -15
        if (myPID.Compute())
        {
          md.setSpeeds(currentSpeed + speed_O, 0);
        }
      }*/
    initializeMotor3_End();   //brakes the motor
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

// when backward command is received, taking in the parameter of how many cm it should move
void moveBackwards(int distance) {
  initializeTick();
  initializeMotor_Start();
  distance = cmToTicks(distance);
  double currentSpeed = MOVE_MAX_SPEED;
  boolean initialStatus = true;
   
  //error checking feedback in a while loop
  while (tick_R <= distance || tick_L +30<= distance) {
    if (myPID.Compute()) {
      if (initialStatus) {
        //md.setSpeeds(-(currentSpeed + speed_O), 0);
         md.setSpeeds(0, -(currentSpeed + speed_O));
        //delay(5);
        initialStatus = false;
      }
      md.setSpeeds(-(currentSpeed+0.1275* speed_O), -(currentSpeed - speed_O));
    }
  }
  initializeMotorBack_End();  //brakes the motor
}


//for enableInterrupt() function, to add the tick for counting 
void leftMotorTime() {tick_L++;}

//for enableInterrupt() function, to add the tick for counting
void rightMotorTime() {tick_R++;}

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
  for (int i = 200; i <400; i+=50) {
    md.setBrakes(i, i);
    delay(10);
  }
  delay(50);
}

// brakes when moving forward 
void initializeMotor2_End() {
  md.setSpeeds(0, 0);
 //md.setBrakes(375, 350);
  for (int i = 200; i <400; i+=50) {
    md.setBrakes(i*1.05, i);
    delay(20);
  }
  delay(50);
}

// brakes when moving forward 
void initializeMotor3_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(400, 400);
  delay(50);
}

// brakes when moving backward
void initializeMotorBack_End() {
  md.setSpeeds(0, 0);
  //md.setBrakes(400, 400);
  for (int i = 200; i <400; i+=50) {
    md.setBrakes(i*1.07, i);
    delay(10);
  }
  delay(50);
}

// brakes when turning left/right 
void initializeMotorTurnR_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(375, 350);
//  for (int i = 200; i <400; i+=100) {
//    md.setBrakes(i, i);
//    delay(5);
//  }
 delay(50);
}
//For LEFT TURN
void initializeMotorTurnL_End() {
  md.setSpeeds(0, 0);
  md.setBrakes(375, 350);
  
//  for (int i = 200; i <400; i+=100) {
//    md.setBrakes(i, i);
//    delay(5);
//  }
 delay(50);
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

//use this function to check RPM of the motors
void testRPM(int M1Speed, int M2Speed){
  md.setSpeeds(M1Speed, M2Speed);  //setSpeeds(Motor1, Motor2)
  delay(1000);
  Serial.println(tick_R/562.25 * 60 );
  Serial.println(tick_L/562.25 * 60);
  tick_R = 0;
  tick_L = 0;
}

//to avoid 1x1 obstacle
void avoid(){

  while(1){
    moveForward(1*10);
    
    int frontIR1 = (int)getFrontL();
    int frontIR2 = (int)getFrontC();
    int frontIR3 = (int)getFrontR();

    int flag = 0;
        
    if(frontIR2 == 1){  //Obstacle is in front of Front Center sensor
      Serial.println("Obstacle Detected by Front Center Sensor");
      delay(500);rotLeft(90);
      delay(500);moveForward(2*10);
      delay(500);rotRight(90);
      delay(500);moveForward(4*10);
      delay(500);rotRight(90);
      delay(500);moveForward(2*10);
      delay(500);rotLeft(90);
    }
    else if(frontIR1 == 1 && frontIR2 != 1){ //Obstacle is in front of Front Left sensor
      Serial.println("Obstacle Detected by Front Left Sensor");
      delay(500);rotRight(90);
      delay(500);moveForward(1*10);
      delay(500);rotLeft(90);
      delay(500);moveForward(4*10);
      delay(500);rotLeft(90);
      delay(500);moveForward(1*10);
      delay(500);rotRight(90);
    }
    else if(frontIR3 == 1 && frontIR2 != 1){ //Obstacle is in front of Front Right sensor
      Serial.println("Obstacle Detected by Front Right Sensor");
      delay(500);rotRight(90);
      delay(500);moveForward(3*10);
      delay(500);rotLeft(90);
      delay(500);moveForward(4*10);
      delay(500);rotLeft(90);
      delay(50000);moveForward(3*10);
      delay(5000);rotRight(90);
    }
    delay(500);
  }  
}

//to avoid 1x1 obstacle diagonally
void avoidDiagonal(){

  while(1){
    moveForward(1*10);
    
    int frontIR1 = (int)getFrontL();
    int frontIR2 = (int)getFrontC();
    int frontIR3 = (int)getFrontR();

    int flag = 0;
        
    if(frontIR2 == 2){  //Obstacle is in front of Front Center sensor
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
    else if(frontIR3 == 1 && frontIR2 != 1){ //Obstacle is in front of Front Left sensor
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
    else if(frontIR1 == 1 && frontIR2 != 1){ //Obstacle is in front of Front Right sensor
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

void rotateRight(int distance, int direct) {
  initializeTick();
  initializeMotor_Start();
  double currentSpeed = ROTATE_MAX_SPEED;

  while (tick_R < distance) {
    if (myPID.Compute())
      md.setSpeeds(0, direct*(currentSpeed - speed_O));
  }
  initializeMotor_End();
}

void rotateLeft(int distance, int direct) {
  initializeTick();
  initializeMotor_Start();

  double currentSpeed = ROTATE_MAX_SPEED;
  while (tick_L  < distance) {
    if (myPID.Compute())
      md.setSpeeds(direct*(currentSpeed - speed_O), 0);
  }
  initializeMotor_End();
}
void rotLeft(int angle) {

  int tick;
  initializeTick();
  initializeMotor_Start();
  if(angle==45){tick=LEFTTURNS[0];}
  else if(angle==90){tick=LEFTTURNS[1];}
  else if(angle==180){tick=LEFTTURNS[2];}
  else{tick = 0;}  


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
  if(angle==45){tick=RIGHTTURNS[0];}
  else if(angle==90){tick=RIGHTTURNS[1];}
  else if(angle==180){tick=RIGHTTURNS[2];}
  else{tick = 0;}

  double currentSpeed = ROTATE_MAX_SPEED;
  while (tick_R < tick) {
    if (myPID.Compute())
      md.setSpeeds((currentSpeed + speed_O), -(currentSpeed + speed_O));
  }
  initializeMotor_End();
}
void turnLeft(double distance){
initializeTick();
 initializeMotor_Start();
  int tick = (int) distance *20;

  double currentSpeed = ROTATE_MAX_SPEED;
  //while (tick_R < tick) {
    //if (myPID.Compute())
      md.setSpeeds(-250,250);
      delay(tick);
      md.setBrakes(400,400);
      delay(500);
  //}
  initializeMotor_End();
  
}

void turnRight(double distance) {//
 initializeTick();
 initializeMotor_Start();
  int tick = (int)  distance *20;

  double currentSpeed = ROTATE_MAX_SPEED;
  //while (tick_R < tick) {
    //if (myPID.Compute()
      md.setSpeeds(250,-250);
      delay(tick);
      md.setBrakes(400,400);
      delay(500);
  //}
  initializeMotor_End();
}


/////////////////////////////////////////////////////////////////END CONVERSION AND MANEUVERING//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for getting the number of tick to move a certain grid
void forwardCalibration(int maxGrid) {
  double desiredDistanceSensor1 = -2.09;
  double desiredDistanceSensor3 = -1.89;
  for (int grid = 1; grid < maxGrid+1; grid++)
  {
    moveForward(grid * 10);
    int ticksL = TICKS[grid-1];
    int ticksR = TICKS[grid-1];
    Serial.print("Tick before moving ");
    Serial.print(grid);
    Serial.print(" grid: ");
    Serial.print(ticksL);
    Serial.print(", ");
    Serial.print(ticksR);
    Serial.print("\n");
  
    double diffLeft = readFrontSensor_FL() - desiredDistanceSensor1;
    double diffRight = readFrontSensor_FR() - desiredDistanceSensor3;

    while ((abs(diffLeft) >= 0.2 && abs(diffLeft) < 4)|| (abs(diffRight) >= 0.2 && abs(diffRight) < 4))
    {
      if (diffLeft <  0 || diffRight < 0)
      {
        if (abs(diffLeft) < abs(diffRight))
        {
          ticksR -= (int)(abs(diffRight*12));
          rotateRight(abs(diffRight*12), abs(diffRight)/diffRight*1);
        }
        else
        {
          ticksL -= (int)(abs(diffLeft*12));
          rotateLeft(abs(diffLeft*12), abs(diffLeft)/diffLeft*1);
        }
      }
      else
      {
        if (abs(diffLeft) >= 0.2)
        {
          if (abs(diffLeft)/diffLeft == 1)
            ticksL += (int)(abs(diffLeft*12));
          else if (abs(diffLeft)/diffLeft == -1)
            ticksL -= (int)(abs(diffLeft*12));
          rotateLeft(abs(diffLeft*12), abs(diffLeft)/diffLeft*1);
        }
        else if (abs(diffRight) >= 0.2)
        {
          if (abs(diffRight)/diffRight == 1)
            ticksR += (int)(abs(diffRight*12));
          else if (abs(diffLeft)/diffRight == -1)
            ticksR -= (int)(abs(diffRight*12));
          rotateRight(abs(diffRight*12), abs(diffRight)/diffRight*1);
        }
      }
      
      diffLeft = readFrontSensor_FL() - desiredDistanceSensor1;
      diffRight = readFrontSensor_FR() - desiredDistanceSensor3;
    }
    Serial.print("Tick after moving ");
    Serial.print(grid);
    Serial.print(" grid: ");
    Serial.print(ticksL);
    Serial.print(", ");
    Serial.print(ticksR);
    Serial.print("\n");


  }
}

void alignFront() {
 // delay(2);
 
  int count = 0;
  double desiredDistanceSensor1 = 0.22;  // minus more means nearer to wall  -0.45
  double desiredDistanceSensor3 = 1.28;  // 1.08plus more means further from wall -0.67
  
  double diffLeft = readFrontSensor_FL() - desiredDistanceSensor1;
  double diffRight = readFrontSensor_FR() - desiredDistanceSensor3;
//  Serial.println(diffLeft);
//  Serial.println(diffRight);
  
  while ((abs(diffLeft) > 0.2 && abs(diffLeft) < 20)|| (abs(diffRight) > 0.2 && abs(diffRight) < 20))
  {   
    if (abs(diffLeft) >= 0.2)
    {
     rotateLeft(abs(diffLeft*8), abs(diffLeft)/diffLeft*1);
    }
    if (abs(diffRight) >= 0.2)
    {
     rotateRight(abs(diffRight*8), abs(diffRight)/diffRight*1);
    }
    diffLeft = readFrontSensor_FL() - desiredDistanceSensor1;
    diffRight = readFrontSensor_FR() - desiredDistanceSensor3;
    
    count++;
    if (count >= 8)
      break;
  }
}
void calAngle(){
   double L,R;
   int count=0;
//   desiredLeft , desiredRight = 12.0;
   double error = 0;
   
  // 3 = right
  // 4 = left

  while(1){
    L = readFrontSensor_FR();
    R = readFrontSensor_FL()+1.52;
    delay(1);
    error = L-R;
//    Serial.print("Error:");
//    Serial.println(error);
    if (count>=10)
      break;
    
    if(error>0) // left further than right
      calRight(error);
    else if(error<0)
      calLeft(error);
    else
      md.setBrakes(400,400);
      
    count++;
   }
   delay(10);  
 }
 void calDistance() {
  int SPEEDL = 120;
  int SPEEDR = 120;
  int count = 0;
  while (readFrontSensor_FL() < 30.0 && readFrontSensor_FR() < 30.0 && count != 5)//30
  {
    if ((readFrontSensor_FR() >= 1.00 && readFrontSensor_FR() < 4.00) || (readFrontSensor_FL() >= 1.00 && readFrontSensor_FL() < 4.00))//10.6,11.5 //10.95,11.05
    {
      md.setBrakes(100, 100);
      break;
    }
    else if (readFrontSensor_FR() < 4.00 || readFrontSensor_FR() < 4.00)
    {
      md.setSpeeds(-SPEEDL, -SPEEDR);
    }
    else {
      md.setSpeeds(SPEEDL, SPEEDR);
    }
    count++;
  }
  md.setBrakes(100, 100);
}
void calRight(double error){
  if(error>0.5)
  {
    md.setSpeeds(-250,250);
    delay(abs(error*30));
    md.setBrakes(400,400);
    delay(50);
  }

  else if (error<0.5)
  {
    md.setSpeeds(-250,250);
    delay(abs(error*50));
    md.setBrakes(400,400);
    delay(50);
  }
}
void calLeft(double error){
  if(error>0.5)
  {
    md.setSpeeds(250,-250);
    delay(abs(error*30));
    md.setBrakes(400,400);
    delay(1000);
  }
  else if(error<0.5)
  {
    md.setSpeeds(250,-250);
    delay(abs(error*50));
    md.setBrakes(400,400);
    delay(50);
  }
}
//void alignRight() {
//  int count = 0;
//  
//  double diff = readRightSensor()- readRightMiddleSensor();//sensor 2 +0.18,sensor1 -0.09 Sensor2, adding allow it to move closer to the wall
//
//  Serial.print(diff);
//  Serial.print("|\n");
//    
//  while (abs(diff) > 0.2)
//  {
//    
//    rotateRight(abs(diff*8), abs(diff)/diff);
//    diff = (readRightSensor()- readRightMiddleSensor());
//    count++;
//    if (count >= 6)
//      break;
//  }
//}


/////////////////////////////////////////////////////////////////END OF CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
