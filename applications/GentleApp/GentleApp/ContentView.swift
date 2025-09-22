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

import SwiftUI
import Charts

struct ContentView: View {
    @StateObject private var vessel = RemoteVessel()
    @State private var showDevInfo = false
    
    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(spacing: 16) {
                    if vessel.isConnected {
                        StatusHeader(vessel: vessel)
                        ControlCard(vessel: vessel)
                        if vessel.servoChamber != nil {
                            ChamberSelector(vessel: vessel)
                        }
                        ProgressSection(vessel: vessel)
                        DeveloperInfoSection(vessel: vessel, expanded: $showDevInfo)
                    } else {
                        AvailableDevicesSection(vessel: vessel)
                    }
                }
                .padding(16)
            }
            .background(.appBackground)
            .navigationTitle(vessel.isConnected ? vessel.connectedDeviceName : "Home Menu")
            .toolbarTitleDisplayMode(.large)
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    ConnectivityPill(statusText: vessel.bluetoothStatus, isConnected: vessel.isConnected)
                }
                if vessel.isConnected {
                    ToolbarItem(placement: .topBarLeading) {
                        Button {
                            vessel.disconnectFromDevice()
                            haptic(.soft)
                        } label: {
                            Label("Disconnect", systemImage: "bolt.slash.fill")
                                .labelStyle(.titleOnly)
                        }
                        .buttonStyle(.bordered)
                        .tint(.red)
                        .accessibilityHint("Disconnect from the current device")
                    }
                }
            }
        }
        .largeDisplayBoost()
        .alert(vessel.alertMessage, isPresented: $vessel.showingAlert) {
            Button("OK") { }
        }
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
        }
        .onDisappear {
            UIApplication.shared.isIdleTimerDisabled = false
        }
    }
}

private struct StatusHeader: View {
    @ObservedObject var vessel: RemoteVessel
    
    var body: some View {
        Card {
            HStack(alignment: .center, spacing: 12) {
                StatusDot(isOK: vessel.isConnected)
                    .accessibilityHidden(true)
                
                VStack(alignment: .leading, spacing: 4) {
                    Text(vessel.isConnected ? "Connected" : "Not Connected")
                        .font(.headline)
                    if vessel.isConnected && vessel.motorErrorLabel != "" {
                        Text(vessel.motorErrorLabel)
                            .font(.subheadline)
                            .foregroundStyle(.red)
                    }
                    if vessel.isConnected && vessel.pressureSensorErrorLabel != "" {
                        Text(vessel.pressureSensorErrorLabel)
                            .font(.subheadline)
                            .foregroundStyle(.red)
                    }
                }
                Spacer(minLength: 12)
                
                if vessel.isConnected {
                    ModeTag(text: vessel.modeDisplay, tone: vessel.modeTone)
                }
            }
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Vessel status")
        .accessibilityValue("\(vessel.isConnected ? "Connected" : "Not connected"), mode \(vessel.modeDisplay)")
    }
}

private struct ControlCard: View {
    @ObservedObject var vessel: RemoteVessel
    
    var body: some View {
        Card {
            VStack(alignment: .leading, spacing: 12) {
                Text("Controls")
                    .font(.title3).bold()
                
                HStack(spacing: 12) {
                    Button {
                        vessel.setMode(vessel.mode == .eversion ? .eversionPaused : .inversionPaused)
                        haptic(.heavy)
                    } label: {
                        Label("Pause", systemImage: "pause.fill")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.orange)
                    .disabled(!vessel.canPause)
                    .accessibilityHint("Pauses current motion")
                    
                    Button {
                        vessel.resumeIfPaused()
                        haptic(.soft)
                    } label: {
                        Label("Resume", systemImage: "play.fill")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.green)
                    .disabled(!vessel.canResume)
                    .accessibilityHint("Resumes motion")
                }
                
                HStack(spacing: 12) {
                    Button {
                        vessel.setMode(.inversion)
                        haptic(.rigid)
                    } label: {
                        Label("Invert", systemImage: "arrow.up.right.and.arrow.down.left")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.purple)
                    .disabled(!vessel.canInvert)
                    
                    Button {
                        vessel.setMode(.eversion)
                        haptic(.rigid)
                    } label: {
                        Label("Evert", systemImage: "arrow.down.left.and.arrow.up.right")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.blue)
                    .disabled(!vessel.canEvert)
                }
                
                Button {
                    vessel.setMode(.idle)
                    haptic(.light)
                } label: {
                    Label("Stop", systemImage: "stop.circle.fill")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)
                .disabled(!vessel.isConnected || vessel.mode == .idle)
            }
        }
    }
}

private struct ChamberSelector: View {
    @ObservedObject var vessel: RemoteVessel
    
