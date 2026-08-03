/*
 * Copyright (c) 2026 GentleCare Corporation. All rights reserved.
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

#pragma once
#include <SparkFun_LPS28DFW_Arduino_Library.h>
#include "pressure_sensor.h"


class LpsSensor: public PressureSensor {
public:
    LpsSensor(const char *pressure_uuid, TwoWire* wire, int32_t SCL_pin, int32_t SDA_pin, uint8_t address);

    void start() override;

    void update(float dt) override;

    float get_temperature();

private:
    float read_psi() override;

    LPS28DFW sensor;
    uint8_t address;
    float temperature_c;
};