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
#include "tension_controller.h"

static const float Kp = 0.005;

TensionController::TensionController()
{
    this->dimmmer = nullptr;
    this->motor = nullptr;
    this->pressure_sensor = nullptr;
    this->voltage_percentage = 0.0;
    this->torque_reference = 0.0;
}

TensionController::TensionController(VoltageDimmer *dimmer, MotorController *motor, PressureSensor *pressure_sensor, float torque_reference)
{
    this->dimmmer = dimmer;
    this->motor = motor;
    this->pressure_sensor = pressure_sensor;
    this->voltage_percentage = 0.0;
    this->torque_reference = torque_reference;
}

void TensionController::update(float dt)
{
    float velocity = this->motor->get_velocity();
    if (velocity < 0.05) {
        this->voltage_percentage = 0.0;
        return;
    }
    
    float torque = this->motor->get_torque();
    float pid = -(torque - this->torque_reference) * Kp;

    this->voltage_percentage = constrain(this->voltage_percentage + pid, 0.2, 0.32);
    this->dimmmer->set_percentage(this->voltage_percentage);
}

void TensionController::set_reference(float reference)
{
    this->torque_reference = reference;
}

float TensionController::get_reference()
{
    return this->torque_reference;
}