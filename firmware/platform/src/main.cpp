/*
 * Copyright (c) 2026 GentleCare Corporation. All rights reserved.
 *
 * This source code and the accompanying materials are the confidential and
 * proprietary information of GentleCare Corporation. Unauthorized copying or
 * distribution of this file, via any medium, is strictly prohibited without
 * the prior written permission of GentleCare Corporation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>

#include "service.h"
#include "pressure_sensor.h"
#include "LPS_sensor.h"
#include "motor_controller.h"
#include "can_motor_controller.h"
#include "voltage_dimmer.h"
#include "pressure_controller.h"
#include "servo.h"
#include "valve.h"
#include "rail.h"
#include "wedges_controller.h"
#include "auto_controller.h"
#include "config.h"
#include "common/uuids.h"

TwoWire default_I2C = TwoWire(0);
TwoWire other_I2C = TwoWire(1);

static Service service;
#if SOFTWARE_VERSION == 0
    static PressureSensor pressure_sensor1(PRESSURE_SENSOR_UUID, &default_I2C, 22, 21);
    static MotorController motor_controller(MOTOR_POSITION_UUID, MOTOR_VELOCITY_UUID, MOTOR_TORQUE_UUID, MOTOR_ERROR_UUID, &Serial1, MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN);
    static VoltageDimmer voltage_dimmer1(CENTRAL_DIMMER_UUID, VOLTAGE_DIMMER_PWM_PIN, VOLTAGE_DIMMER_LEDC_CHANNEL);
#elif SOFTWARE_VERSION == 1
    static LpsSensor pressure_sensor1(PRESSURE_SENSOR_UUID, &default_I2C, 22, 21, 0x5C);
    // to be updated with new versions
    static CanMotorController motor_controller(MOTOR_POSITION_UUID, MOTOR_VELOCITY_UUID, MOTOR_TORQUE_UUID, MOTOR_ERROR_UUID, &Serial1);
    static VoltageDimmer voltage_dimmer1(CENTRAL_DIMMER_UUID, VOLTAGE_DIMMER_PWM_PIN, VOLTAGE_DIMMER_LEDC_CHANNEL);
#endif

#if WEDGE_V0
    static Valve valve(VALVE_STATE_UUID, VALVE_DIGITAL_PIN1, VALVE_DIGITAL_PIN2);
    static PressureSensor pressure_sensor2(PRESSURE_SENSOR2_UUID, &other_I2C, PRESSURE_SENSOR_SCL_PIN, PRESSURE_SENSOR_SDA_PIN);
#elif WEDGE_V1
    static LpsSensor pressure_sensor2(PRESSURE_SENSOR_UUID, &default_I2C, PRESSURE_SENSOR_SCL_PIN, PRESSURE_SENSOR_SDA_PIN, 0x5D);
    static Servo valve(SERVO_ANGLE_UUID, SERVO_PWM_PIN, SERVO_LEDC_CHANNEL);
#elif GLIDE_V0 || GLIDE_V1
    static VoltageDimmer voltage_dimmer2(OUTER_DIMMER_UUID, VOLTAGE_DIMMER2_PWM_PIN, VOLTAGE_DIMMER2_LEDC_CHANNEL);
#elif GLIDE_V2
    static VoltageDimmer voltage_dimmer2(OUTER_DIMMER_UUID, VOLTAGE_DIMMER2_PWM_PIN, VOLTAGE_DIMMER2_LEDC_CHANNEL);
    // will have new voltage dimmer and maybe more pressure sensors
#endif

#if PLATFORM_TYPE == 0
    static Servo servo1(SERVO_ANGLE_UUID, SERVO_PWM_PIN, SERVO_LEDC_CHANNEL);
    static Rail rail(JOYSTICK_UUID, LEFT_PIN, RIGHT_PIN);
    static WedgesController wedges_controller(AUTO_CONTROL_MODE_UUID, AUTO_CONTROL_PROGRESS_UUID, TIMER_UUID, &voltage_dimmer1, &motor_controller, &pressure_sensor1, &pressure_sensor2, &servo1, &valve, &rail);
#elif PLATFORM_TYPE == 1
    static PressureController pressure_controller(PRESSURE_CONTROLLER_UUID, &voltage_dimmer1, &pressure_sensor1);
    static AutoController auto_controller(AUTO_CONTROL_MODE_UUID, AUTO_CONTROL_PROGRESS_UUID, &voltage_dimmer1, &voltage_dimmer2, &motor_controller, &pressure_sensor1, &pressure_controller);
#endif


float prev_time = millis();
void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);  

// Max of 16 peripherals
service.add_peripheral(&pressure_sensor1);
service.add_peripheral(&motor_controller);
service.add_peripheral(&voltage_dimmer1);

#if PLATFORM_TYPE == 0
    service.add_peripheral(&servo1);
    service.add_peripheral(&rail);
    service.add_peripheral(&pressure_sensor2);
    service.add_peripheral(&valve);
    service.add_peripheral(&wedges_controller);
#elif PLATFORM_TYPE == 1
    service.add_peripheral(&voltage_dimmer2);
    service.add_peripheral(&auto_controller);
    service.add_peripheral(&pressure_controller);
#endif

service.start();
}

void loop()
{
    service.update();
    pressure_sensor1.set_calibrating(voltage_dimmer1.get_voltage() == 0.0);
    
    #if PLATFORM_TYPE == 0
        //Serial.print(">Pressure 2: ");
        //Serial.println(pressure_sensor2.get_pressure());
        if (voltage_dimmer1.get_voltage() == 0.0) {
            pressure_sensor2.pressure_offset = pressure_sensor1.pressure_offset;
        }
    #endif
   
    // Serial.print(">Pressure 1: ");
    // Serial.println(pressure_sensor1.get_pressure());
    // Serial.print(">Angle: ");
    // Serial.println(servo.get_angle());
    // Serial.print(">Position: ");
    // Serial.println(motor_controller.get_position());
    // Serial.print(">Velocity: ");
    // Serial.println(motor_controller.get_velocity());
    // Serial.print(">Voltage: ");
    // Serial.println(voltage_dimmer1.get_voltage());
    // Serial.print(">Torque: ");
    // Serial.println(motor_controller.get_torque());

}