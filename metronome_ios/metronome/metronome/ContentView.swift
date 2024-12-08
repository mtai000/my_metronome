//
//  ContentView.swift
//  metronome
//
//  Created by macmini on 2024/12/8.
//

import SwiftUI
import AVFoundation
import AudioToolbox

struct MetronomeView: View {
    @State private var title:String = "Metronome"
    @StateObject var m_class = MetronomeClass()
      
    var body: some View {
        VStack {
            Text("\(title)").font(.title).multilineTextAlignment(.center)
            Spacer()
            //Text("\(m_class.tickCount)").font(.system(size:14))
            Slider(value: Binding(
                get: { Double(m_class.bpm) },
                set: { m_class.bpm = Int($0) }
            ), in: Double(m_class.min_bpm)...Double(m_class.max_bpm), step: 1)
            .padding()
            
            HStack(spacing: 10) {
                Button(action: { m_class.bpm_update(isAdd: false, val: 10)}) {
                    Text("-10").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
                Button(action: { m_class.bpm_update(isAdd: false, val: 5)}) {
                    Text("-5").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
                Button(action: { m_class.bpm_update(isAdd: false, val: 1)}) {
                    Text("-1").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
                Button(action: { m_class.bpm_update(isAdd:  true, val: 1)}) {
                    Text("+1").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
                Button(action: { m_class.bpm_update(isAdd:  true, val: 5) }) {
                    Text("+5").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
                Button(action: { m_class.bpm_update(isAdd:  true, val: 10) }) {
                    Text("+10").font(.system(size:12)).padding().frame(width: 55, height: 40).background(Color.purple).foregroundColor(.white).cornerRadius(8)
                }
            }
            HStack(spacing: 50) {
                HStack {
                    Picker("Time Signature", selection: .constant(4)) {
                        Text("2/4").tag(2)
                        Text("3/4").tag(3)
                        Text("4/4").tag(4)
                    }}
                .frame(width: 80, height: 40)
                .pickerStyle(MenuPickerStyle())
                
                HStack{
                    Spacer()
                    Text("\(m_class.bpm) BPM").font(.headline)
                        .multilineTextAlignment(.center)
                        .frame(width: 80, height: 40)
                }
                
                HStack{
                    Spacer()
                    Toggle("SW", isOn: $m_class.isRunning)
                        .toggleStyle(SwitchToggleStyle(tint: .purple))
                        .onChange(of: m_class.isRunning) {
                            if m_class.isRunning {
                                m_class.startMetronome()
                            }
                            else
                            {
                                m_class.stopMetronome()
                            }
                        }
                }
            }
            Spacer()
            
        }
        .padding()
    }
}

#Preview {
    MetronomeView()
}
