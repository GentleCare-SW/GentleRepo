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
#include "voltage_dimmer.h"
#include "motor_controller.h"
#include "pressure_sensor.h"
#include "servo.h"
#include "tension_controller.h"
#include "valve.h"
#include "auto_controller.h"
#include "steering.h"


class WedgesController: public Peripheral {
public:
    WedgesController(const char *mode_uuid, const char *progress_uuid, const char *timer_uuid, VoltageDimmer *dimmer, MotorController *motor, PressureSensor *pressure_sensor1, PressureSensor *pressure_sensor2, Servo *servo, Valve *valve, Steering *rail);

    void update(float dt) override;

    void mode_changed(ServiceMode mode) override;

    void set_mode(float mode);

    float get_mode();

    void auto_eversion(float progress);

    void auto_inversion(float progress);

    float get_progress();

    long get_time();

    void toggle_paused();

private:
    VoltageDimmer *dimmer;
    MotorController *motor;
    Valve *valve;
    PressureSensor *pressure_sensor1;
    PressureSensor *pressure_sensor2;
    Servo *servo;
    Steering *rail;
    AutoControlMode mode;
    TensionController tension_controller;
    int32_t rail_pin;
    int32_t to_rail_pin;
    bool timer_active;
    unsigned long timer_start;
};