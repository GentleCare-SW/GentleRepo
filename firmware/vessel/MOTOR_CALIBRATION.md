53-gear motor config:

```python
odrv0.config.dc_bus_overvoltage_trip_level = 30.0
odrv0.config.dc_max_positive_current = 15.0
odrv0.config.brake_resistor0.resistance = 2.0
odrv0.config.brake_resistor0.enable = True
odrv0.config.brake_resistor0.enable_dc_bus_voltage_feedback = True
odrv0.config.brake_resistor0.dc_bus_voltage_feedback_ramp_start = 26.0
odrv0.config.brake_resistor0.dc_bus_voltage_feedback_ramp_end = 30.0
odrv0.axis0.config.motor.motor_type = MotorType.PMSM_CURRENT_CONTROL
odrv0.axis0.config.motor.pole_pairs = 12
odrv0.axis0.config.motor.torque_constant = 3.975
odrv0.axis0.config.motor.calibration_current = 3.0
odrv0.axis0.config.calibration_lockin.current = 3.0
odrv0.axis0.config.motor.current_soft_max = 6.0
odrv0.axis0.config.motor.current_hard_max = 9.0
odrv0.axis0.controller.config.vel_limit = 20.0
odrv0.axis0.controller.config.vel_gain = 4.0
odrv0.axis0.controller.config.vel_integrator_gain = 8.0
odrv0.inc_encoder0.config.cpr = 25600
odrv0.inc_encoder0.config.enabled = True
odrv0.config.enable_uart_a = True
odrv0.config.gpio7_mode = GpioMode.UART_A
odrv0.config.gpio6_mode = GpioMode.UART_A
odrv0.save_configuration()
```

***

81-gear motor config:

```python
odrv0.config.dc_bus_overvoltage_trip_level = 54.0
odrv0.config.dc_max_positive_current = 15.0
odrv0.config.brake_resistor0.resistance = 2.0
odrv0.config.brake_resistor0.enable = True
odrv0.config.brake_resistor0.enable_dc_bus_voltage_feedback = True
odrv0.config.brake_resistor0.dc_bus_voltage_feedback_ramp_start = 50.0
odrv0.config.brake_resistor0.dc_bus_voltage_feedback_ramp_end = 54.0
odrv0.axis0.config.motor.motor_type = MotorType.PMSM_CURRENT_CONTROL
odrv0.axis0.config.motor.pole_pairs = 8
odrv0.axis0.config.motor.torque_constant = 7.4439
odrv0.axis0.config.motor.calibration_current = 2.0
odrv0.axis0.config.calibration_lockin.current = 2.0
odrv0.axis0.config.motor.current_soft_max = 4.0
odrv0.axis0.config.motor.current_hard_max = 6.0
odrv0.axis0.controller.config.vel_limit = 30.0
odrv0.axis0.controller.config.vel_gain = 1.0
odrv0.axis0.controller.config.vel_integrator_gain = 2.0
odrv0.inc_encoder0.config.cpr = 4096
odrv0.inc_encoder0.config.enabled = True
odrv0.config.enable_uart_a = True
odrv0.config.gpio7_mode = GpioMode.UART_A
odrv0.config.gpio6_mode = GpioMode.UART_A
odrv0.save_configuration()
```