    var body: some View {
        Card {
            VStack(alignment: .leading, spacing: 12) {
                Text("Active Chamber")
                    .font(.title3).bold()
                
                HStack(spacing: 12) {
                    SelectablePill(title: "Chamber 0",
                                   isSelected: vessel.servoChamber == 0,
                                   action: { vessel.setChamber(0); haptic(.heavy) })
                    .disabled(!vessel.canSelectChamber)
                    
                    SelectablePill(title: "Chamber 1",
                                   isSelected: vessel.servoChamber == 1,
                                   action: { vessel.setChamber(1); haptic(.heavy) })
                    .disabled(!vessel.canSelectChamber)
                }
                .accessibilityElement(children: .contain)
                .accessibilityLabel("Chamber selection")
                .accessibilityValue(vessel.servoChamber == nil ? "None" : "Chamber \(vessel.servoChamber!)")
            }
        }
    }
}

private struct ProgressSection: View {
    @ObservedObject var vessel: RemoteVessel
    
    var body: some View {
        Card {
            VStack(alignment: .leading, spacing: 12) {
                Text("Progress")
                    .font(.title3).bold()
                
                let value = vessel.progress ?? 0
                ProgressView(value: value, total: 1.0)
                    .progressViewStyle(.linear)
                
                HStack {
                    Text(vessel.progressLabel)
                        .font(.subheadline).bold()
                    Spacer()
                    if vessel.isMoving {
                        AnimatedDot()
                            .accessibilityHidden(true)
                    }
                }
                .accessibilityElement(children: .combine)
                .accessibilityLabel("Progress")
                .accessibilityValue(vessel.progressLabel)
            }
        }
    }
}

private struct AvailableDevicesSection: View {
    @ObservedObject var vessel: RemoteVessel
    
    var body: some View {
        Card {
            VStack(alignment: .leading, spacing: 12) {
                Text("Available Devices")
                    .font(.title3).bold()
                
                if vessel.availableDevices.isEmpty {
                    HStack(spacing: 8) {
                        ProgressView()
                        Text("Searching…")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                    }
                    .accessibilityLabel("Searching for devices")
                } else {
                    LazyVStack(spacing: 10) {
                        ForEach(vessel.availableDevices, id: \.self) { name in
                            Button {
                                vessel.connectToDevice(name)
                                haptic(.light)
                            } label: {
                                HStack(spacing: 10) {
                                    Image(systemName: "antenna.radiowaves.left.and.right")
                                    Text(name)
                                        .font(.body).bold()
                                        .lineLimit(1)
                                    Spacer()
                                    Image(systemName: "chevron.right")
                                        .foregroundStyle(.secondary)
                                }
                                .frame(maxWidth: .infinity, alignment: .leading)
                            }
                            .buttonStyle(.borderedProminent)
                            .tint(.blue)
                            .accessibilityLabel("Connect to \(name)")
                        }
                    }
                }
            }
        }
        .accessibilityElement(children: .contain)
        .accessibilityLabel("Available devices list")
    }
}

private struct MetricPoint: Identifiable {
    let id = UUID()
    let time: Date
    let value: Double
}

private struct LiveMetricChart: View {
    let title: String
    let unit: String
    let series: [MetricPoint]

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text(title).font(.headline)
                Spacer()
                if let last = series.last?.value {
                    Text(String(format: "%.2f%@", last, unit))
                        .font(.subheadline).monospacedDigit()
                        .foregroundStyle(.secondary)
                        .accessibilityLabel("\(title) latest")
                        .accessibilityValue("\(last) \(unit)")
                }
            }

            Chart(series) {
                LineMark(
                    x: .value("Time", $0.time),
                    y: .value(title, $0.value)
                )
                .interpolationMethod(.monotone)
            }
            .chartXAxis {
                AxisMarks(values: .automatic(desiredCount: 3)) { _ in
                    AxisGridLine()
                }
            }
            .chartYAxis {
                AxisMarks(position: .leading)
            }
            .chartYScale()
            .frame(height: 160)
            .accessibilityElement(children: .combine)
            .accessibilityLabel("\(title) chart")
        }
    }
}

