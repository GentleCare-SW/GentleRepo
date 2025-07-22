
```python
odrv0.config.enable_brake_resistor = True
odrv0.config.brake_resistance = 2.0
odrv0.axis0.controller.config.vel_limit = 200.0
odrv0.axis0.motor.config.current_lim = 6.0
odrv0.axis0.motor.config.pole_pairs = 12
odrv0.axis0.motor.config.torque_constant = 8.27 / 135.0
odrv0.axis0.motor.config.resistance_calib_max_voltage = 6.0
odrv0.axis0.motor.config.calibration_current = 1.0
odrv0.axis0.encoder.config.cpr = 25600
odrv0.axis0.encoder.config.use_index = True
odrv0.save_configuration()
```

```python
odrv0.axis0.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE
odrv0.axis0.encoder.config.pre_calibrated = True
odrv0.axis0.motor.config.pre_calibrated = True
odrv0.save_configuration()
```

```python
odrv0.axis0.requested_state = AXIS_STATE_ENCODER_INDEX_SEARCH
odrv0.axis0.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL
```