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
    NimBLEDevice::init("GentleBombDetonator");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    this->scanner = NimBLEDevice::getScan();
    this->scanner->setScanCallbacks(this);
    this->scanner->setActiveScan(true);
    this->scanner->setMaxResults(0);

    this->client = NimBLEDevice::createClient();
}

void Remote::update()
{
    while (!this->client->isConnected()) {
        if (!this->scanner->isScanning())
            this->scanner->start(0);

        if (this->found_device) {
            this->found_device = false;
            this->scanner->stop();

            this->client->connect(this->device);
            this->client->setConnectionParams(6, 12, 0, 100);
            this->service = this->client->getService(VESSEL_UUID);
            this->pressure_sensor = this->service->getCharacteristic(PRESSURE_SENSOR_UUID);
            this->motor_position = this->service->getCharacteristic(MOTOR_POSITION_UUID);
            this->motor_velocity = this->service->getCharacteristic(MOTOR_VELOCITY_UUID);
            this->motor_torque = this->service->getCharacteristic(MOTOR_TORQUE_UUID);
            this->voltage_percentage = this->service->getCharacteristic(VOLTAGE_PERCENTAGE_UUID);
            this->servo_angle = this->service->getCharacteristic(SERVO_ANGLE_UUID);
            this->servo_chamber = this->service->getCharacteristic(SERVO_CHAMBER_UUID);
            this->auto_control_mode = this->service->getCharacteristic(AUTO_CONTROL_MODE_UUID);
        }
    }
}

void Remote::onResult(const NimBLEAdvertisedDevice *device)
{
#if DEBUG_MODE
    Serial.printf("Device: %s\n", device->toString().c_str());
#endif
    if (device->getServiceUUID().toString() != VESSEL_UUID)
        return;
    
    this->device = (NimBLEAdvertisedDevice *)device;
    this->found_device = true;
}

float Remote::get_pressure()
{
    if (this->pressure_sensor == nullptr)
        return 0.0;

    return *(float *)this->pressure_sensor->readValue().data();
}

float Remote::get_motor_position()
{
    if (this->motor_position == nullptr)
        return 0.0;

    return *(float *)this->motor_position->readValue().data();
}

float Remote::get_motor_velocity()
{
    if (this->motor_velocity == nullptr)
        return 0.0;

    return *(float *)this->motor_velocity->readValue().data();
}

float Remote::get_motor_torque()
{
    if (this->motor_torque == nullptr)
        return 0.0;

    return *(float *)this->motor_torque->readValue().data();
}

float Remote::get_voltage_percentage()
{
    if (this->voltage_percentage == nullptr)
        return 0.0;

    return *(float *)this->voltage_percentage->readValue().data();
}

float Remote::get_servo_angle()
{
    if (this->servo_angle == nullptr)
        return 0.0;

    return *(float *)this->servo_angle->readValue().data();
}

float Remote::get_pressure_reference()
{
    if (this->pressure_reference == nullptr)
        return 0.0;

    return *(float *)this->pressure_reference->readValue().data();
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

    this->voltage_percentage->writeValue((uint8_t *)&percentage, sizeof(percentage));
}

void Remote::set_servo_angle(float angle)
{
    if (this->servo_angle == nullptr)
        return;

    this->servo_angle->writeValue((uint8_t *)&angle, sizeof(angle));
}

void Remote::set_servo_chamber(float chamber)
{
    if (this->servo_chamber == nullptr)
        return;

    this->servo_chamber->writeValue((uint8_t *)&chamber, sizeof(chamber));
}

void Remote::set_pressure_reference(float pressure)
{
    if (this->pressure_reference == nullptr)
        return;

    this->pressure_reference->writeValue((uint8_t *)&pressure, sizeof(pressure));
}

void Remote::set_auto_control_mode(float mode)
{
    if (this->auto_control_mode == nullptr)
        return;

    this->auto_control_mode->writeValue((uint8_t *)&mode, sizeof(mode));
}