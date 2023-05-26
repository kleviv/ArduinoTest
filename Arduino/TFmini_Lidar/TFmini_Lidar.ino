#include <SoftwareSerial.h>
#include <Servo.h>
#include <TFMPlus.h>

/********  Lidar  ********/
SoftwareSerial Serial1(2, 3);  // pin 2 and 3
// const int HEADER = 0x59;
// int uart[9];
// int distance;
// int strength;
// int check;


TFMPlus lidar;
int16_t distance = 0;
int16_t flux = 0;
int16_t temp = 0;

float avgDistance[4];
int average_distance;

/********  Servo  ********/
Servo servo;
int position;

/****** Haptic motor *****/
const int motorPin = 5;
int inputMin = 0;
int inputMax = 100;
int outputMin = 127;
int outputMax = 50;
int threshold = 100;

bool static_test = true;

void setup() {

  /********  Lidar  ********/
  Serial.begin(115200);
  delay(20);
  Serial1.begin(115200);
  delay(20);
  lidar.begin(&Serial1);

  /********  Servo  ********/
  //servo.attach(9);

  /****** Haptic motor *****/
  //pinMode(motorPin, OUTPUT);
}


void loop() {

  if (static_test) {
    position = 90;
    //servo.write(position);

    /********  Lidar  ********/
    for (int i = 0; i<4; i++) {
      delay(10);
      lidar.getData(distance, flux, temp);
      avgDistance[i] = distance;
    }

    average_distance = (avgDistance[0]+avgDistance[1]+avgDistance[2]+avgDistance[3])/4; 
    Serial.print(average_distance);
    Serial.print(",");
    Serial.print(position);
    Serial.print(",");

    // vibrate when the distance is less than the threshold
    // if (distance < threshold) {
    //   hapticMotor(distance);
    // } else if (distance > threshold) {
    //   analogWrite(motorPin, LOW);
    // }
  }
  /********  Static  ********/
  else {

    for (position = 0; position <= 180; position += 1) {
      lidar.getData(distance, flux, temp);
      servo.write(position);

      Serial.print(distance);
      Serial.print(",");
      Serial.print(position);
      Serial.print(",");

      // if (distance < threshold) {
      //   hapticMotor(distance);
      // } else if (distance > threshold) {
      //   analogWrite(motorPin, LOW);
      // }
      delay(50);
    }

    for (position = 180; position >= 0; position -= 1) {
      lidar.getData(distance, flux, temp);
      servo.write(position);
      Serial.print(distance);
      Serial.print(",");
      Serial.print(position);
      Serial.print(",");

      // if (distance < threshold) {
      //   hapticMotor(distance);
      // } else if (distance > threshold) {
      //   analogWrite(motorPin, LOW);
      // }
      delay(50);
    }
  }
}

void hapticMotor(int inputValue) {
  // use map function to change the intensity of the vibration depending on the distance
  // range from 100cm - 0cm
  int mappedValue = map(inputValue, inputMin, inputMax, outputMin, outputMax);
  analogWrite(motorPin, mappedValue);
  //delay(10);
}



// int getLidarData(SoftwareSerial Serial1){
// if (Serial1.available()) {

//   if (Serial1.read() == HEADER) {

//     uart[0] = HEADER;

//     if (Serial1.read() == HEADER) {
//       uart[1] = HEADER;

//       for (int i = 2; i < 9; i++) {
//         uart[i] = Serial1.read();
//       }
//       check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
//       if (uart[8] == (check&0xff)) {
//         distance = uart[2] + uart[3] * 256;
//         strength = uart[4] + uart[5] * 256;

//         Serial.print(distance);
//         Serial.print("\n");

//         return distance;
//         delay(300);
//       }
//     }
//   }
// }

// }
