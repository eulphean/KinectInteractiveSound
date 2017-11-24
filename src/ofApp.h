#pragma once

#include "ofMain.h"
#include "AudioPlayer.h"
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
    void keyPressed(int key);
  
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
    ofxOscReceiver receive;
    ofVec2f mappedOsc;
  
    // Audio player responsible to play the sample.
    AudioPlayer audioPlayer;
  
    ofxPanel gui;
    ofxFloatSlider brightness;
};

