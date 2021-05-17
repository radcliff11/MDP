//EDIT THIS FOR CALIBRATION
int NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION = 30  ;     // This is the number of readings the sensors would read for each calibration try. The Higher the value the more stable
int NUM_OF_TRIES_CALIBRATION = 16;                     //Number of tries the robot will calibrate
float CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE = 19;           // Min Distance the Robot will calibrate Its Angle
float CAL_DIST_MIN_DISTANCE_TO_CALIBRATE = 19;           // Min Distance the Robot will calibrate Its Distance from Wall
float LEFT_TO_WALL_DISTANCE_THRESHOLD[2] = {1.62, 2.18};  // Min and Max Threshold of Distance from Wall for Front Left Sensor
float RIGHT_TO_WALL_DISTANCE_THRESHOLD[2] = {2.59, 3.16}; // Min and Max Threshold of Distance from Wall for Front Right Sensor
///////////////////////////////////////////////////////////////CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calibrate() {
  calAngle();
  delay(1);
  calDistance();
  delay(2);
}

void calAngle() {
  double L, R;
  int count = 0;
  //   desiredLeft , desiredRight = 12.0;
  double error = 0;
  double leftToWallDistance = 99.0;
  double rightToWallDistance = 99.0;
  while (1) {
    //L = getDistance(sensorRead(30, LF), LF) - 1.6;
    L = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, CF), CF) + 0.8;//0.34 //Using Front Centre Sensor and Front Right Sensor: Commented out
    R = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, RF), RF);
    //    delay(10);
    error = L - R;  


    if (count >= NUM_OF_TRIES_CALIBRATION)
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
  int count = 0;

  while (1)  {

    //    leftToWallDistance = readFrontSensor_FL();
    //    rightToWallDistance = readFrontSensor_FR();


    if (count >= NUM_OF_TRIES_CALIBRATION){
      break;
    }
    if (count == 4){
      calAngle();
    }
    leftToWallDistance = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, CF), CF);  //Using Front Centre Sensor and Front Right Sensor: Commented out
    rightToWallDistance = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, RF), RF);
    error = leftToWallDistance - rightToWallDistance;

    if (leftToWallDistance > CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE ||
        rightToWallDistance > CAL_ANGLE_MIN_DISTANCE_TO_CALIBRATE)
      break;
    //    Serial.print(leftToWallDistance);
    //    Serial.print(":");
    //    Serial.print(rightToWallDistance);
    //    Serial.print("\n");
    if ((leftToWallDistance >= LEFT_TO_WALL_DISTANCE_THRESHOLD[0] &&
         leftToWallDistance < LEFT_TO_WALL_DISTANCE_THRESHOLD[1]
        ) || (rightToWallDistance >= RIGHT_TO_WALL_DISTANCE_THRESHOLD[0] &&
              rightToWallDistance < RIGHT_TO_WALL_DISTANCE_THRESHOLD[1]))
      break;

    if (rightToWallDistance < RIGHT_TO_WALL_DISTANCE_THRESHOLD[0] || leftToWallDistance < LEFT_TO_WALL_DISTANCE_THRESHOLD[0])
    {
      //      moveForward(-100,0.9);
      md.setSpeeds(-100, -100);
      delay(abs(error) * 250);
      md.setBrakes(400, 400);
    }
    else {
      //      moveForward(100,0.9);L
      md.setSpeeds(100, 100);
      delay(abs(error) * 250);
      md.setBrakes(400, 400);
    }
    count++;
  }
}
void calRight(float error) {
  delay(1);
  if (error > 0.5)
  {
    md.setSpeeds(-250, 250);
    delay(abs(error * 50));
    md.setBrakes(400, 400);
  }

  else if (error < 0.5)
  {
    md.setSpeeds(-250, 250);
    delay(abs(error * 30));
    md.setBrakes(400, 400);
  }
}

void calLeft(float error) {
  delay(1);
  if (error > 0.5)
  {
    md.setSpeeds(250, -250);
    delay(abs(error * 50));
    md.setBrakes(400, 400);
  }
  else if (error < 0.5)
  {
    md.setSpeeds(250, -250);
    delay(abs(error * 30));
    md.setBrakes(400, 400);
  }
}
/////////////////////////////////////////////////////////////////END OF CALIBRATION//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
