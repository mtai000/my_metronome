//
//  debug.swift
//  metronome
//
//  Created by macmini on 2024/12/8.
//
import Foundation
final class DebugLog{
    static let shared = DebugLog()
    
    private init() {}
    
    func cout(_ message: Any) {
        #if DEBUG
        print(message)
        #endif
    }
    
}
