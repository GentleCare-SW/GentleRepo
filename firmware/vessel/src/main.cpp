/*
 * Copyright (c) 2025 GentleCare Corporation. All rights reserved.
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
#include "vessel.h"
#include "pressure_sensor.h"
#include "motor_controller.h"
#include "voltage_dimmer.h"
#include "pressure_controller.h"
#include "auto_controller.h"
#include "servo.h"
#include "monitor.h"
#include "config.h"
#include "common/uuids.h"

static Vessel vessel;
static PressureSensor pressure_sensor(PRESSURE_SENSOR_UUID, PRESSURE_SENSOR_ERROR_UUID, PRESSURE_SENSOR_ADC_PIN, PRESSURE_SENSOR_CONSTANT);
static MotorController motor_controller(MOTOR_POSITION_UUID, MOTOR_VELOCITY_UUID, MOTOR_TORQUE_UUID, MOTOR_ERROR_UUID, &Serial1, MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN);
static VoltageDimmer voltage_dimmer(VOLTAGE_PERCENTAGE_UUID, VOLTAGE_DIMMER_PWM_PIN, VOLTAGE_DIMMER_LEDC_CHANNEL);
static Servo servo(SERVO_ANGLE_UUID, SERVO_CHAMBER_UUID, SERVO_PWM_PIN, SERVO_LEDC_CHANNEL);
static PressureController pressure_controller(PRESSURE_CONTROLLER_UUID, &voltage_dimmer, &pressure_sensor);
static AutoController auto_controller(AUTO_CONTROL_MODE_UUID, AUTO_CONTROL_PROGRESS_UUID, &voltage_dimmer, &motor_controller, &pressure_sensor, &servo);
static Monitor monitor(MONITOR_STATUS_UUID, &pressure_sensor, &motor_controller);

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    vessel.add_peripheral(&pressure_sensor);
    vessel.add_peripheral(&voltage_dimmer);
#if ENABLE_SERVO
    vessel.add_peripheral(&servo);
#endif
    vessel.add_peripheral(&pressure_controller);
    vessel.add_peripheral(&auto_controller);
    vessel.add_peripheral(&motor_controller);
    vessel.add_peripheral(&monitor);
    vessel.start();
}

void loop()
{
    pressure_sensor.set_calibrating(voltage_dimmer.get_percentage() == 0.0);
    vessel.update();
}