private struct DeveloperInfoSection: View {
    @ObservedObject var vessel: RemoteVessel
    @Binding var expanded: Bool
    
    @State var sliderValue: Float = 0.0

    @State var chart1Series: [MetricPoint] = []
    @State var chart2Series: [MetricPoint] = []
    let sampleInterval = 0.1
    let maxSamples = 150
    @State var isPlotPaused = false

    var body: some View {
        Card {
            DisclosureGroup(isExpanded: $expanded) {
                Grid(alignment: .leading, horizontalSpacing: 12, verticalSpacing: 10) {
                    DevRow("Air Pressure (PSI)", vessel.airPressure, suffix: nil, formatting: .number)
                    DevRow("Pressure Sensor Error", vessel.pressureSensorError, suffix: nil, formatting: .integer)
                    DevRow("Voltage (V)", vessel.voltage != nil ? vessel.voltage! : nil, suffix: nil, formatting: .number)
                    DevRow("Motor Position (rad)", vessel.motorPosition, suffix: nil, formatting: .number)
                    DevRow("Motor Velocity (rad/s)", vessel.motorVelocity, suffix: nil, formatting: .number)
                    DevRow("Motor Torque (Nm)", vessel.motorTorque, suffix: nil, formatting: .number)
                    DevRow("Motor Error", vessel.motorError, suffix: nil, formatting: .integer)
                    DevRow("Servo Chamber", vessel.servoChamber, suffix: nil, formatting: .integer)
                    DevRow("Mode", vessel.mode?.rawValue, suffix: nil, formatting: .string)
                    DevRow("Monitor Status", vessel.monitorStatus, suffix: nil, formatting: .integer)
                    DevRow("Bluetooth Status", vessel.bluetoothStatus, suffix: nil, formatting: .string)
                }
                .padding(.top, 8)

                VStack(alignment: .leading, spacing: 8) {
                    HStack {
                        Text("Live Plots").font(.title3).bold()
                        Spacer()
                        Button(isPlotPaused ? "Resume" : "Pause") { isPlotPaused.toggle() }
                            .buttonStyle(.bordered)
                            .disabled(!vessel.isConnected)
                            .accessibilityHint("Pause or resume live chart updates")
                    }

                    LiveMetricChart(
                        title: "Air Pressure",
                        unit: " PSI",
                        series: chart1Series,
                    )

                    LiveMetricChart(
                        title: "Motor Torque",
                        unit: " Nm",
                        series: chart2Series,
                    )
                }
                .padding(.top, 4)
                
                Divider().padding(.vertical, 8)
                
                VStack(alignment: .leading, spacing: 12) {
                    Text("Manual Control")
                        .font(.headline)
                    
                    HStack {
                        Text("Voltage: \(sliderValue, specifier: "%.1f") V")
                            .font(.subheadline)
                    }
                    
                    let voltageBinding = Binding<Float>(
                        get: { sliderValue },
                        set: {
                            sliderValue = $0
                            vessel.setVoltage($0)
                        }
                    )
                    
                    Slider(value: voltageBinding, in: 0...72.0, step: 2.0) {}
                    .disabled(!vessel.isConnected)
                    
                    HStack(spacing: 12) {
                        Button {
                        } label: {
                            Label("Motor In", systemImage: "arrow.left.circle.fill")
                                .frame(maxWidth: .infinity)
                        }
                        .buttonStyle(.borderedProminent)
                        .tint(.purple)
                        .disabled(!vessel.isConnected)
                        .onLongPressGesture(minimumDuration: 0, maximumDistance: 30, pressing: { isPressing in
                                if isPressing {
                                    vessel.setMotorVelocity(-20.0)
                                } else {
                                    vessel.setMotorVelocity(0.0)
                                }
                            },
                            perform: {})
                        
                        Button {
                        } label: {
                            Label("Motor Out", systemImage: "arrow.right.circle.fill")
                                .frame(maxWidth: .infinity)
                        }
                        .buttonStyle(.borderedProminent)
                        .tint(.blue)
                        .disabled(!vessel.isConnected)
                        .onLongPressGesture(minimumDuration: 0, maximumDistance: 30, pressing: { isPressing in
                                if isPressing {
                                    vessel.setMotorVelocity(20.0)
                                } else {
                                    vessel.setMotorVelocity(0.0)
                                }
                            },
                            perform: {})
                    }
                }
            } label: {
                HStack {
                    Text("Developer Info")
                        .font(.title3).bold()
                    Spacer()
                }
            }
        }
        .accessibilityElement(children: .contain)
        .task(id: vessel.isConnected) {
            guard vessel.isConnected else { return }
            while vessel.isConnected {
                if !isPlotPaused {
                    let now = Date()
                    await MainActor.run {
                        if let pressure = vessel.airPressure {
                            append(.init(time: now, value: Double(pressure)), to: &chart1Series)
                        }
                        if let torque = vessel.motorTorque {
                            append(.init(time: now, value: Double(torque)), to: &chart2Series)
                        }
                    }
                }
                try? await Task.sleep(nanoseconds: UInt64(sampleInterval * 1_000_000_000))
            }
        }
        .onChange(of: vessel.isConnected) { wasConnected, connected in
            if !connected {
                chart1Series.removeAll()
                chart2Series.removeAll()
            }
        }
    }

