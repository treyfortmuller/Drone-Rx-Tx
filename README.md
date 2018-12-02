# Drone-Rx-Tx
Transmission of 2, 2-axis joysticks' data from Arduino Uno (Tx) to Arduino Nano (Rx) with nrf24L01+ modules for control of a small, custom built quadcopter.

* [Video](https://www.youtube.com/watch?v=jWmNdqT4Q9A&t=22s)

* [DevPost](https://devpost.com/software/drone-transmitter)

* [Writeup](https://goo.gl/xdytBq)


### Features
* Transmit.ino takes in two, 2-axis potentiometer values (a total of four values) then utilizes nrf24L01 modules and adapter boards with separate power supplies to transmit 4 channel analog joystick position.
* Receive.ino maps the raw potentiometer values (0-1024) to PWM values in ms (1000-2000). It then generates a PPM signal from the series of PWM values to send to flight controller.
* Piko BLX flight controller runs on the open source flight control firmware BetaFlight. 
* Laser Cut Drone Chassis
* 3D designed and printed Transmitter
* FPV Camera for first person view camera view in real time.
* 3D designed and printed landing gear and camera mount
