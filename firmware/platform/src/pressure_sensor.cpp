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
#include "pressure_sensor.h"


PressureSensor::PressureSensor(const char *pressure_uuid, const char *error_uuid)
{
    this->last_psi = 0.0;
    this->moving_pressure = 0.0;
    this->moving_squared_pressure = 0.0;
    this->pressure_derivative = 0.0;
    this->pressure_offset = 0.0;
    this->calibrating = false;
    this->error = PressureSensorError::NONE;

    this->add_characteristic(pressure_uuid, nullptr, std::bind(&PressureSensor::get_pressure, this));
    this->add_characteristic(error_uuid, nullptr, std::bind(&PressureSensor::get_error, this));
}

void PressureSensor::start()
{
    this->sensor = Adafruit_MPRLS();
    if (! this->sensor.begin()) {
        this->error = PressureSensorError::NOT_CONNECTED;
        Serial.println("Failed to communicate with pressure sensor, check wiring?");
    }
    this->error = PressureSensorError::NONE;
}

void PressureSensor::update(float dt)
{
    Peripheral::update(dt);

    float psi = this->read_psi();
    //if (abs(psi - this->last_psi) > 1.0)
    //    psi = this->last_psi + (psi > this->last_psi ? 1.0 : -1.0);

    //float alpha = exp(-dt);
    float alpha = 0.5;

    float previous_moving_pressure = this->moving_pressure;
    this->moving_pressure = this->moving_pressure * alpha + psi * (1.0 - alpha);
    this->moving_squared_pressure = this->moving_squared_pressure * alpha + psi * psi * (1.0 - alpha);

    this->pressure_derivative = (this->moving_pressure - previous_moving_pressure) / dt;

    if (this->calibrating)
        this->pressure_offset = this->moving_pressure;

    float std = sqrt(max(0.0f, this->moving_squared_pressure - this->moving_pressure * this->moving_pressure));
    if (this->error == PressureSensorError::NONE && std > 10.0)
        this->error = PressureSensorError::NOT_CONNECTED;
    else if (this->error == PressureSensorError::NOT_CONNECTED && std <= 10.0)
        this->error = PressureSensorError::NONE;

    this->last_psi = psi;
}

float PressureSensor::read_psi()
{
    float pressure_hPa = this->sensor.readPressure();
    return pressure_hPa / 68.947572932;
}

float PressureSensor::get_pressure()
{
    //return this->read_psi() - this->pressure_offset;
    return this->moving_pressure - this->pressure_offset;
}

float PressureSensor::get_derivative()
{
    return this->pressure_derivative;
}

bool PressureSensor::is_ok()
{
    return this->error == PressureSensorError::NONE;
}

void PressureSensor::set_calibrating(bool calibrating)
{
    this->calibrating = calibrating;
}

float PressureSensor::get_error()
{
    return (float)this->error;
}