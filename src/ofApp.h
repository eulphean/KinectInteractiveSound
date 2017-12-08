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
// #define _USE_VIDEO

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
    void updateWorldCoordinates();
    void processTrackedObjects();
  
    // Point Cloud.
    glm::vec3 depthToPointCloudPos(int x, int y, float z);
    void drawPointCloud();
  
    // Kinect parameters.
    ofxKinectV2 * kinect;
    ofTexture texDepth;
    ofPixels depthPixels;
    ofFloatPixels rawDepthPixels;
  
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
    ofxGuiGroup trackerGroup;
    ofxFloatSlider minArea, maxArea, threshold, persistence, maxDistance, blurVal, smoothingRate;
  
    // Contour Finder.
    ofxCv::ContourFinder contourFinder;
    ofxCv::RectTrackerFollower<TrackedRect> tracker;
  
    ofEasyCam cam;
  
    // Turn on/off a point cloud.
    bool showPointCloud = false;
    bool showTexture = false;
    bool showContours = false;
    bool showFollowers = false;
  
    ofVideoPlayer vidPlayer1;
    ofVideoPlayer vidPlayer2;
    ofVideoPlayer vidPlayer3;
    ofVideoPlayer vidPlayer4;
    ofVideoPlayer vidPlayer5;

    ofVideoPlayer *currentVidPlayer;
  
    // Tracked polyline.
    ofPolyline trackedPoly;
  
    bool hideGui = false;
};

