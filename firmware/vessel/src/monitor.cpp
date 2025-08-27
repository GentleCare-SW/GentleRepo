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

#include "monitor.h"

static const float UPDATE_INTERVAL = 0.05;
static const float PRESSURE_LIMIT = 4.0;
static const float TORQUE_LIMIT = 7.0;

Monitor::Monitor(const char *status_uuid, PressureSensor *pressure_sensor, MotorController *motor_controller)
{
    this->status = MonitorStatus::OK;
    this->pressure_sensor = pressure_sensor;
    this->motor_controller = motor_controller;
    this->last_update_time = 0;

    this->add_characteristic(status_uuid, nullptr, std::bind(&Monitor::get_status, this));
}

void Monitor::update(float dt)
{
    if (millis() - this->last_update_time > UPDATE_INTERVAL * 1000) {
        this->last_update_time = millis();

        float pressure = this->pressure_sensor->get_pressure();
        float torque = this->motor_controller->get_torque();

        MonitorStatus status = MonitorStatus::OK;

        if (torque > TORQUE_LIMIT)
            status = MonitorStatus::HIGH_TORQUE;
        else if (this->pressure_sensor->is_connected() && pressure > PRESSURE_LIMIT)
            status = MonitorStatus::HIGH_PRESSURE;

        this->status = status;
    }
}

float Monitor::get_status()
{
    return (float)this->status;
}