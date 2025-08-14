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
#include "valve.h"

Valve::Valve(const char *uuid, int32_t dac_pin)
{
    this->dac_pin = dac_pin;
    this->percentage = 0.0;

    this->add_characteristic(uuid, std::bind(&Valve::set_percentage, this, std::placeholders::_1), std::bind(&Valve::get_percentage, this));
}

void Valve::set_percentage(float percentage)
{
    this->percentage = constrain(percentage, 0.0, 1.0);
    dacWrite(this->dac_pin, (uint8_t)(this->percentage * 255.0));
}

float Valve::get_percentage()
{
    return this->percentage;
}

void Valve::mode_changed(VesselMode mode)
{
    this->set_percentage(0.0);
}