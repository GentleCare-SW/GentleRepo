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
#include "config.h"

static const float MOTOR_UPDATE_INTERVAL = 0.002;

enum class AxisState {
    IDLE = 1,
    FULL_CALIBRATION_SEQUENCE = 3,
    CLOSED_LOOP_CONTROL = 8,
};

MotorController::MotorController(const char *position_uuid, const char *velocity_uuid, const char *torque_uuid, const char *error_uuid, HardwareSerial *serial, int32_t rx_pin, int32_t tx_pin)
{
    this->serial = serial;
    this->rx_pin = rx_pin;
    this->tx_pin = tx_pin;
    this->position = 0.0;
    this->velocity = 0.0;
    this->torque = 0.0;
    this->error = MotorControllerError::NONE;
    this->add_characteristic(position_uuid, nullptr, std::bind(&MotorController::get_position, this));
    this->add_characteristic(velocity_uuid, std::bind(&MotorController::set_velocity, this, std::placeholders::_1), std::bind(&MotorController::get_velocity, this));
    this->add_characteristic(torque_uuid, std::bind(&MotorController::set_torque, this, std::placeholders::_1), std::bind(&MotorController::get_torque, this));
    this->add_characteristic(error_uuid, nullptr, std::bind(&MotorController::get_error, this));
}

void MotorController::start()
{
    delay(2000);

    this->serial->begin(BAUD_RATE, SERIAL_8N1, this->rx_pin, this->tx_pin);
    while (!this->serial);
    this->serial->setTimeout(2000);

    this->write_state((int)AxisState::FULL_CALIBRATION_SEQUENCE);
    do {
        delay(100);
    } while ((AxisState)this->read_state() != AxisState::IDLE && this->error == MotorControllerError::NONE);

    this->write_state((int)AxisState::CLOSED_LOOP_CONTROL);
    delay(100);
    if ((AxisState)this->read_state() != AxisState::CLOSED_LOOP_CONTROL) {
        this->set_error(MotorControllerError::CALIBRATION_FAILED);
        return;
    }

    this->set_velocity(0.0);
    this->last_update_time = micros();
}

void MotorController::update(float dt)
{
    Peripheral::update(dt);

    int64_t current_time = micros();
    if (current_time - this->last_update_time > MOTOR_UPDATE_INTERVAL * 1e6) {
        dt = (current_time - this->last_update_time) / 1e6;
        this->last_update_time = current_time;

        this->position = this->read_position() * -1.0 / GEARBOX_RATIO;
        this->velocity = this->read_velocity() * -60.0 / GEARBOX_RATIO;
        float torque = this->read_torque() * -TORQUE_CONSTANT;

        float alpha = exp(-6.0 * dt);
        this->torque = (1.0 - alpha) * torque + alpha * this->torque;

        if (this->read_error() != 0)
            this->set_error(MotorControllerError::CONTROL_ERROR);
    }
}

void MotorController::mode_changed(VesselMode mode)
{
    this->set_velocity(0.0);
}

void MotorController::set_velocity(float velocity)
{
    this->write_velocity(velocity * -GEARBOX_RATIO / 60.0);
}

void MotorController::set_torque(float torque)
{
    this->write_torque(-torque);
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

void MotorController::set_error(MotorControllerError error)
{
    if (this->error != MotorControllerError::NONE && error != MotorControllerError::NONE)
        return;
    
    this->error = error;
}

float MotorController::get_error()
{
    return (float)this->error;
}

String MotorController::wait_for_response()
{
    if (this->error == MotorControllerError::NOT_RESPONDING)
        return "";

    int64_t start_time = millis();
    String response = this->serial->readStringUntil('\n');
    if (millis() - start_time > 1000)
        this->set_error(MotorControllerError::NOT_RESPONDING);
    
    return response;
}

void MotorController::write_state(int state)
{
    this->serial->printf("w axis0.requested_state %i\n", state);
}

int MotorController::read_state()
{
    this->serial->printf("r axis0.current_state\n");
    return this->wait_for_response().toInt();
}

void MotorController::write_torque(float torque)
{
    this->serial->printf("c 0 %f\n", torque);
}

float MotorController::read_torque()
{
    this->serial->printf("r axis0.motor.foc.Iq_setpoint\n");
    return this->wait_for_response().toFloat();
}

void MotorController::write_velocity(float velocity)
{
    this->serial->printf("v 0 %f\n", velocity);
}

float MotorController::read_velocity()
{
    this->serial->printf("r axis0.vel_estimate\n");
    return this->wait_for_response().toFloat();
}

float MotorController::read_position()
{
    this->serial->printf("r axis0.pos_estimate\n");
    return this->wait_for_response().toFloat();
}

int MotorController::read_error()
{
    this->serial->printf("r axis0.procedure_result\n");
    int result = this->wait_for_response().toInt();
    return result == 1 || result == 0 ? 0 : result;
}

bool MotorController::is_ok()
{
    return this->error == MotorControllerError::NONE;
}