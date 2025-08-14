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
#include "remote.h"
#include "common/uuids.h"

Remote::Remote()
{
    found_device = false;
}

void Remote::start()
{
    BLEDevice::init("GentleBombDetonator");
    this->scanner = BLEDevice::getScan();
    this->scanner->setAdvertisedDeviceCallbacks(this);
    this->scanner->setActiveScan(true);

    this->client = BLEDevice::createClient();
}

void Remote::update()
{
    while (!this->client->isConnected()) {
        this->scanner->start(1);
        if (this->found_device) {
            this->found_device = false;

            this->client->connect(&this->device);
            this->service = this->client->getService(VESSEL_UUID);
            this->pressure_sensor = this->service->getCharacteristic(PRESSURE_SENSOR_UUID);
            this->motor_position = this->service->getCharacteristic(MOTOR_POSITION_UUID);
            this->motor_velocity = this->service->getCharacteristic(MOTOR_VELOCITY_UUID);
            this->motor_torque = this->service->getCharacteristic(MOTOR_TORQUE_UUID);
            this->voltage_percentage = this->service->getCharacteristic(VOLTAGE_PERCENTAGE_UUID);
        }
    }
}

void Remote::onResult(BLEAdvertisedDevice device)
{
#if DEBUG_MODE
    Serial.printf("Device: %s \n", device.toString().c_str());
#endif
    if (device.getName() != "GentleBomb")
        return;
    
    this->device = device;
    this->found_device = true;
}

float Remote::get_pressure()
{
    if (this->pressure_sensor == nullptr)
        return 0.0;

    return this->pressure_sensor->readFloat();
}

float Remote::get_motor_position()
{
    if (this->motor_position == nullptr)
        return 0.0;

    return this->motor_position->readFloat();
}

float Remote::get_motor_velocity()
{
    if (this->motor_velocity == nullptr)
        return 0.0;

    return this->motor_velocity->readFloat();
}

float Remote::get_motor_torque()
{
    if (this->motor_torque == nullptr)
        return 0.0;

    return this->motor_torque->readFloat();
}

float Remote::get_voltage_percentage()
{
    if (this->voltage_percentage == nullptr)
        return 0.0;

    return this->voltage_percentage->readFloat();
}

void Remote::set_motor_velocity(float velocity)
{
    if (this->motor_velocity == nullptr)
        return;

    this->motor_velocity->writeValue((uint8_t *)&velocity, sizeof(velocity));
}

void Remote::set_motor_torque(float torque)
{
    if (this->motor_torque == nullptr)
        return;

    this->motor_velocity->writeValue((uint8_t *)&torque, sizeof(torque));
}

void Remote::set_voltage_percentage(float percentage)
{
    if (this->voltage_percentage == nullptr)
        return;

    this->motor_velocity->writeValue((uint8_t *)&percentage, sizeof(percentage));
}