

//====================================================================    MOVEMENTS ===================================================================================================
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
          delay(5);
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
          delay(7);
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
        delay(5);
        initialStatus = false;
      }
      md.setSpeeds(-(currentSpeed + 0.1275 * speed_O), -(currentSpeed - speed_O));
    }
  }
  initializeMotorBack_End();  //brakes the motor
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
void rotLeft(int angle) {
  int currentSpeed = ROTATE_MAX_SPEED;
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
  } while (tick_L < tick) {
    if (myPID.Compute())
      md.setSpeeds(-(currentSpeed + speed_O), currentSpeed + speed_O);
  }
  initializeMotor_End();
}

void rotRight(int angle) {//
  int currentSpeed = ROTATE_MAX_SPEED;
  initializeTick();
  initializeMotor_Start();
  int tick;
  if (angle == 45) {
    tick = RIGHTTURNS[0];
  } else if (angle == 90) {
    tick = RIGHTTURNS[1];
  } else if (angle == 180) {
    tick = RIGHTTURNS[2];
  } else {
    tick = 0;
  } while (tick_R < tick) {
    if (myPID.Compute())
      md.setSpeeds((currentSpeed + speed_O), -(currentSpeed + speed_O));
  }
  initializeMotor_End();
}
/////////////////////////////////////////////////////////////////END  OF MOVEMENTS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
