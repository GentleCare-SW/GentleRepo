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

#pragma once

#include "peripheral.h"
#include "pressure_sensor.h"
#include "motor_controller.h"

enum class MonitorStatus {
    OK,
    HIGH_PRESSURE,
    HIGH_TORQUE
};

class Monitor: public Peripheral {
public:
    Monitor(const char *status_uuid, PressureSensor *pressure_sensor, MotorController *motor_controller);

    void update(float dt) override;

    float get_status();

private:
    MonitorStatus status;
    PressureSensor *pressure_sensor;
    MotorController *motor_controller;
    int64_t last_update_time;
};