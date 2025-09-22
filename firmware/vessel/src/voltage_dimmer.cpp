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
#include "voltage_dimmer.h"
#include "config.h"

VoltageDimmer::VoltageDimmer(const char *uuid, int32_t pwm_pin, int32_t ledc_channel)
{
    this->pwm_pin = pwm_pin;
    this->ledc_channel = ledc_channel;
    this->voltage = 0.0;

    this->add_characteristic(uuid, std::bind(&VoltageDimmer::set_voltage, this, std::placeholders::_1), std::bind(&VoltageDimmer::get_voltage, this));
}

void VoltageDimmer::start()
{
    pinMode(this->pwm_pin, OUTPUT);
    ledcSetup(this->ledc_channel, 1000, 16);
    ledcAttachPin(this->pwm_pin, this->ledc_channel);
    this->set_voltage(this->voltage);
}

void VoltageDimmer::set_voltage(float voltage)
{
    this->voltage = constrain(voltage, 0.0, 120.0);
    float percentage = this->voltage_to_pwm_percentage(this->voltage);
    ledcWrite(this->ledc_channel, (uint32_t)(percentage * 0xffff));
}

float VoltageDimmer::get_voltage()
{
    return this->voltage;
}

void VoltageDimmer::mode_changed(VesselMode mode)
{
    this->set_voltage(0.0);
}

static const float A = 1.3;
static const float P = 1.9;
static const float Q = 0.6;

float VoltageDimmer::pwm_percentage_to_voltage(float percentage)
{
    percentage = constrain(percentage, 0.0, 1.0);
    if (percentage == 1.0)
        return MAX_DIMMER_VOLTAGE;
    
    return MAX_DIMMER_VOLTAGE * pow(1.0 + A * pow(percentage / (1.0 - percentage), -P), -Q);
}

float VoltageDimmer::voltage_to_pwm_percentage(float voltage)
{
    voltage = constrain(voltage, 0.0, MAX_DIMMER_VOLTAGE);
    if (voltage == 0.0)
        return 0.0;

    return 1.0 / (1.0 + pow((pow(voltage / 120.0, -1.0 / Q) - 1.0) / A, 1.0 / P));
}