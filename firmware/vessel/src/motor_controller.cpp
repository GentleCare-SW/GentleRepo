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
#include <ODriveArduino.h>
#include "motor_controller.h"

static ODriveArduino odrive(Serial1);

bool motor_controller_initialize(int32_t rx_pin, int32_t tx_pin)
{
    delay(1000);

    Serial1.begin(BAUD_RATE, SERIAL_8N1, rx_pin, tx_pin);
    while (!Serial1);

    delay(1000);

    if (!odrive.runState(ODRIVE_AXIS, AXIS_STATE_ENCODER_INDEX_SEARCH, true, 1.0))
        return true;

    odrive.runState(ODRIVE_AXIS, AXIS_STATE_CLOSED_LOOP_CONTROL, false);

    delay(1000);

    odrive.setVelocity(ODRIVE_AXIS, 0.0);
    return true;
}

void motor_controller_set_velocity(float velocity)
{
    odrive.setVelocity(ODRIVE_AXIS, velocity);
}

float motor_controller_get_velocity()
{
    return odrive.getVelocity(ODRIVE_AXIS);
}

float motor_controller_get_position()
{
    return odrive.getPosition(ODRIVE_AXIS);
}

float motor_controller_get_current()
{
    Serial1.printf("r axis%i.motor.current_control.Iq_setpoint\n", ODRIVE_AXIS);
    return odrive.readFloat();
}