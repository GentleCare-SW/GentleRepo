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
#include <NimBLEDevice.h>

class Remote: public NimBLEScanCallbacks {
public:
    Remote();

    void start();

    void update();

    void onResult(const NimBLEAdvertisedDevice *device) override;

    float get_pressure();

    float get_motor_position();

    float get_motor_velocity();

    float get_motor_torque();

    float get_voltage_percentage();

    float get_pressure_reference();

    void set_motor_velocity(float velocity);

    void set_motor_torque(float torque);

    void set_voltage_percentage(float percentage);

    void set_pressure_reference(float pressure);

private:
    NimBLEScan *scanner;
    NimBLEClient *client;
    bool found_device;
    NimBLEAdvertisedDevice *device;
    NimBLERemoteService *service;

    NimBLERemoteCharacteristic *pressure_sensor;
    NimBLERemoteCharacteristic *motor_position;
    NimBLERemoteCharacteristic *motor_velocity;
    NimBLERemoteCharacteristic *motor_torque;
    NimBLERemoteCharacteristic *voltage_percentage;
    NimBLERemoteCharacteristic *pressure_reference;
};