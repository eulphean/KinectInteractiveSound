#pragma once

#include "ofMain.h"
#include "ofxKinectV2.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxPDSP.h"

#define PORT 8000

class ofApp : public ofBaseApp{

	public:
    const int pixelSkip = 10;
    const int minBrightness = 2500;

		void setup();
		void update();
		void draw();
  
    // Sound methods. 
    void updateSound();
  
    // Kinect parameters.
    ofxPanel panel;
    ofxKinectV2 * kinect;
    ofTexture depthTexture;
  
    float avgX;
    float avgY;
    float avgBrightness;
  
  
    // Touch OSC parameters.
    void setCurrentTrackAndPlay(int val, ofSoundPlayer * newCurrentTrack);
    ofxOscReceiver receive;
    ofVec2f mappedOsc;
    ofSoundPlayer *currentTrack = NULL;
  
    ofSoundPlayer track1;
    ofSoundPlayer track2;
    ofSoundPlayer track3;
  
    // PDSP parameters.
    ofxPDSPEngine   engine;
    
    pdsp::Sampler       sampler0;
    pdsp::Sampler       sampler1;
    pdsp::Amp           amp0;
    pdsp::Amp           amp1;
    
    pdsp::SampleBuffer  sample;
    pdsp::ADSR          env;
    
    ofxPDSPTrigger envGate;
    ofxPDSPTrigger sampleTrig;
    
    pdsp::Decimator     decimator;
  
    pdsp::BitNoise    noise;
  
  float noiseGain;
  
    // Method to play the track with PDSP add on.
    void playWithPDSP(int val, int trackNum);
  
    // Speed or decimator mode.
    bool isDecimatorMode = false;
  
    bool isPlaying = false;
};
