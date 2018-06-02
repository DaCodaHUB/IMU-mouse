/*
   To do: Scaling (May need to split x and y axis)
   (Move mouse using coordinates)
   Lighter Cable
   Calibrate maximum angles

   Flat: (0.33, 0.24, 1.41)g (1g = 9.8 m/s²)

   aref: 3.3V (3V?)
*/

#include <ADXL335.h>
#include <Mouse.h>
//#include <MouseTo.h>

#define ELEMENTS 3
#define GRAVITY 1
#define OFFSET 10
#define DATAPOINTS 50
#define RESTCOUNT 250
#define MOUSERATE 25

const int ledPin = 16;
const int leftButtonPin = 14;
const int resetButton = 8;
const int pin_x = A3;
const int pin_y = A2;
const int pin_z = A1;
const float aref = 3.3;
ADXL335 accel(pin_x, pin_y, pin_z, aref);

// Smoothing
const int numReadings = 10;
int readings[numReadings * ELEMENTS];      // the readings from the analog input
int readIndex[ELEMENTS];              // the index of the current reading
int total[ELEMENTS];                  // the running total
int average[ELEMENTS];                // the average

float accelXYZ[3];
float accelZero[3];
float sum[3];
int roll = 0;
int pitch = 0;
int lastRoll = 0;
int lastPitch = 0;
int counter = 0;
int freezeCounter = 0;
boolean calculate = false;

const short debugDelay = 10;
const short responseDelay = 5;

const float Scale = 2;
const short degreeOffset = 8;
const short calOffset = 0;
boolean clicked = false;
boolean lastClick = false;
boolean resting = false;
short calibratingSwitch = 5;

int rightMost = 0;
int leftMost = 0;
int forwardMost = 0;
int backwardMost = 0;

void mouseControl (int roll, int pitch);

