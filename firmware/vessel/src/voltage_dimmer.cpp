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

VoltageDimmer::VoltageDimmer(const char *uuid, int32_t pwm_pin)
{
    this->pwm_pin = pwm_pin;
    this->percentage = 0.0;

    this->add_characteristic(uuid, std::bind(&VoltageDimmer::set_percentage, this, std::placeholders::_1), std::bind(&VoltageDimmer::get_percentage, this));
}

void VoltageDimmer::start()
{
    pinMode(this->pwm_pin, OUTPUT);
    analogWriteFrequency(1000);
    analogWriteResolution(16);
    analogWrite(this->pwm_pin, 0);
}

void VoltageDimmer::set_percentage(float percentage)
{
    this->percentage = constrain(percentage, 0.0, 1.0);
    analogWrite(this->pwm_pin, (uint32_t)(this->percentage * 0xffff));
}

float VoltageDimmer::get_percentage()
{
    return this->percentage;
}