/*
 * Copyright (c) 2025 GentleCare Corporation. All rights reserved.
 *
 * This source code and the accompanying materials are the confidential and
 * proprietary information of GentleCare Corporation. Unauthorized copying or
 * distribution of this file, via any medium, is strictly prohibited without
 * the prior written permission of John Payne.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import CoreBluetooth

class RemoteVessel: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var bluetoothStatus: String = "Starting..."
    @Published var isConnected: Bool = false
    @Published var airPressure: Float?
    @Published var motorTorque: Float?
    
    private var pollTimer: Timer!
    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var characteristics: [CBUUID: CBCharacteristic] = [:]
    private var values: [CBUUID: Float] = [:]
    
    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: nil)
        pollTimer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true, block: { [weak self] _ in self?.poll() })
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            bluetoothStatus = "Bluetooth ON. Scanning..."
            startScan()
        case .poweredOff:
            bluetoothStatus = "Bluetooth is OFF. Please enable it in Settings."
        case .unauthorized:
            bluetoothStatus = "No Bluetooth permission. Grant it in Settings."
        case .unsupported:
            bluetoothStatus = "This device doesn't support Bluetooth."
        case .resetting:
            bluetoothStatus = "Bluetooth resetting..."
        default:
            bluetoothStatus = "Bluetooth state unknown."
        }
    }
    
    private func startScan() {
        if central.state != .poweredOn {
            return;
        }
        
        central.scanForPeripherals(withServices: [VESSEL_UUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        bluetoothStatus = "Discovered device. Connecting..."
        self.peripheral = peripheral
        
        central.stopScan()
        peripheral.delegate = self
        central.connect(peripheral)
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        bluetoothStatus = "Connected to device. Discovering services..."
        peripheral.discoverServices([VESSEL_UUID])
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Failed to connect. Retrying..."
        startScan()
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Disconnected from device. Starting scan..."
        isConnected = false
        startScan()
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: (any Error)?) {
        if error != nil || peripheral.services == nil {
            bluetoothStatus = "Service discovery error."
            return
        }
        
        bluetoothStatus = "Discovered services. Discovering characteristics..."
        peripheral.discoverCharacteristics(CHARACTERISTIC_UUIDS, for: peripheral.services![0])
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: (any Error)?) {
        if error != nil || peripheral.services![0].characteristics == nil {
            bluetoothStatus = "Characteristic discovery error."
            return
        }
        
        for characteristic in peripheral.services![0].characteristics! {
            characteristics[characteristic.uuid] = characteristic
        }
        
        bluetoothStatus = "Connected to device."
        isConnected = true
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        if error != nil || characteristic.value == nil {
            bluetoothStatus = "Failed to read value."
            return
        }
        
        if characteristic.value!.count != 4 {
            return
        }
        
        let value = Float(bitPattern: UInt32(littleEndian: characteristic.value!.withUnsafeBytes { $0.load(as: UInt32.self) }))
        values[characteristic.uuid] = value
    }
    
    private func poll() {
        if !isConnected {
            return
        }
        
        for uuid in CHARACTERISTIC_UUIDS {
            if characteristics[uuid] == nil {
                continue
            }
            
            peripheral!.readValue(for: characteristics[uuid]!)
        }
        
        airPressure = values[PRESSURE_SENSOR_UUID]
        motorTorque = values[MOTOR_TORQUE_UUID]
    }
}
