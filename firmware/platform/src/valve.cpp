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


Valve::Valve(const char *valve_uuid, int32_t digital_pin1, int32_t digital_pin2)
{
    this->digital_pin1=digital_pin1; //3-way valve
    this->digital_pin2=digital_pin2; //2-way valve
    this->state = 0.0;

    //this->add_characteristic(proportional_uuid, std::bind(&Valve::set_volt, this, std::placeholders::_1), std::bind(&Valve::get_volt, this));
    this->add_characteristic(valve_uuid, std::bind(&Valve::set_state, this, std::placeholders::_1), std::bind(&Valve::get_state, this));
}

void Valve::start()
{
    pinMode(this->digital_pin1, OUTPUT);
    pinMode(this->digital_pin2, OUTPUT);
    this->set_state(0.0);
    this->last_update_time = micros();
}

void Valve::set_state(float state)
{
    this->state = state;
    if (state == 0.0) { //hold
        digitalWrite(this->digital_pin1, LOW);
        digitalWrite(this->digital_pin2, LOW);
    } else if (state == 1.0) { //drain
        digitalWrite(this->digital_pin1, LOW);
        digitalWrite(this->digital_pin2, HIGH);
    } else if (state == 2.0) { //fill
        digitalWrite(this->digital_pin1, HIGH);
        digitalWrite(this->digital_pin2, HIGH);
    }
}

float Valve::get_state()
{
    return this->state;
}