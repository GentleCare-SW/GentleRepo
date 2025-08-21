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

struct ContentView: View {
    @StateObject private var vessel = RemoteVessel()
    @State private var showDevInfo = false
    
    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(spacing: 16) {
                    StatusHeader(vessel: vessel)
                    ControlCard(vessel: vessel)
                    ChamberSelector(vessel: vessel)
                    ProgressSection(vessel: vessel)
                    DeveloperInfoSection(vessel: vessel, expanded: $showDevInfo)
                }
                .padding(16)
            }
            .background(.appBackground)
            .navigationTitle("Control Panel")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    ConnectivityPill(statusText: vessel.bluetoothStatus, isConnected: vessel.isConnected)
                }
            }
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
                    if vessel.isConnected {
                        Text("Mode: \(vessel.modeDisplay)")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
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
                        vessel.setMode(.eversion)
                        haptic(.rigid)
                    } label: {
                        Label("Evert", systemImage: "arrow.down.left.and.arrow.up.right")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.blue)
                    .disabled(!vessel.canEvert)
                    
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

private struct DeveloperInfoSection: View {
    @ObservedObject var vessel: RemoteVessel
    @Binding var expanded: Bool
    
    var body: some View {
        Card {
            DisclosureGroup(isExpanded: $expanded) {
                Grid(alignment: .leading, horizontalSpacing: 12, verticalSpacing: 10) {
                    DevRow("Air Pressure (PSI)", vessel.airPressure, suffix: nil, formatting: .number)
                    DevRow("Voltage (%)", vessel.voltagePercentage, suffix: "%", formatting: .number)
                    DevRow("Motor Position (rad)", vessel.motorPosition, suffix: nil, formatting: .number)
                    DevRow("Motor Velocity (rad/s)", vessel.motorVelocity, suffix: nil, formatting: .number)
                    DevRow("Motor Torque (Nm)", vessel.motorTorque, suffix: nil, formatting: .number)
                    DevRow("Servo Chamber", vessel.servoChamber, suffix: nil, formatting: .integer)
                    DevRow("Mode", vessel.mode?.rawValue, suffix: nil, formatting: .string)
                    DevRow("Bluetooth Status", vessel.bluetoothStatus, suffix: nil, formatting: .string)
                }
                .padding(.top, 8)
            } label: {
                HStack {
                    Text("Developer Info")
                        .font(.title3).bold()
                    Spacer()
                }
            }
        }
        .accessibilityElement(children: .contain)
    }
}

private struct Card<Content: View>: View {
    @ViewBuilder var content: Content
    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            content
        }
        .padding(16)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.thickMaterial, in: RoundedRectangle(cornerRadius: 16, style: .continuous))
        .overlay(
            RoundedRectangle(cornerRadius: 16, style: .continuous)
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
    var body: some View {
        Circle()
            .fill(isOK ? .green : .red)
            .frame(width: 14, height: 14)
            .overlay(Circle().stroke(.white, lineWidth: 2))
            .shadow(radius: 1, x: 0, y: 1)
    }
}

private struct ConnectivityPill: View {
    var statusText: String
    var isConnected: Bool
    var body: some View {
        HStack(spacing: 6) {
            Circle().fill(isConnected ? .green : .gray)
                .frame(width: 8, height: 8)
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
    @State private var scale: CGFloat = 1.0
    var body: some View {
        Circle()
            .fill()
            .frame(width: 8, height: 8)
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
