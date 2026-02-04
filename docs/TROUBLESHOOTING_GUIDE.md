# Troubleshooting Guide
This guide is a record of problems we have encountered and how to fix them.

## Motor Errors
Motor error codes:
1. Not responding
   - Common reasons: Motor power or A/B/C connection is bad, tx/rx pin disconnected from arduino
2. Calibration Failed
   - Common reasons: Problem with encoder connection, the shaft has too much resistance
4. Control Error
   - Common reasons: motor started running out of control (spinoout) or too much torque caused the motor to hit the current limit

### Odrive Color Code
Red: Error, odrive is disabled  
Green: Calibrating or running  
Blue: Idle (indicates a problem if it returns to blue after calibration)  


## Using odrivetool in the terminal
List of helpful commands:
- dump_errors(odrv0)
- odrv0.axis0.procedure_result and odrv0.axis0.disarm_reason
- odrv0.clear_errors()
- run_state(odrv0.axis0, AxisState.FULL_CALIBRATION_SEQUENCE)  
  A full list of states can be found [here](https://docs.odriverobotics.com/v/latest/fibre_types/com_odriverobotics_ODrive.html#ODrive.Axis.AxisState)
- odrv0.erase_configuration()
- odrv0.config and odrv0.axis0.config
- odrv0.save_configuration

### Odrive Procedure Result Codes
0. Success
1. Busy
2. Cancelled
3. Disarmed
4. No response
5. Pole pair cpr mismatch
6. Phase resistance out of range
7. Phase inductance out of range
8. Unbalanced phases
9. Invalid motor type
10. Illegal hall state
11. Timeout
12. Homing without end stop
13. Invalid state
14. Not calibrated
15. Not converging 

See [Odrive python package](https://docs.odriverobotics.com/v/latest/guides/python-package.html) for more odrivetool uses.  
See [Odrive API](https://docs.odriverobotics.com/v/latest/fibre_types/com_odriverobotics_ODrive.html#ODrive) for all other details.

## Servo Problems
We had an intermittent problem where running the servo would cause the ESP32 to disconnect. I believe this was because the servo was momentarily drawing too much current from the power supply and the ESP32 didn't have enough power to function. This is why we incorporated a 24V to 5V buck converter into the wedge's circuit which can supply more current. We were previously running all of the electronics from the voltage dimmer, which only supplies 100mA at 5V. There should be at least 1A available to make sure the servo can be powered stably.

## Other Issues
If the remote seems to be working very slowly and the knobs are much less responsive than usual, check to make sure the pressure sensor is connected. If it is disconnected, it will slow down the loop time of the ESP32 significantly. 