    private func append(_ point: MetricPoint, to array: inout [MetricPoint]) {
        array.append(point)
        if array.count > maxSamples {
            array.removeFirst(array.count - maxSamples)
        }
    }
}

private struct LargeDisplayBoost: ViewModifier {
    @Environment(\.horizontalSizeClass) private var hSize

    func body(content: Content) -> some View {
        if hSize == .regular {
            content
                .controlSize(.extraLarge)
                .dynamicTypeSize(.xxLarge)
        } else {
            content
        }
    }
}

private extension View {
    func largeDisplayBoost() -> some View { modifier(LargeDisplayBoost()) }
}

private struct Card<Content: View>: View {
    @ViewBuilder var content: Content

    @ScaledMetric(relativeTo: .title3) private var cardPadding: CGFloat = 16
    @ScaledMetric(relativeTo: .title3) private var cornerRadius: CGFloat = 16

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            content
        }
        .padding(cardPadding)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.thickMaterial, in: RoundedRectangle(cornerRadius: cornerRadius, style: .continuous))
        .overlay(
            RoundedRectangle(cornerRadius: cornerRadius, style: .continuous)
                .strokeBorder(.quaternary, lineWidth: 0.5)
        )
        .shadow(color: .black.opacity(0.05), radius: 8, x: 0, y: 4)
    }
}

private struct ModeTag: View {
    let text: String
    let tone: Color
    
    var body: some View {
        Text(text)
            .font(.caption).bold()
            .padding(.horizontal, 10).padding(.vertical, 6)
            .background(tone.opacity(0.15), in: Capsule())
            .foregroundStyle(tone)
    }
}

private struct StatusDot: View {
    let isOK: Bool
    @ScaledMetric(relativeTo: .body) private var dotSize: CGFloat = 14

    var body: some View {
        Circle()
            .fill(isOK ? .green : .red)
            .frame(width: dotSize, height: dotSize)
            .overlay(Circle().stroke(.white, lineWidth: 2))
            .shadow(radius: 1, x: 0, y: 1)
    }
}

private struct ConnectivityPill: View {
    var statusText: String
    var isConnected: Bool
    @ScaledMetric(relativeTo: .footnote) private var dotSize: CGFloat = 8

    var body: some View {
        HStack(spacing: 6) {
            Circle().fill(isConnected ? .green : .gray)
                .frame(width: dotSize, height: dotSize)
            Text(statusText)
                .font(.footnote).monospacedDigit()
        }
        .padding(.horizontal, 10).padding(.vertical, 6)
        .background(.ultraThinMaterial, in: Capsule())
    }
}

private struct SelectablePill: View {
    let title: String
    let isSelected: Bool
    let action: () -> Void
    
    var body: some View {
        Button(action: action) {
            HStack(spacing: 8) {
                Image(systemName: isSelected ? "checkmark.circle.fill" : "circle")
                Text(title)
                    .font(.body).bold()
                    .lineLimit(1)
                    .minimumScaleFactor(0.9)
            }
            .frame(maxWidth: .infinity)
        }
        .buttonStyle(.bordered)
        .tint(isSelected ? .accentColor : .secondary)
        .padding(.vertical, 2)
        .accessibilityAddTraits(isSelected ? .isSelected : [])
    }
}

private struct AnimatedDot: View {
    @ScaledMetric(relativeTo: .body) private var dotSize: CGFloat = 8
    @State private var scale: CGFloat = 1.0
    
