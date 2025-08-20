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

enum VesselMode: Float {
    case idle = 0.0
    case eversion = 1.0
    case eversionPaused = 2.0
    case inversion = 3.0
    case inversionPaused = 4.0
}

class RemoteVessel: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var bluetoothStatus: String = "Starting..."
    @Published var isConnected: Bool = false
    @Published var airPressure: Float?
    @Published var voltagePercentage: Float?
    @Published var motorPosition: Float?
    @Published var motorVelocity: Float?
    @Published var motorTorque: Float?
    @Published var servoChamber: Float?
    @Published var mode: VesselMode?
    
    private var pollTimer: Timer!
    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var characteristics: [CBUUID: CBCharacteristic] = [:]
    private var values: [CBUUID: Float] = [:]
    private var waitingResponse: [CBUUID: Bool] = [:]
    
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
        waitingResponse[characteristic.uuid] = false
    }
    
    private func poll() {
        if !isConnected {
            return
        }
        
        for uuid in CHARACTERISTIC_UUIDS {
            if characteristics[uuid] == nil || (waitingResponse[uuid] ?? false) {
                continue
            }
            
            waitingResponse[uuid] = true
            peripheral!.readValue(for: characteristics[uuid]!)
        }
        
        airPressure = values[PRESSURE_SENSOR_UUID]
        voltagePercentage = values[VOLTAGE_PERCENTAGE_UUID]
        motorPosition = values[MOTOR_POSITION_UUID]
        motorVelocity = values[MOTOR_VELOCITY_UUID]
        motorTorque = values[MOTOR_TORQUE_UUID]
        servoChamber = values[SERVO_CHAMBER_UUID]
        if values[AUTO_CONTROL_MODE_UUID] != nil {
            mode = VesselMode(rawValue: values[AUTO_CONTROL_MODE_UUID]!)
        }
    }
    
    private func writeValue(_ value: Float, for uuid: CBUUID) {
        if peripheral == nil || characteristics[uuid] == nil {
            return
        }
        
        var bitPattern = value.bitPattern.littleEndian
        let data = withUnsafeBytes(of: &bitPattern) { Data($0) }
        peripheral!.writeValue(data, for: characteristics[uuid]!, type: .withResponse)
    }
    
    public func setMode(_ mode: VesselMode) {
        writeValue(mode.rawValue, for: AUTO_CONTROL_MODE_UUID)
    }
    
    public func setChamber(_ chamber: Int) {
        writeValue(Float(chamber), for: SERVO_CHAMBER_UUID)
    }
}
