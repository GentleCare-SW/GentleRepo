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
#include "vessel.h"
#include "common/uuids.h"

Vessel::Vessel()
{
    this->mode = VesselMode::IDLE;
    this->peripheral_count = 0;
}

void Vessel::add_peripheral(Peripheral *peripheral)
{
    if (this->peripheral_count >= MAX_PERIPHERALS)
        return;
    
    this->peripherals[this->peripheral_count++] = peripheral;
}

void Vessel::start()
{
    for (int i = 0; i < this->peripheral_count; i++)
        this->peripherals[i]->start();

    BLEDevice::init("GentleBomb");
    this->server = BLEDevice::createServer();
    this->server->setCallbacks(this);
    this->service = this->server->createService(VESSEL_UUID);

    for (int i = 0; i < this->peripheral_count; i++) {
        Peripheral *peripheral = this->peripherals[i];
        for (int j = 0; j < peripheral->characteristic_count; j++) {
            Characteristic *characteristic = &peripheral->characteristics[j];
            characteristic->characteristic = this->service->createCharacteristic(characteristic->uuid, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
            characteristic->characteristic->setCallbacks(characteristic);
        }
    }

    this->service->start();
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(this->service->getUUID());
    advertising->setScanResponse(true);
    BLEDevice::startAdvertising();

    this->last_update_time = micros();
}

void Vessel::update()
{
    int64_t current_time = micros();
    float dt = (current_time - this->last_update_time) / 1e6;
    
    for (int i = 0; i < this->peripheral_count; i++)
        this->peripherals[i]->update(dt);
    
    this->last_update_time = current_time;
}

void Vessel::onConnect(BLEServer *server)
{
    this->mode = VesselMode::MANUAL_CONTROL;
}

void Vessel::onDisconnect(BLEServer *server)
{
    this->mode = VesselMode::IDLE;
    BLEDevice::startAdvertising();
}