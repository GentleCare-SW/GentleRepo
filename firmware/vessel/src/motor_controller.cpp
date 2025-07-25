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
#include "motor_controller.h"

typedef enum AxisState {
    AXIS_STATE_IDLE = 1,
    AXIS_STATE_FULL_CALIBRATION_SEQUENCE = 3,
    AXIS_STATE_CLOSED_LOOP_CONTROL = 8,
} axis_state_t;

static HardwareSerial *odrive_serial;

static void set_state(axis_state_t state)
{
    odrive_serial->printf("w axis0.requested_state %i\n", state);
}

static axis_state_t get_state()
{
    odrive_serial->printf("r axis0.current_state\n");
    return (axis_state_t)odrive_serial->readStringUntil('\n').toInt();
}

bool motor_controller_initialize(HardwareSerial *serial, int32_t rx_pin, int32_t tx_pin)
{
    odrive_serial = serial;
    odrive_serial->begin(BAUD_RATE, SERIAL_8N1, rx_pin, tx_pin);
    while (!*odrive_serial);

    set_state(AXIS_STATE_FULL_CALIBRATION_SEQUENCE);
    do {
        delay(100);
    } while (get_state() != AXIS_STATE_IDLE);

    set_state(AXIS_STATE_CLOSED_LOOP_CONTROL);
    delay(100);
    if (get_state() != AXIS_STATE_CLOSED_LOOP_CONTROL)
        return false;

    motor_controller_set_velocity(0.0);
    return true;
}

void motor_controller_set_velocity(float velocity)
{
    odrive_serial->printf("v 0 %f\n", velocity);
}

void motor_controller_set_torque(float torque)
{
    odrive_serial->printf("c 0 %f\n", torque / GEARBOX_RATIO);
}

float motor_controller_get_velocity()
{
    odrive_serial->printf("r axis0.vel_estimate\n");
    return odrive_serial->readStringUntil('\n').toFloat();
}

float motor_controller_get_position()
{
    odrive_serial->printf("r axis0.pos_estimate\n");
    return odrive_serial->readStringUntil('\n').toFloat();
}

float motor_controller_get_torque()
{
    odrive_serial->printf("r axis0.motor.foc.Iq_setpoint\n");
    return odrive_serial->readStringUntil('\n').toFloat() * TORQUE_CONSTANT * GEARBOX_RATIO;
}