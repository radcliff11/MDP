//EDIT THIS FOR SENSOR READINGS
int NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING = 80;      //The higher the less chance of phantom blocks. Please be careful and choose a balance between reliability and speed
//HOW THE SENSORS WORKS TO GET THIS GRID IS IT WILL TAKE 50 SAMPLES AND GET THE MEDIAN FROM THIS 50 SAMPLES. TOO MUCH SAMPLES MAY ALSO BECOME UNRELIABLE.
//PREVIOUSLY WE WERE TAKING 31X19 SAMPLES WHICH IS ALMOST 600 SAMPLES.
//EDIT THE VALUES FOR GRID 1 2 3 RESPECTIVELY
float FL_GRID[3] = {6.82, 15.91, 27.53};
float FC_GRID[3] = {5.26, 13.78, 17.58};
float FR_GRID[3] = {5.45, 14.45, 24.45};
float LL_GRID[5] = {17.1, 23.70, 33, 45.3, 56.5};
float RM_GRID[3] = {8.12, 17.67, 29.3};

//=========================Send Sensor String Codes=============================
void sendSensors() {
  int* sr = new int(5);
  sr[0] = getDistanceinGrids(getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING, LF), LF), LF); //reads 30 samples gives median in grid
  sr[1] = getDistanceinGrids(getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING, CF), CF), CF);//reads 30 samples gives median in grid
  sr[2] = getDistanceinGrids(getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING, RF), RF), RF);//reads 30 samples gives median in grid
  sr[3] = getDistanceinGrids(getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING, LL), LL), LL);//reads 30 samples gives median in grid
  sr[4] = getDistanceinGrids(getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_FOR_MAPPING, RM), RM), RM);//reads 50 samples gives median in grid
  Serial.print("A2:");
  for (int i = 0; i < 4; ++i) {
    Serial.print(sr[i]);
    Serial.print(",");
  }
  Serial.print(sr[4]);
  Serial.print("\n");
  Serial.flush();
}
void distanceSensors() {
  float* sr = new float(5);
  //print Distance Reading of Sensors
  sr[0] = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, LF), LF); //reads 30 samples gives median in grid
  sr[1] = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, CF), CF);//reads 30 samples gives median in grid
  sr[2] = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, RF), RF);//reads 30 samples gives median in grid
  sr[3] = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, LL), LL);
  sr[4] = getDistance(sensorRead(NUM_OF_SENSOR_READINGS_TO_TAKE_CALIBRATION, RM), RM);
  Serial.print("A2:");
  for (int i = 0; i < 4; ++i) {
    Serial.print(sr[i]);
    Serial.print(",");
  }
  Serial.print(sr[4]);
  Serial.print("\n");
  // float error =  sr[0] - sr[1];
  // Serial.println(error);
  Serial.flush();
}

//=========================End of Send Sensor String Codes======================

//==============================Get Sensor Codes================================
int sensorRead(int n, int sensor) {
  int x[n];
  int i;
  int sum = 0;
  for (i = 0; i < n; i++) {
    delay(1);
    x[i] = analogRead(sensor);
  }
  insertionsort(x, n);
  return x[n / 2];        //Return Median
}

void insertionsort(int array[], int length) {
  int i, j;
  int temp;
  for (i = 1; i < length; i++) {
    for (j = i; j > 0; j--) {
      if (array[j] < array[j - 1]) {
        temp = array[j];
        array[j] = array[j - 1];
        array[j - 1] = temp;
      }
      else
        break;
    }
  }
}

float getDistance(int reading, int sensor) {
  float cm;

  switch (sensor) {
    case LF:
      cm = 6088.0 / (reading  + 7); // 21-26=2  11-17=1  -6=0
      break;
    case CF:
      cm = 6088.0 / (reading  + 7); // 18-21=2  10-14=1  -5=0
      break;
    case RF:
      cm = 6088.0 / (reading  + 7); // 19-23=2  10-14=1  -5=0
      break;
    case RM:
      cm = 6088.0 / (reading  + 7); // 17-22=2   8-13=1  -2=0
      break;
    case LL:
      cm = 15500.0 / (reading + 29); // 46-=5 36-40=4  26-31=3  17-22=2
      break;
    default:
      return -1;
  }

  return cm - 10;
}
//=========================End of Get Sensor Codes==============================



//===============================Get Grid Codes=================================
int getDistanceinGrids(int reading, int sensor) {
  //  Serial.println(reading);
  int grid = -1;
  switch (sensor) {
    case LF:
      if (reading <= FL_GRID[0]) {
        grid = 1;
      } else if (reading <= FL_GRID[1]) {
        grid = 2;
      } else if (reading <= FL_GRID[2]) {
        grid = 3;
      }
      break;
    case CF:
      if (reading <= FC_GRID[0]) {
        grid = 1;
      } else if (reading <= FC_GRID[1]) {
        grid = 2;
      } else if (reading <= FC_GRID[2]) {
        grid = 3;
      }
      break;
    case RF:
      if (reading <= FR_GRID[0]) {
        grid = 1;
      } else if (reading <= FR_GRID[1]) {
        grid = 2;
      } else if (reading <= FR_GRID[2]) {
        grid = 3;
      }
      break;
    case RM:
      if (reading <= RM_GRID[0]) {
        grid = 1;
      } else if (reading <= RM_GRID[1]) {
        grid = 2;
      } else if (reading <= RM_GRID[2]) {
        grid = 3;
      } break;
    case LL:
      if (reading <= LL_GRID[0]) {
        grid = 1;
      } else if (reading <= LL_GRID[1]) {
        grid = 2;
      } else if (reading <= LL_GRID[2]) {
        grid = 3;
      } else if (reading <= LL_GRID[3]) {
        grid = 4;
      } else if (reading <= LL_GRID[4]) {
        grid = 5;
      } break;
    default: return -1;
  }
  return grid;
}
//=========================End of Get Grid Codes================================
/////////////////////////////////////////////////////////////////////////END OF SENSORS/////////////////////////////////////////////////////////////////////////////////////////
