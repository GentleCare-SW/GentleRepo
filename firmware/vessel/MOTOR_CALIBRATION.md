ODRIVE v3.6:

```python
odrv0.config.enable_brake_resistor = True
odrv0.config.brake_resistance = 2.0
odrv0.axis0.controller.config.vel_limit = 200.0
odrv0.axis0.motor.config.current_lim = 6.0
odrv0.axis0.motor.config.pole_pairs = 12
odrv0.axis0.motor.config.torque_constant = 8.27 / 135.0
odrv0.axis0.motor.config.resistance_calib_max_voltage = 10.0
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

***

ODRIVE S1:

```python
odrv0.config.dc_bus_overvoltage_trip_level = 30.0
odrv0.config.dc_max_positive_current = 15.0
odrv0.config.brake_resistor0.resistance = 2.0
odrv0.config.brake_resistor0.enable = True
odrv0.axis0.config.motor.motor_type = MotorType.PMSM_CURRENT_CONTROL
odrv0.axis0.config.motor.pole_pairs = 12
odrv0.axis0.config.motor.torque_constant = 8.27 / 135.0
odrv0.axis0.config.motor.calibration_current = 3.0
odrv0.axis0.config.calibration_lockin.current = 3.0
odrv0.axis0.config.motor.current_soft_max = 6.0
odrv0.axis0.config.motor.current_hard_max = 10.0
odrv0.axis0.controller.config.vel_limit = 20.0
odrv0.inc_encoder0.config.cpr = 25600
odrv0.inc_encoder0.config.enabled = True
odrv0.config.enable_uart_a = True
odrv0.config.gpio7_mode = GpioMode.UART_A
odrv0.config.gpio6_mode = GpioMode.UART_A
odrv0.save_configuration()
```

```python
odrv0.axis0.requested_state = AxisState.FULL_CALIBRATION_SEQUENCE
odrv0.axis0.requested_state = AxisState.CLOSED_LOOP_CONTROL
```