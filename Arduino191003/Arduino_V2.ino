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
//printDistanceReading();
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
    for (int i = 0; i <= 1; i++){
      if(commands[i] == '\n'){
        break;
      }

      char command = commands[i];//get the value of the command from the 3rd char
      switch (command) {
        // move forward 
        case 'W':
          moveForward(10);delay(10);printSensorReading();
           
          break;
        // turn left
        case 'A':
          rotLeft(90);delay(10);printSensorReading();
          
          break;
        // move backward
        case 'S':
          moveBackwards(10);delay(10);printSensorReading();
        
          break;
        // turn right
        case 'D':
          rotRight(90);delay(10);printSensorReading();
       
          break;
        // move diagonally
        case '2':
          moveForwardTick(800);delay(10);printSensorReading();
           
          break;        
        // brake
        case 'P':
          initializeMotor2_End();delay(10);printSensorReading();
         
          break;
        // turn left diagonally
        case 'Q':
          rotLeft(45);delay(10);printSensorReading();
          
          break;
        // turn right diagonally
        case 'E':
          rotRight(45);delay(10);printSensorReading();
          
          break;
        // turn 180 degrees
        case 'X':
          rotRight(180);
          delay(10);
          printSensorReading();
          
          break; 
         // print Sensor Reading
        case 'F':
       // calDistance();
      //   calDistance();
         calAngle();
        //calAngle();
       
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
        }
      } 
  }
}