    var body: some View {
        Circle()
            .fill()
            .frame(width: dotSize, height: dotSize)
            .scaleEffect(scale)
            .onAppear {
                withAnimation(.easeInOut(duration: 0.9).repeatForever(autoreverses: true)) {
                    scale = 1.4
                }
            }
    }
}

private enum DevFormat {
    case number, integer, string
}

private struct DevRow<T>: View {
    let label: String
    let value: T?
    let suffix: String?
    let formatting: DevFormat
    
    init(_ label: String, _ value: T?, suffix: String?, formatting: DevFormat) {
        self.label = label; self.value = value; self.suffix = suffix; self.formatting = formatting
    }
    
    var body: some View {
        HStack {
            Text(label).font(.subheadline)
            Spacer()
            Text(formatted(value) ?? "N/A")
                .font(.subheadline).monospacedDigit()
                .foregroundStyle(value == nil ? .secondary : .primary)
        }
        .accessibilityElement(children: .combine)
        .accessibilityLabel(label)
        .accessibilityValue(formatted(value) ?? "Not available")
    }
    
    private func formatted(_ value: T?) -> String? {
        guard let v = value else { return nil }
        switch formatting {
        case .number:
            if let n = v as? NSNumber { return "\(n)" + (suffix ?? "") }
            if let d = v as? Double { return String(format: "%.3f", d) + (suffix ?? "") }
            if let f = v as? Float { return String(format: "%.3f", f) + (suffix ?? "") }
            if let i = v as? Int { return "\(i)" + (suffix ?? "") }
            return "\(v)"
        case .integer:
            if let i = v as? Int { return "\(i)" + (suffix ?? "") }
            if let d = v as? Double { return "\(Int(d))" + (suffix ?? "") }
            return "\(v)"
        case .string:
            return "\(v)"
        }
    }
}

private extension ShapeStyle where Self == Color {
    static var appBackground: Color {
        Color(UIColor.systemGroupedBackground)
    }
}

private func haptic(_ style: UIImpactFeedbackGenerator.FeedbackStyle) {
    let gen = UIImpactFeedbackGenerator(style: style)
    gen.prepare()
    gen.impactOccurred()
}

private extension RemoteVessel {
    var modeDisplay: String {
        if mode == nil {
            return ""
        }
        switch mode! {
            case .idle: return "Idle"
            case .eversion: return "Everting"
            case .eversionPaused: return "Paused"
            case .inversion: return "Inverting"
            case .inversionPaused: return "Paused"
        }
    }
    
    var modeTone: Color {
        switch mode {
        case .some(.idle): return .gray
        case .some(.eversion): return .blue
        case .some(.inversion): return .purple
        case .some(.eversionPaused), .some(.inversionPaused): return .orange
        case .none: return .gray
        }
    }
    
    var isMoving: Bool {
        switch mode {
        case .some(.eversion), .some(.inversion): return true
        default: return false
        }
    }
    var isPaused: Bool {
        switch mode {
        case .some(.eversionPaused), .some(.inversionPaused): return true
        default: return false
        }
    }
    
    var canPause: Bool {
        isConnected && isMoving
    }
    var canResume: Bool {
        isConnected && isPaused
    }
    var canEvert: Bool {
        isConnected && !(mode == .eversion || mode == .eversionPaused)
    }
    var canInvert: Bool {
        isConnected && !(mode == .inversion || mode == .inversionPaused)
    }
    var canSelectChamber: Bool {
        isConnected && !isMoving
    }
    
    var progressLabel: String {
        let pct = Int(round((progress ?? 0) * 100))
        return "\(pct)%"
    }
    
    var motorErrorLabel: String {
        if motorError == 1.0 {
            return "ERROR: Motor not responding."
        } else if motorError == 2.0 {
            return "ERROR: Motor calibration failed."
        } else if motorError == 3.0 {
            return "ERROR: Motor control failed."
        }
        
        return ""
    }
    
    var pressureSensorErrorLabel: String {
        if pressureSensorError == 1.0 {
            return "ERROR: Pressure sensor disconnected."
        }
        
        return ""
    }
    
    func resumeIfPaused() {
        switch mode {
        case .some(.eversionPaused): setMode(.eversion)
        case .some(.inversionPaused): setMode(.inversion)
        default: break
        }
    }
}

#Preview {
    ContentView()
}
