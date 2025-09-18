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

#pragma once
#include "peripheral.h"
#include "voltage_dimmer.h"
#include "motor_controller.h"
#include "pressure_sensor.h"

class TensionController: public Peripheral {
public:
    TensionController();

    TensionController(VoltageDimmer *voltage_dimmer, MotorController *motor_controller, PressureSensor *pressure_sensor, float torque_reference);

    void update(float dt) override;

    void set_reference(float reference);

    float get_reference();

    void set_max_velocity(float max_velocity);

    void reset();

private:
    float voltage_percentage;
    float velocity;
    float min_velocity;
    float max_velocity;
    float torque_reference;
    VoltageDimmer *dimmmer;
    MotorController *motor;
    PressureSensor *pressure_sensor;
};