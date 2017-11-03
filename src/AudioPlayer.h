
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include  "ofxGui.h"


class AudioPlayer : public pdsp::Patchable {
    
public:
    AudioPlayer() { patch(); } 
    AudioPlayer( const AudioPlayer & other ) { patch(); }
  
    enum State {
      playing = 0,
      paused,
      stopped
    };
  
    // Check to see if it's time to move to the next sample.
    void update();
  
    // Stop and play the sample.
    void addSample(string path);
  
    // Update sound effects based on human movement. 
    void updateSound(float avgBrightness);
  
    // Play, Pause, and Stop
    void play();
    void pause();
    void stop();
  
    void setPitchMode (bool isPitchMode);
    bool getPitchMode ();
  
    void setSeekPosition (float seekPosition);
    void setGain (float gain);
  
    void drawGui();
    
private:
    // State of the system.
    State sampleState;
  
    // Current sample
    int sampleIdx;
  
    // Indicates if the pitch mode is on.
    bool pitchMode;
  
    // Update sound constants.
    const int minBrightness = 80;
    const int maxBrightness = 90;
  
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
    pdsp::BitNoise   noise;
  
    // GUI
    ofxPanel gui;
    ofxPDSPValue  osc_seek_ctrl;
    ofParameterGroup  osc1_group;
  
    // Get playhead position.
    float getMeterPosition();
  
    void killAmbientNoise();
  
    void patch();
};   
    
