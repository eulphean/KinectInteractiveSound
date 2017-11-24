#pragma once

#include "ofMain.h"
#include "TrackedRect.h"
#include "AudioPlayer.h"
#include "ofxKinectV2.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxPDSP.h"
#include "ofxCv.h"

#define PORT 8000

// Uncoment this if you don't want to use the test bench. 
#define _USE_VIDEO

class ofApp : public ofBaseApp{

	public:
    const int pixelSkip = 10;
    const int minBrightness = 2500;

		void setup();
		void update();
		void draw();
    void keyPressed(int key);
    void exit();
    void processOSCMessages();
  
    // Kinect parameters.
    ofxKinectV2 * kinect;
    ofTexture texDepth;
    ofPixels depthPixels;
  
    float avgX;
    float avgY;
    float avgBrightness;
  
    // Touch OSC parameters.
    ofxOscReceiver receive;
    ofVec2f mappedOsc;
  
    // Audio player responsible to play the sample.
    AudioPlayer audioPlayer;
  
    // GUI.
    ofxPanel gui;
    ofxGuiGroup kinectGroup;
    ofxGuiGroup cvGroup;
    ofxFloatSlider minArea, maxArea, threshold;
  
    // Contour Finder.
    ofxCv::ContourFinder contourFinder;
    ofxCv::RectTrackerFollower<TrackedRect> tracker;
  
    ofEasyCam cam;
  
    #ifdef _USE_VIDEO
  
      ofVideoPlayer vidPlayer1;
      ofVideoPlayer vidPlayer2;
      ofVideoPlayer vidPlayer3;
      ofVideoPlayer vidPlayer4;
  
      ofVideoPlayer *currentVidPlayer;
  
    #endif
  
    // Tracked objects
    ofFbo following;
};

