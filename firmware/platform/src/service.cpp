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
#include "service.h"
#include "config.h"
#include "common/uuids.h"

Service::Service()
{
    this->mode = ServiceMode::IDLE;
    this->peripheral_count = 0;
}

void Service::add_peripheral(Peripheral *peripheral)
{
    if (this->peripheral_count >= MAX_PERIPHERALS)
        return;
    
    this->peripherals[this->peripheral_count++] = peripheral;
}

void Service::start()
{
    for (int i = 0; i < this->peripheral_count; i++)
        this->peripherals[i]->start();

    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPowerLevel(ESP_PWR_LVL_P9);
    this->ble_server = NimBLEDevice::createServer();
    this->ble_server->setCallbacks(this);
    this->ble_service = this->ble_server->createService(SERVICE_UUID);

    for (int i = 0; i < this->peripheral_count; i++) {
        Peripheral *peripheral = this->peripherals[i];
        for (int j = 0; j < peripheral->characteristic_count; j++) {
            Characteristic *characteristic = &peripheral->characteristics[j];
            characteristic->characteristic = this->ble_service->createCharacteristic(characteristic->uuid, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);
            characteristic->characteristic->setCallbacks(characteristic);
        }
    }

    this->ble_service->start();
    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(this->ble_service->getUUID());
    NimBLEDevice::startAdvertising();

    this->last_update_time = micros();
}

void Service::update()
{
    uint32_t current_time = micros();
    float dt = max(1e-10, (current_time - this->last_update_time) / 1e6);
    
    for (int i = 0; i < this->peripheral_count; i++)
        this->peripherals[i]->update(dt);
    
    this->last_update_time = current_time;
}

void Service::onConnect(NimBLEServer *server, NimBLEConnInfo& info)
{
    this->set_mode(ServiceMode::CONNECTED);
    server->updateConnParams(info.getConnHandle(), 6, 12, 0, 100);
}

void Service::onDisconnect(NimBLEServer *server, NimBLEConnInfo& info, int reason)
{
    this->set_mode(ServiceMode::IDLE);
    NimBLEDevice::startAdvertising();
}

void Service::set_mode(ServiceMode mode)
{
    this->mode = mode;
    for (int i = 0; i < this->peripheral_count; i++)
        this->peripherals[i]->mode_changed(mode);
}