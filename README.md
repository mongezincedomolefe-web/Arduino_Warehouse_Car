# Arduino Warehouse Car — Autonomous Pick-and-Place Robot

An autonomous line-following robot built on an Arduino Uno that simulates a warehouse pick-and-place system: it follows a taped path, detects numbered "stations" with an ultrasonic sensor, and uses a servo-controlled gripper to pick up a box at one station and deliver it to another.

## How It Works

- **Line following** — a single digital IR sensor detects the black line. Because there's only one sensor, the robot can't tell which side it's drifted to, so the logic is simple: see the line → drive forward; lose the line → stop and search (turn one way, check, then sweep the other way if still lost).
- **Station detection** — an HC-SR04 ultrasonic sensor detects marker points along the route. Each detected station increments a counter.
- **Pick-and-place** — station 1 is always the pickup point. A servo-driven lift arm lowers, a second servo closes the gripper claw, then the arm raises. At the box's designated delivery station, the sequence reverses to release it.
- **Multi-box cycling** — after each delivery, the robot resets and starts the cycle again for the next box, each with its own target delivery station.

## Hardware

| Component | Purpose |
|---|---|
| Arduino Uno R3 | Main controller |
| L298N Motor Driver | Drives the two DC gear motors |
| 2x TT DC Gear Motors + Wheels | Differential drive |
| IR Line Sensor (digital) | Line detection |
| HC-SR04 Ultrasonic Sensor | Station/marker detection |
| 2x SG90 Micro Servos | Gripper lift + claw |
| Robot chassis + caster wheel, battery pack | Frame & power |

## Pin Map

| Function | Pin |
|---|---|
| Motor driver IN1–IN4 | 4, 5, 6, 7 |
| IR line sensor | 2 |
| Ultrasonic trig / echo | 13 / 12 |
| Lift servo | 11 |
| Grip servo | 10 |

## Setup

1. Open `ELB1502_pick_and_place_v1_commented.ino` in the Arduino IDE.
2. Wire the components according to the pin map above.
3. Upload the sketch, then open the Serial Monitor (9600 baud).
4. Hold the IR sensor over plain background vs. the black line and check the printed value — set `LINE_DETECTED_LOW` in the code to match your specific sensor module's polarity.
5. Tune `LIFT_UP`, `LIFT_DOWN`, `GRIP_OPEN`, `GRIP_CLOSE`, and `STATION_TRIGGER_CM` to your physical build.

## Status

Actively in development — currently working on integrating and calibrating the IR line sensor for reliable path tracking.

## Notes
build, wiring, and code are my own.
