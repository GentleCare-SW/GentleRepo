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
#include "service.h"
#include "pressure_sensor.h"
#include "motor_controller.h"
#include "voltage_dimmer.h"
#include "pressure_controller.h"
#include "auto_controller.h"
#include "servo.h"
#include "control_panel.h"
#include "config.h"
#include "common/uuids.h"

static Service service;
static PressureSensor pressure_sensor(PRESSURE_SENSOR_UUID, PRESSURE_SENSOR_ERROR_UUID, PRESSURE_SENSOR_ADC_PIN, PRESSURE_SENSOR_CONSTANT);
static MotorController motor_controller(MOTOR_POSITION_UUID, MOTOR_VELOCITY_UUID, MOTOR_TORQUE_UUID, MOTOR_ERROR_UUID, &Serial1, MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN);
static VoltageDimmer voltage_dimmer(DIMMER_VOLTAGE_UUID, VOLTAGE_DIMMER_PWM_PIN, VOLTAGE_DIMMER_LEDC_CHANNEL);
static Servo servo(SERVO_ANGLE_UUID, SERVO_CHAMBER_UUID, SERVO_PWM_PIN, SERVO_LEDC_CHANNEL);
static PressureController pressure_controller(PRESSURE_CONTROLLER_UUID, &voltage_dimmer, &pressure_sensor);
static AutoController auto_controller(AUTO_CONTROL_MODE_UUID, AUTO_CONTROL_PROGRESS_UUID, &voltage_dimmer, &motor_controller, &pressure_sensor, &servo);
static ControlPanel control_panel(CONTROL_PANEL_STOP_PIN, CONTROL_PANEL_PAUSE_PIN, CONTROL_PANEL_INVERT_PIN, CONTROL_PANEL_EVERT_PIN, CONTROL_PANEL_CHAMBER_PIN, &auto_controller, &motor_controller, &pressure_sensor, &voltage_dimmer, &servo);

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

#if ENABLE_CONTROL_PANEL
    service.add_peripheral(&control_panel);
#endif
    service.add_peripheral(&pressure_sensor);
    service.add_peripheral(&voltage_dimmer);
#if ENABLE_SERVO
    service.add_peripheral(&servo);
#endif
    service.add_peripheral(&pressure_controller);
    service.add_peripheral(&auto_controller);
#if ENABLE_MOTOR_CONTROLLER
    service.add_peripheral(&motor_controller);
#endif
    service.start();
}

void loop()
{
    pressure_sensor.set_calibrating(voltage_dimmer.get_voltage() == 0.0);
    service.update();
}