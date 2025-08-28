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
#include <Arduino.h>
#include "peripheral.h"

enum class MotorControllerError {
    NONE,
    NOT_RESPONDING,
    CALIBRATION_FAILED,
    CONTROL_ERROR,
};

class MotorController: public Peripheral {
public:
    MotorController(const char *position_uuid, const char *velocity_uuid, const char *torque_uuid, const char *error_uuid, HardwareSerial *serial, int32_t rx_pin, int32_t tx_pin);

    void start() override;

    void update(float dt) override;

    void mode_changed(VesselMode mode) override;

    void set_velocity(float velocity);

    void set_torque(float torque);

    float get_velocity();

    float get_position();

    float get_torque();

    float get_error();

    bool is_ok();

private:
    HardwareSerial *serial;
    int32_t rx_pin;
    int32_t tx_pin;
    float position;
    float velocity;
    float torque;
    int64_t last_update_time;
    MotorControllerError error;

    void set_error(MotorControllerError error);

    String wait_for_response();

    void write_state(int state);

    int read_state();

    void write_torque(float torque);

    float read_torque();

    void write_velocity(float velocity);

    float read_velocity();

    float read_position();

    int read_error();
};