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

enum class AutoControlMode {
    IDLE,
    EVERSION,
    EVERSION_PAUSED,
    INVERSION,
    INVERSION_PAUSED,
};

class AutoController: public Peripheral {
public:
    AutoController(const char *mode_uuid, const char *progress_uuid, VoltageDimmer *dimmer, MotorController *motor, PressureSensor *pressure_sensor, Servo *servo);

    void update(float dt) override;

    void mode_changed(VesselMode mode) override;

    void set_mode(float mode);

    float get_mode();

    float get_progress();

private:
    VoltageDimmer *dimmer;
    MotorController *motor;
    PressureSensor *pressure_sensor;
    Servo *servo;
    AutoControlMode mode;
    TensionController tension_controller;
};