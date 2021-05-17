//#include <MsTimer2.h>
#include <EnableInterrupt.h>
#include "DualVNH5019MotorShield.h"
#include <PID_v1.h>
//FOR SENSORS
#define LF A0 // 
#define CF A1 //
#define RF A2 //
#define LL A3 // Long
#define RM A4 //

const int LEFT_PULSE = 3;           // M1 - LEFT motor pin number on Arduino board
const int RIGHT_PULSE = 11;         // M2 - RIGHT motor pin number on Arduino board
const int MOVE_FAST_SPEED = 200;    //if target distance more than 30cm
const int MOVE_MAX_SPEED = 200;     //if target distance more than 60cm 350

const int TURN_MAX_SPEED = 200;     //change this value to calibrate turning. If the rotation overshoots, decrease the speed
const int ROTATE_MAX_SPEED = 150;   //used in rotateLeft() and rotateRight()

//TICKS[0] for general cm -> ticks calibration.
//TICKS[1-9] with specific distance (by grids) e.g. distance=5, TICKS[5]
//Grids           1     2     3     4     5     6     7     8    9  //10 is used for diagonal move
int TICKS[10] = {505, 1110, 1720, 2380, 2985, 3590, 4210, 4735, 5345, 790};  // for movement of each grid
float POW[10] = {0.430,0.440,0.430,0.430,0.426,0.426,0.425,0.4245,0.4222,0.428};//{0.525, 0.430, 0.423, 0.425, 0.425, 0.425, 0.435, 0.425, 0.430, 0.435}; // Motor Power per grid

//EDIT FOR SPECIFIC TURNS   45   90   180 degree
const int LEFTTURNS[3] = {290, 688, 1525};
const int RIGHTTURNS[3] = {300, 700, 1505};

//EDIT THIS FOR PID [DO NOT TOUCH FOR THIS ROBOT!]
const double kp = 18.6, ki = 0.3, kd = 2.00; //18.7,0.1,2.00
double tick_R = 0;
double tick_L = 0;
double speed_O = 0;

//motor declaration as 'md' e.g. to set motor speed ==> md.setSpeeds(Motor1, Motor2)
DualVNH5019MotorShield md;  //our motor is Pololu Dual VNH5019 Motor Driver Shield
PID myPID(&tick_R, &speed_O, &tick_L, kp, ki, kd, REVERSE);


//////////////////////////////////////////////////////////////////////////SETUP, INITIALIZE CONVERSION ////////////////////////////////////////////////////////////////////////////////////////
// converting distance (cm) to ticks
int cmToTicks(int cm) {
  int dist = (cm / 10) - 1; //dist is the index no. of the TICKS array of size 10
  if (dist < 11)
    return TICKS[dist]; //TICKS[10] = {545, 1155, 1760, 2380, 2985, 3615, 4195, 4775, 5370};
  return 0;
}

float cmtoPow(int cm) {
  int dist = (cm / 10) - 1;
  if (dist < 11)
    return POW[dist];
  return 0;

}
//for enableInterrupt() function, to add the tick for counting
void leftMotorTime() {
  tick_L++;
}

//for enableInterrupt() function, to add the tick for counting
void rightMotorTime() {
  tick_R++;
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
String commandString = "";

void loop() {
  commandString = "";
  while (Serial.available() > 0) {

    char currChar = char(Serial.read());

    //      commandString = commandString + currChar;
    if (currChar == '\n') {
      break;
    }
    executeCommand(currChar);
  }
//    rotRight(45);
//    delay(300);
//    for(int i=0; i<5; i++){
//      moveForward(10);
//      delay(1000);
//    }
//    rotLeft(90);
//    delay(300);
//    rotLeft(90);
//    delay(300);
//    for(int i; i<5; i++){
//      moveForward(10);
//      delay(1000);
//    }
//    delay(2000);

//    for(int i=0; i<4; i++){
//      rotLeft(90);
//      delay(1000);
//    }
//delay(2000);
// getDistanceinGrids();
//sendSensors();
}



//==============================================================  SETUP, INITIALIZE CONVERSION =======================================================================================================