void setup()
{
  Serial.begin(9600);

  //while (!Serial) {}

  for (int i = 0; i < (numReadings * ELEMENTS); i++) {
    readings[i] = 0;
  }
  for (int i = 0; i < ELEMENTS; i++) {
    readIndex[i] = 0;
    total[i] = 0;
    average[i] = 0;
    accelXYZ[i] = 0;
    accelZero[i] = 0;
    sum[i] = 0;
  }
  pinMode(leftButtonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Mouse.begin();
}

void loop()
{
  //this is required to update the values
  int val1 = Smoothing (0, analogRead(pin_x) * 5 / 3.3);
  int val2 = Smoothing (1, analogRead(pin_y) * 5 / 3.3);
  int val3 = Smoothing (2, analogRead(pin_z) * 5 / 3.3);
  accel.update(val1, val2, val3);
  counter += 1;
  /*
    Serial.print(val1);
    Serial.print(",\t");
    Serial.print(val2);
    Serial.print(",\t");
    Serial.println(val3);
  */
  accelXYZ[0] = accel.getX();
  accelXYZ[1] = accel.getY();
  accelXYZ[2] = accel.getZ();
  /*
    float rho = accel.getRho();
    float phi = accel.getPhi();
    float theta = accel.getTheta();
  */

  if (calibratingSwitch != 0) {
    if (calibratingSwitch == 5) {
      if (counter <= (DATAPOINTS + OFFSET) ) {
        if (counter > OFFSET) {
          if (counter != (DATAPOINTS + OFFSET)) {
            sum[0] += accelXYZ[0];
            sum[1] += accelXYZ[1];
            sum[2] += accelXYZ[2];
          } else {
            accelZero[0] = sum[0] / DATAPOINTS;
            accelZero[1] = sum[1] / DATAPOINTS;
            accelZero[2] = (sum[2] / DATAPOINTS) - 1;
            digitalWrite (ledPin, HIGH);
            delay(1000);
            calibratingSwitch = 4;
            Serial.println("running");
          }
        }
        /*
        Serial.print(accelXYZ[0]);
        Serial.print(",\t");
        Serial.print(accelXYZ[1]);
        Serial.print(",\t");
        Serial.println(accelXYZ[2]);
        */
      }
      //Serial.println("running");
    } else {
      accelXYZ[0] = (accelXYZ[0] - accelZero[0]) * GRAVITY;
      accelXYZ[1] = (accelXYZ[1] - accelZero[1]) * GRAVITY;
      accelXYZ[2] = (accelXYZ[2] - accelZero[2]) * GRAVITY;
      RP_calculate(accelXYZ[0], accelXYZ[1], accelXYZ[2]);

      digitalWrite (ledPin, LOW);
      if (digitalRead(leftButtonPin) == HIGH) {
        clicked = true;
      } else {
        clicked = false;
      }

      switch (calibratingSwitch) {
        case 4:
          if (clicked == true && lastClick == false) {
            rightMost = -roll;
            calibratingSwitch = 3;
          }
          break;
        case 3:
          if (clicked == true && lastClick == false) {
            leftMost = roll;
            calibratingSwitch = 2;
          }
          break;
        case 2:
          if (clicked == true && lastClick == false) {
            forwardMost = -pitch;
            calibratingSwitch = 1;
          }
          break;
        case 1:
          if (clicked == true && lastClick == false) {
            backwardMost = pitch;
            calibratingSwitch = 0;
          }
          break;
        default: break;
      }
      lastClick = clicked;
      Serial.print(roll);
      Serial.print(",\t");
      Serial.println(pitch);
      Serial.print(rightMost);
      Serial.print(",\t");
      Serial.print(leftMost);
      Serial.print(",\t");
      Serial.print(forwardMost);
      Serial.print(",\t");
      Serial.println(backwardMost);
    }
  } else {
    accelXYZ[0] = (accelXYZ[0] - accelZero[0]) * GRAVITY;
    accelXYZ[1] = (accelXYZ[1] - accelZero[1]) * GRAVITY;
    accelXYZ[2] = (accelXYZ[2] - accelZero[2]) * GRAVITY;
    RP_calculate(accelXYZ[0], accelXYZ[1], accelXYZ[2]);
    /*
      Serial.print(accelXYZ[0]);
      Serial.print(",\t");
      Serial.print(accelXYZ[1]);
      Serial.print(",\t");
      Serial.print(accelXYZ[2]);
      Serial.print(",\t");
    */

    if (roll >= (lastRoll - 1) && roll <= (lastRoll + 1) && pitch >= (lastPitch - 1) && pitch <= (lastPitch + 1)) {
      freezeCounter += 1;
      //Serial.println(freezeCounter);
    } else {
      freezeCounter = 0;
    }

    if (freezeCounter > RESTCOUNT) {
      resting = true;
      freezeCounter = 300;
      digitalWrite (ledPin, HIGH);
      delay(500);
    } else {
      resting = false;
    }

    Serial.print(roll);
    Serial.print(",\t");
    Serial.println(pitch);
    if (resting == false) mouseControl (roll, pitch);
    lastRoll = roll;
    lastPitch = pitch;
    counter = 100;
  }

  delay(debugDelay);
}

//this function was taken from my format float library
String formatFloat(double value, int places, int* string_width)
{
  //if value is positive infinity
  if (isinf(value) > 0)
  {
    return "+Inf";
  }

  //Arduino does not seem to have negative infinity
  //keeping this code block for reference
  //if value is negative infinity
  if (isinf(value) < 0)
  {
    return "-Inf";
  }

  //if value is not a number
  if (isnan(value) > 0)
  {
    return "NaN";
  }

  //always include a space for the dot
  int num_width = 1;

  //if the number of decimal places is less than 1
  if (places < 1)
  {
    //set places to 1
    places = 1;

    //and truncate the value
    value = (float)((int)value);
  }

  //add the places to the right of the decimal
  num_width += places;

  //if the value does not contain an integral part
  if (value < 1.0 && value > -1.0)
  {
    //add one for the integral zero
    num_width++;
  }
  else
  {

    //get the integral part and
    //get the number of places to the left of decimal
    num_width += ((int)log10(abs(value))) + 1;
  }
  //if the value in less than 0
  if (value < 0.0)
  {
    //add a space for the minus sign
    num_width++;
  }

  //make a string the size of the number
  //plus 1 for string terminator
  char s[num_width + 1];

  //put the string terminator at the end
  s[num_width] = '\0';


  //initalize the array to all zeros
  for (int i = 0; i < num_width; i++)
  {
    s[i] = '0';
  }

  //characters that are not changed by
  //the function below will be zeros

  //set the out variable string width
  //lets the caller know what we came up with
  *string_width = num_width;

  //use the avr-libc function dtosrtf to format the value
  return String(dtostrf(value, num_width, places, s));
}

int Smoothing (int eleNum, int data) {
  int readingIndex = (eleNum * numReadings) + readIndex[eleNum];
  // subtract the last reading:
  total[eleNum] = total[eleNum] - readings[readingIndex];
  // read from the sensor:
  readings[readingIndex] = data;
  // add the reading to the total:
  total[eleNum] = total[eleNum] + readings[readingIndex];
  // advance to the next position in the array:
  readIndex[eleNum] = readIndex[eleNum] + 1;

  // if we're at the end of the array...
  if (readIndex[eleNum] >= numReadings) {
    // ...wrap around to the beginning:
    readIndex[eleNum] = 0;
  }

  // calculate the average:
  average[eleNum] = total[eleNum] / numReadings;
  return average[eleNum];
}

void RP_calculate(float x, float y, float z) {
  double x_Buff = float(x);
  double y_Buff = float(y);
  double z_Buff = float(z);
  roll = atan2(y_Buff , z_Buff) * 57.3;
  pitch = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * 57.3;
}

void mouseControl (int roll, int pitch) {
  if (digitalRead(leftButtonPin) == HIGH) {
    if (clicked == false) {
      Mouse.click();
      delay(300);
      clicked = true;
    } else {
      Mouse.press();
    }
  } else {
    if (clicked == true) {
      Mouse.release();
    }
  }

  if (pitch > degreeOffset && roll > -(degreeOffset) && roll < degreeOffset) {                   // Move up
    Mouse.move(0, (MOUSERATE * pitch / forwardMost - calOffset) / Scale, 0);
    delay(responseDelay);
  } else if (pitch < -(degreeOffset) && roll > -(degreeOffset) && roll < degreeOffset) {         // Move down
    Mouse.move(0, (MOUSERATE * pitch / backwardMost + calOffset) / Scale, 0);
    delay(responseDelay);
  } else if (pitch > degreeOffset && roll < -(degreeOffset)) {                                   // Move up + right
    Mouse.move(-(MOUSERATE * roll / rightMost + calOffset) / Scale, (MOUSERATE * pitch / forwardMost - calOffset) / Scale, 0);
    delay(responseDelay);
  } else if (pitch > degreeOffset && roll > degreeOffset) {                                      // Move up + left
    Mouse.move(-(MOUSERATE * roll / leftMost - calOffset) / Scale, (MOUSERATE * pitch / forwardMost - calOffset) / Scale, 0);
    delay(responseDelay);
  } else if (roll < -(degreeOffset) && pitch > -(degreeOffset) && pitch < degreeOffset) {        // Move right
    Mouse.move(-(MOUSERATE * roll / rightMost + calOffset) / Scale, 0, 0);
    delay(responseDelay);
  } else if (roll > degreeOffset && pitch > -(degreeOffset) && pitch < degreeOffset) {           // Move left
    Mouse.move(-(MOUSERATE * roll / leftMost - calOffset) / Scale, 0, 0);
    delay(responseDelay);
  } else if (roll < -(degreeOffset) && pitch < -(degreeOffset)) {                                // Move down + right
    Mouse.move(-(MOUSERATE * roll / rightMost + calOffset) / Scale, (MOUSERATE * pitch / backwardMost + calOffset) / Scale, 0);
    delay(responseDelay);
  } else if (roll > degreeOffset && pitch < -(degreeOffset)) {                                   // Move down + left
    Mouse.move(-(MOUSERATE * roll / leftMost - calOffset) / Scale, (MOUSERATE * pitch / backwardMost + calOffset) / Scale, 0);
    delay(responseDelay);
  }
}

