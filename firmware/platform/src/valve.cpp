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

#include "valve.h"
#include "config.h"


Valve::Valve(const char *proportional_uuid, const char *on_off_uuid, int32_t digital_pin, int32_t pwm_pin, int32_t ledc_channel)
{
    this->pwm_pin = pwm_pin;
    this->digital_pin = digital_pin;
    this->ledc_channel = ledc_channel;
    this->prop_voltage = 0.0;
    this->state = 0.0;

    this->add_characteristic(proportional_uuid, std::bind(&Valve::set_volt, this, std::placeholders::_1), std::bind(&Valve::get_volt, this));
    this->add_characteristic(on_off_uuid, std::bind(&Valve::set_state, this, std::placeholders::_1), std::bind(&Valve::get_state, this));
}

void Valve::start()
{
    pinMode(this->digital_pin, OUTPUT);
    pinMode(this->pwm_pin, OUTPUT);
    ledcSetup(this->ledc_channel, 400, 16);
    ledcAttachPin(this->pwm_pin, this->ledc_channel);
    this->set_volt(0.0);
    this->last_update_time = micros();

}

void Valve::set_volt(float voltage)
{
    Serial.println("set voltage");
    this->prop_voltage = constrain(voltage, 0.0, 15.0);
    uint32_t duty_cycle = this->prop_voltage / 15.0 * 0xffff;
    ledcWrite(this->ledc_channel, duty_cycle);
}

float Valve::get_volt()
{
    return this->prop_voltage;
}

void Valve::set_state(float state)
{
    Serial.println("set state");
    this->state = state;
    digitalWrite(this->digital_pin, state == 0.0? LOW : HIGH);
}

float Valve::get_state()
{
    return this->state;
}