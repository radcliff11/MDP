/////////////////////////////////////////////////////////////////// FOR COMMANDS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void executeCommand(char command) {
  switch (command) {
    case '\0':
    case '\n':
      break;
    case 'P':
      distanceSensors();
      break;
    // turn 180 degrees
    case 'X':
      rotLeft(90); delay(200); rotLeft(90);delay(200);
      break;
    // right align
    case 'F':
      rotRight(90); calibrate(); rotLeft(90);
      break;
    case 'L':
      calibrate();
      break;
    case 'V':
      rotRight(90); calibrate(); rotLeft(90);
      break;
    case 'C':
      calibrate();
      break;
    case 'O':
      sendSensors();
      break;
    // turn left
    case 'N':
     rotLeft(90); delay(200);
      break;
    case 'A':
      rotLeft(90); delay(200);
      break;
    // turn right
    case 'D':
     rotRight(90); delay(200);
     break;
    case 'M':
      rotRight(90); delay(200);
      break;
    // turn left diagonally
    case 'Q':
      rotLeft(45); delay(200);
      break;

    // turn right diagonally
    case 'E':
      rotRight(45); delay(200);
      break;
    // move diagonally
    case 'R':
      moveForward(100); delay(200 );
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
//      for (int n = 0; n < command - '0'; n++) {
        moveForward((command-'0')*10);
        delay(200);
//      }
      delay(200);
      break;
    case 'W':
      moveForward(10);
      break;
  }
}
