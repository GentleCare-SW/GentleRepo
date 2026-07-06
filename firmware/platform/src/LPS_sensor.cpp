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
#include "LPS_sensor.h"


LpsSensor::LpsSensor(const char *pressure_uuid, TwoWire* wire, int32_t SCL_pin, int32_t SDA_pin, uint8_t address)
{
    this->clock_pin = SCL_pin;
    this->data_pin = SDA_pin;
    this->wire = wire;
    this->address = address;
    this->moving_pressure = 0.0;
    this->moving_squared_pressure = 0.0;
    this->pressure_derivative = 0.0;
    this->pressure_offset = 0.0;
    this->calibrating = false;
    this->error = PressureSensorError::NONE;

    this->add_characteristic(pressure_uuid, nullptr, std::bind(&LpsSensor::get_pressure, this));
    //this->add_characteristic(error_uuid, nullptr, std::bind(&LpsSensor::get_error, this));
    //this->add_characteristic(temperature_uuid, nullptr, std::bind(&LpsSensor::get_temperature, this));
}


void LpsSensor::start()
{
    LPS28DFW pressure_sensor;
    this->sensor = pressure_sensor;
    this->wire->begin(this->data_pin, this->clock_pin);
    this->error = PressureSensorError::NONE;
    if (! this->sensor.begin(address) != LPS28DFW_OK) {
        this->error = PressureSensorError::NOT_CONNECTED;
        Serial.println("Failed to communicate with pressure sensor, check wiring?");
    }
}

void LpsSensor::update(float dt)
{
    Peripheral::update(dt);
    if (this->error == PressureSensorError::NOT_CONNECTED)
        return;

    this->sensor.getSensorData();
    float psi = this->read_psi();
    this->temperature_c = this->sensor.data.heat.deg_c;
    
    float alpha = 0.75;

    float previous_moving_pressure = this->moving_pressure;
    this->moving_pressure = this->moving_pressure * alpha + psi * (1.0 - alpha);
    this->moving_squared_pressure = this->moving_squared_pressure * alpha + psi * psi * (1.0 - alpha);

    this->pressure_derivative = (this->moving_pressure - previous_moving_pressure) / dt;

    if (this->calibrating && abs(this->pressure_derivative) <= 0.01) {
        this->pressure_offset = this->moving_pressure;
    }

    //float std = sqrt(max(0.0f, this->moving_squared_pressure - this->moving_pressure * this->moving_pressure));
    
}

float LpsSensor::read_psi()
{
    float pressure_hPa = this->sensor.data.pressure.hpa;
    return pressure_hPa / 68.947572932;
}

float LpsSensor::get_pressure()
{
    //return this->read_psi() - this->pressure_offset;
    return this->moving_pressure - this->pressure_offset;
}

float LpsSensor::get_temperature()
{
    return this->temperature_c;
}

float LpsSensor::get_derivative()
{
    return this->pressure_derivative;
}

void LpsSensor::set_calibrating(bool calibrating)
{
    this->calibrating = calibrating;
}

void LpsSensor::set_error(PressureSensorError error)
{
    this->error = error;
}

float LpsSensor::get_error()
{
    return (float)this->error;
}