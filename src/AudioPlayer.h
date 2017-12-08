
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include  "ofxGui.h"

enum State {
  playing = 0,
  paused,
  stopped
};

class AudioPlayer : public pdsp::Patchable {
    
public:
    AudioPlayer() { patch(); } 
    AudioPlayer( const AudioPlayer & other ) { patch(); }
  
    // Check to see if it's time to move to the next sample.
    void update();
  
    // Stop and play the sample.
    void addSample(string path);
  
    // Update sound effects based on human movement. 
    void updateSound(float perimeter, int objectCount);
  
    // Play, Pause, and Stop
    void play();
    void pause();
    void stop();
    void setSampleGain(float gain);
    void drawGui();
    State getPlaybackState();
    
private:
    // State of the system.
    State sampleState;
  
    // Current sample
    int sampleIdx;
  
    // 0 - Pitch, 1 - Delay, 2 - Decimation
    // Cycle with each person.
    int effectState;
  
    // Update sound constants.
    int minPerimeter = 2000;
    int maxPerimeter = 7000;
    // Currently, we maintain 2 effects. Pitch and decimation.
    // TODO: Also, add delay.
    const int totalEffects = 3;
  
    // PDSP parameters.
    ofxPDSPEngine engine;
  
    // Sampler
    pdsp::Sampler  sampler;
    vector<pdsp::SampleBuffer*> samples;
  
    // Amp
    pdsp::Amp  amp;

    // Triggers.
    pdsp::ADSR      env;
    ofxPDSPTrigger envGate;
    ofxPDSPTrigger sampleTrig;
  
    // Effects.
    pdsp::Decimator  decimator;
    pdsp::DampedDelay delay;
  
    // GUI
    ofxPanel gui;
    ofxPDSPValue  osc_seek_ctrl;
    ofParameterGroup  osc1_group;
  
    // Get playhead position.
    float getMeterPosition();
  
    void updateEffectState(int objectCount);
  
    void patch();
};   
    
