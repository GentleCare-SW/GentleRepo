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


#pragma once
#include <ACAN2517FD.h>
#include <MoteusAcan2517fd.h>
#include "motor_controller.h"


class CanMotorController: public MotorController {
public:
    CanMotorController(const char *position_uuid, const char *velocity_uuid, const char *torque_uuid, const char *error_uuid, HardwareSerial *serial);

    void start() override;

    void update(float dt) override;

    void mode_changed(ServiceMode mode) override;

    void set_velocity(float velocity);

    // void set_error(float error);

    // float get_velocity();

    // float get_position();

    // float get_torque();

    // float get_error();

    // bool is_ok();

private:
    // The ACAN2517FD driver and the Moteus controller live as file-scope objects
    // in can_motor_controller.cpp (single CAN motor), mirroring the reference
    // sketch. The ISR passed to can.begin() must be a bare function pointer, and a
    // captureless lambda can only reference a file-scope `can`, not a member.
    uint16_t loopCount = 0;
    float cmd_position;
    float cmd_velocity;
    void send_command();
    void moteus_query(const Moteus::Query::Result& query);
};