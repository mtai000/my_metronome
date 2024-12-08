//
//  MetronomeClass.swift
//  metronome
//
//  Created by macmini on 2024/12/8.
//
import AVFoundation

class MetronomeClass: ObservableObject {
    private var audioEngine     : AVAudioEngine!
    private var playerNode      : AVAudioPlayerNode!
    private var audioBuffer     : AVAudioPCMBuffer!
    private var audioFile       : AVAudioFile!
    private var nextBeatTime    : AVAudioTime!
    private var bpmChangeWait   : DispatchWorkItem?
    private var metronomeTimer  : DispatchSourceTimer?
    
    var max_bpm: Int = 200
    var min_bpm: Int = 30
    @Published var bpm: Int = 60 {
        didSet{
                slowUpdateBeats()
        }
    }
    
    @Published var tickCount: Int = 0
    @Published var isRunning = false;
    
    init(){
        setupAudio()
    }
    
    func bpm_update(isAdd:Bool,val:Int)
    {
        if isAdd {
            bpm = min(max_bpm,bpm + val)
            slowUpdateBeats()
        } else {
            bpm = max(min_bpm, bpm - val)
            slowUpdateBeats()
        }
    }
    private func slowUpdateBeats()
    {
        if isRunning == false {return}
        self.bpmChangeWait?.cancel()
        bpmChangeWait = DispatchWorkItem{
            self.updateBeats()
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.3, execute: self.bpmChangeWait!)
    }
    private func setupAudio()
    {
        audioEngine = AVAudioEngine()
        playerNode = AVAudioPlayerNode()
        
        audioEngine.attach(playerNode)
        
        let mixer = audioEngine.mainMixerNode
        audioEngine.connect(playerNode, to: mixer, format: nil)
        
        guard let url = Bundle.main.url(forResource: "ding", withExtension: "wav") else {
            print("Not found voice file")
            return
        }
        
        audioFile = try! AVAudioFile(forReading: url)
        DebugLog.shared.cout(audioFile.length)
        audioBuffer = AVAudioPCMBuffer(pcmFormat: audioFile.processingFormat, frameCapacity: AVAudioFrameCount(audioFile.length))!
        try! audioFile.read(into: audioBuffer)
        
        do{
            try audioEngine.start()
        }catch{
            DebugLog.shared.cout("Fail to start audio engine:\(error.localizedDescription)")
        }
    }
    
    func startMetronome(){
        DebugLog.shared.cout("start metronome")
        self.playerNode.stop()
        self.nextBeatTime = nil
        tickCount = 0
        playerNode.play()

        metronomeTimer = DispatchSource.makeTimerSource(queue: .main)
        metronomeTimer?.schedule(deadline: .now(), repeating: 60.0 / Double(bpm))
        metronomeTimer?.setEventHandler { [weak self] in
            guard let self = self else { return }
            self.scheduleBeats()
        }
        metronomeTimer?.resume()
    }
    
    func stopMetronome(){
        DebugLog.shared.cout("stop metronome")
        DispatchQueue.main.async {
            self.playerNode.stop()
            self.nextBeatTime = nil
            self.metronomeTimer?.cancel()
            self.metronomeTimer = nil
        }
    }
        
    private func scheduleBeats(){
        let interval = 60.0 / Double(bpm)
        let framesPerBeat = AVAudioFramePosition(interval * audioFile.processingFormat.sampleRate)
        
        guard let lastRenderTime = audioEngine.outputNode.lastRenderTime else {
            DebugLog.shared.cout("last Render Time nil")
            return
        }
        
        guard let playerTime = playerNode.playerTime(forNodeTime: lastRenderTime) else {
            DebugLog.shared.cout("playerTime nil")
            return
        }
        
        DebugLog.shared.cout("\n\t\t=======================")
        DebugLog.shared.cout("Current bpm value         : \(bpm)")
        DebugLog.shared.cout("interval                  : \(interval)")
        DebugLog.shared.cout("frames Per Beat           : \(framesPerBeat)")
        var nextSampleTime : AVAudioFramePosition
        
        if nextBeatTime == nil {
            nextSampleTime = playerTime.sampleTime + framesPerBeat
        } else {
            nextSampleTime = nextBeatTime!.sampleTime
        }
        
        DebugLog.shared.cout("next Sample Time: \(nextSampleTime)")
        for _ in 0..<min(3,bpm/10) {
            let scheduleTime = AVAudioTime(sampleTime: nextSampleTime, atRate: audioFile.processingFormat.sampleRate)
            playerNode.scheduleFile(audioFile, at: scheduleTime,completionHandler: nil)
            nextSampleTime += framesPerBeat
        }
        
        nextBeatTime = AVAudioTime(sampleTime: nextSampleTime, atRate: audioFile.processingFormat.sampleRate)
    }
    
    
    private func updateBeats() {
        DispatchQueue.main.async {
              self.metronomeTimer?.cancel()
              self.startMetronome()
        }
    }
}
