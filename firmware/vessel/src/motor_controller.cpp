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

enum class AxisState {
    IDLE = 1,
    FULL_CALIBRATION_SEQUENCE = 3,
    CLOSED_LOOP_CONTROL = 8,
};

static void set_state(HardwareSerial *serial, AxisState state)
{
    serial->printf("w axis0.requested_state %i\n", state);
}

static AxisState get_state(HardwareSerial *serial)
{
    serial->printf("r axis0.current_state\n");
    return (AxisState)serial->readStringUntil('\n').toInt();
}

MotorController::MotorController(const char *position_uuid, const char *velocity_uuid, const char *torque_uuid, HardwareSerial *serial, int32_t rx_pin, int32_t tx_pin)
{
    this->serial = serial;
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->position = 0.0;
    this->velocity = 0.0;
    this->torque = 0.0;
    this->add_characteristic(position_uuid, nullptr, std::bind(&MotorController::get_position, this));
    this->add_characteristic(velocity_uuid, std::bind(&MotorController::set_velocity, this, std::placeholders::_1), std::bind(&MotorController::get_velocity, this));
    this->add_characteristic(torque_uuid, std::bind(&MotorController::set_torque, this, std::placeholders::_1), std::bind(&MotorController::get_torque, this));
}

void MotorController::start()
{
    delay(2000);

    this->serial->begin(BAUD_RATE, SERIAL_8N1, this->rx_pin, this->tx_pin);
    while (!this->serial);
    
    set_state(this->serial, AxisState::FULL_CALIBRATION_SEQUENCE);
    do {
        delay(100);
    } while (get_state(this->serial) != AxisState::IDLE);

    set_state(this->serial, AxisState::CLOSED_LOOP_CONTROL);
    delay(100);
    if (get_state(this->serial) != AxisState::CLOSED_LOOP_CONTROL) {
        Serial.printf("Failed to set motor controller to CLOSED_LOOP_CONTROL state\n");
        return;
    }

    this->set_velocity(0.0);
    this->last_update_time = micros();
}

void MotorController::update(float dt)
{
    int64_t current_time = micros();
    if (current_time - this->last_update_time > MOTOR_UPDATE_INTERVAL * 1e6) {
        dt = (current_time - this->last_update_time) / 1e6;
        this->last_update_time = current_time;

        this->serial->printf("r axis0.encoder.pos_estimate\n");
        this->position = this->serial->readStringUntil('\n').toFloat() * -TWO_PI / GEARBOX_RATIO;
        this->serial->printf("r axis0.vel_estimate\n");
        this->velocity = this->serial->readStringUntil('\n').toFloat() * -TWO_PI / GEARBOX_RATIO;
        this->serial->printf("r axis0.motor.foc.Iq_setpoint\n");
        float torque = this->serial->readStringUntil('\n').toFloat() * TORQUE_CONSTANT * -GEARBOX_RATIO;

        float alpha = exp(-6.0 * dt);
        this->torque = (1.0 - alpha) * torque + alpha * this->torque;
    }
}

void MotorController::mode_changed(VesselMode mode)
{
    this->set_velocity(0.0);
}

void MotorController::set_velocity(float velocity)
{
    this->serial->printf("v 0 %f\n", velocity * -GEARBOX_RATIO / TWO_PI);
}

void MotorController::set_torque(float torque)
{
    this->serial->printf("c 0 %f\n", torque / -GEARBOX_RATIO);
}

float MotorController::get_velocity()
{
    return this->velocity;
}

float MotorController::get_position()
{
    return this->position;
}

float MotorController::get_torque()
{
    return this->torque;
}