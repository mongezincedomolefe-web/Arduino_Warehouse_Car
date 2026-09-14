#include <Servo.h>

// ================= PIN MAP (per your wiring) =================
// L298N Motor Driver
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

// IR Line Sensor (single sensor, digital)
const int irPin = 2;

// LOW or HIGH when sitting ON the black line - flip the logic below if needed.

// HC-SR04 Ultrasonic Sensor
const int trigPin = 13;
const int echoPin = 12;

// Servos
Servo liftServo;   // was the "ultrasonic pan" servo - repurposed to raise/lower the gripper arm
Servo gripServo;   // opens/closes the gripper claw
const int liftServoPin = 11;
const int gripServoPin = 10;

// ================= TUNE THESE TO YOUR BUILD =================
const int LIFT_UP     = 30;   // angle when arm is raised
const int LIFT_DOWN   = 120;  // angle when arm is lowered to grab/release
const int GRIP_OPEN   = 40;   // claw open angle
const int GRIP_CLOSE  = 120;  // claw closed angle
const int STATION_TRIGGER_CM = 8;  // how close a station marker must be to trigger a stop

// ================= STATION / BOX LOGIC =================
int stationCount = 0;                 // resets after each full pick+deliver cycle
const int PICKUP_STATION = 1;         // station number = pickup point
const int DELIVERY_STATIONS[] = {2, 3, 4};  // destinations for Box A, B, C in turn
int boxRunNumber = 0;                 // which pick-and-place cycle we're on (0=Box A, 1=Box B, 2=Box C)

// ================= LINE SEARCH MEMORY =================
int lastTurnDirection = 1;  // 1 = try right first, -1 = try left first

void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(irPin, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  liftServo.attach(liftServoPin);
  gripServo.attach(gripServoPin);

  liftServo.write(LIFT_UP);
  gripServo.write(GRIP_OPEN);

  delay(500); // let servos settle before the robot moves
}

// ================= MOTOR PRIMITIVES =================
void motorsForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void motorsStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
void motorsTurnLeft() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void motorsTurnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);
}
void motorsReverse() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

// ================= LINE FOLLOWING (single sensor) =================
// Only one sensor means we can't tell which side the line drifted to.
// Strategy: drive forward while the line is seen; the instant it's lost,
// do a short turn to search for it, alternating direction if needed.
void followLine() {
  int lineState = digitalRead(irPin);

  if (lineState == LOW) {        // LOW = on the black line (verify polarity!)
    motorsForward();
  } else {
    motorsStop();
    delay(50);
    searchForLine();
  }
}

void searchForLine() {
  if (lastTurnDirection == 1) {
    motorsTurnRight();
  } else {
    motorsTurnLeft();
  }
  delay(150);
  motorsStop();

  if (digitalRead(irPin) == LOW) return;  // re-found the line

  lastTurnDirection *= -1;                 // flip and try the other way, a bit further
  if (lastTurnDirection == 1) {
    motorsTurnRight();
  } else {
    motorsTurnLeft();
  }
  delay(300);
  motorsStop();
}

// ================= ULTRASONIC DISTANCE =================
long readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  long distance = duration * 0.0343 / 2;         // convert to cm
  return distance;
}

bool atStationMarker() {
  long d = readDistanceCM();
  return (d > 0 && d < STATION_TRIGGER_CM);
}

// ================= GRIPPER SEQUENCES =================
void pickUpBox() {
  motorsStop();
  Serial.println("Picking up box...");
  liftServo.write(LIFT_DOWN);
  delay(500);
  gripServo.write(GRIP_CLOSE);
  delay(500);
  liftServo.write(LIFT_UP);
  delay(500);
}

void releaseBox() {
  motorsStop();
  Serial.println("Releasing box...");
  liftServo.write(LIFT_DOWN);
  delay(500);
  gripServo.write(GRIP_OPEN);
  delay(500);
  liftServo.write(LIFT_UP);
  delay(500);
}

// ================= STATION HANDLING =================
void handleStation() {
  stationCount++;
  Serial.print("Reached station #");
  Serial.println(stationCount);

  if (stationCount == PICKUP_STATION) {
    delay(300);
    pickUpBox();
  } else {
    int targetStation = DELIVERY_STATIONS[boxRunNumber % 3];
    if (stationCount == targetStation) {
      delay(300);
      releaseBox();
      boxRunNumber++;
      stationCount = 0;   // reset for the next box's cycle
    }
  }

  delay(500);        // clear the marker before resuming
  motorsForward();
  delay(300);
}

// ================= MAIN LOOP =================
void loop() {
  if (atStationMarker()) {
    handleStation();
  } else {
    followLine();
  }
}
