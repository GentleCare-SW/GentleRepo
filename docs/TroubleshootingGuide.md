### Troubleshooting Guide
This guide is intended to be a record of errors we have encountered and how they were fixed.

## Motor Errors
Motor error codes:
1. Not responding
2. Calibration Failed
3. Control Error

## Servo Problems
We had an intermittent problem where running the servo would cause the ESP32 to disconnect. This is probably because the servo was momentarily drawing too much current for the ESP to handle. I don't know what made it start working again. It started working after I unplugged the voltage dimmer pins from the breadboard then put them back (Also, the vacuum started running when I unplugged these). In the future, we might want to incorporate something into the circuit that can limit the current. 
