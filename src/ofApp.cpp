#include "ofApp.h"

using namespace std;

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
  ofBackground(ofColor::black);
  receive.setup(PORT);
  
  // Load GUI from a pre-saved XML file.
  gui.setup();

  #ifdef _USE_VIDEO
  
    // Preload all test videos if we are using the videos.
    vidPlayer1.load("1.mov");
    vidPlayer2.load("2.mov");
    vidPlayer3.load("3.mov");
    vidPlayer4.load("4.mov");
  
    // Assign current video player to 1st video player.
    currentVidPlayer = &vidPlayer1;
  
    // Play the first test video.
    currentVidPlayer -> play();

  #else 
  
    // Check if we have a Kinect device connected.
    ofxKinectV2 tmp;
    vector <ofxKinectV2::KinectDeviceInfo> deviceList = tmp.getDeviceList();
  
    if (deviceList.size() > 0) {
      cout << "Success: Kinect found.";
    } else {
      cout << "Failure: Kinect not found.";
      return;
    }
  
    kinectGroup.setup("Kinect");
  
    // Setup Kinect. [Assumption] Only a single Kinect will be
    // connected to the system.
    kinect = new ofxKinectV2();
    kinect->open(deviceList[0].serial);
    kinectGroup.add(kinect->params);

    gui.add(&kinectGroup);
  
  #endif
  
  // openCv GUI.
  cvGroup.setup("ofxCv");
  cvGroup.add(minArea.setup("Min area", 10, 1, 100));
  cvGroup.add(maxArea.setup("Max area", 200, 1, 500));
  cvGroup.add(threshold.setup("Threshold", 128, 0, 255));
  
  // Tracker properties.
  
	tracker.setPersistence(120);
	tracker.setMaximumDistance(300);

  // Add the groups to main GUI.
  gui.add(&cvGroup);

  // Restore the GUI from XML file.
  gui.loadFromFile("KinectInteractive.xml");

  // "What do you desire" by Alan Watts sample to be played. 
  audioPlayer.addSample("/Users/amaykataria/Documents/of_v20170714_osx_release/apps/Silo/KinectInteractiveSound/bin/data/1.wav");
}

//--------------------------------------------------------------
void ofApp::update(){

  // Set contourFinder with sliders to get the right value for contour finding.
  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);
  
  // Depth image matrix that we will pass to Contour finder.
  cv::Mat depthImgMat;
  
  #ifdef _USE_VIDEO

    // Update video player.
    currentVidPlayer -> update();
  
    // Video player related updates.
    if (currentVidPlayer -> isFrameNew()) {
      blur((*currentVidPlayer), 10);
      depthPixels = currentVidPlayer -> getPixels();
      texDepth.loadData(depthPixels);
      depthImgMat = ofxCv::toCv(depthPixels);
      
      // Offset distances to center the polyline.
      //widthOffset = ofGetWidth()/2 - depthPixels.getWidth()/2;
      //heightOffset = ofGetHeight()/2 - depthPixels.getHeight()/2;
      
      // Find contours.
      contourFinder.findContours(depthImgMat);
      tracker.track(contourFinder.getBoundingRects());
      
      // This is to center the polyline. Let's see if I need this. I should setup
      // the video such that it takes the entire screen pretty much. Like full screen.
      // updatePolyline(widthOffset, heightOffset);
    }

  #else
  
    if (kinect != NULL){
      // Update Kinect.
      kinect->update();
      if( kinect->isFrameNew()){
        // Do work here with the data obtained from Kinect.
        
        // Get depth pixels.
        ofPixels pixels = kinect -> getDepthPixels();
        
        // Create the depth texture from Kinect pixels.
        texDepth.loadData(pixels);
      }
    }
  
  #endif
  
  // Update Z Distance of tracked objects based on the brightness
  // of these objects.
  updateZDistances();
  
  // Handle touch OSC messages. 
  processOSCMessages();
  
  // Update Audio player.
  audioPlayer.update();
  
  // TODO: Enable this, when GUI is used.
  //audioPlayer.updateSound(brightness);
}

// Set Z distance for the Tracked objects.
void ofApp::updateZDistances() {
  vector <TrackedRect>& followers = tracker.getFollowers();
  for (int i = 0; i < followers.size(); i++) {
    int label = followers[i].getLabel();
    
    // Only update the Z distance if it's recently seen.
    // Else, we won't update the Z distance.
    if (tracker.getLastSeen(label) == 0) {
      // Center of the bounding rectangle.
      ofVec2f center = followers[i].getCenter();
      
      // Brightness of the pixel at center.
      int pixelIndex = center.x + center.y * depthPixels.getWidth();
      int brightness = depthPixels.getColor(center.x, center.y).getBrightness();
      
      // Map the distance in Z coordinate.
      int z = ofMap (brightness, 255, 0, 300, -300, true);
      followers[i].updateCenterWithZ(z);
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  cam.begin();
  
    ofScale(2, 2, 2);
    
    ofDrawAxis(10);
    texDepth.draw(0, 0);
    
    //contourFinder.draw();
    
    // Get the followers and draw them.
    vector<TrackedRect>& followers = tracker.getFollowers();
    for (int i = 0; i < followers.size(); i++) {
      followers[i].draw();
    }
    
  cam.end();
}

void ofApp::keyPressed(int key) {
  // --------------- AUDIO ----------------------

  // 1
  if (key == 49) {
    audioPlayer.play();
  }
  
  // 2
  if (key == 50) {
    audioPlayer.pause();
  }
  
  // 3
  if (key == 51) {
    audioPlayer.stop();
  }
  
  // 4
  if (key == 52) {
    bool pitchMode = audioPlayer.getPitchMode();
    audioPlayer.setPitchMode(!pitchMode);
  }
  
  // --------------- VIDEO ----------------------
  
  // 5
  if (key == 53) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer1;
      currentVidPlayer -> play();
    }
  }
  
  // 6
  if (key == 54) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer2;
      currentVidPlayer -> play();
    }
  }
  
  // 7
  if (key == 55) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer2;
      currentVidPlayer -> play();
    }
  }
  
  // 8
  if (key == 56) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer3;
      currentVidPlayer -> play();
    }
  }
  
  // 9
  if (key == 57) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> setPaused(true);
    } else {
      currentVidPlayer -> setPaused(false);
    }
  }
}

void ofApp::processOSCMessages() {
  // Touch OSC updates.
  while (receive.hasWaitingMessages()) {
    ofxOscMessage m;
    // Set the next message.
    #pragma warning(disable: WARNING_CODE)
    receive.getNextMessage(&m);
    
    if (m.getAddress() == "/3/toggle1") {
      int val = m.getArgAsInt(0);
      if (val) {
        audioPlayer.play();
      }
    }
    
    if (m.getAddress() == "/3/toggle2") {
      int val = m.getArgAsInt(0);
      if (val) {
        audioPlayer.pause();
      }
    }
    
    if (m.getAddress() == "/3/toggle3") {
      int val = m.getArgAsInt(0);
      if (val) {
        audioPlayer.stop();
      }
    }
    
    if (m.getAddress() == "/3/toggle4") {
      int val = m.getArgAsInt(0);
      audioPlayer.setPitchMode(val);
    }
    
    if (m.getAddress() == "/3/xy") {
      float oscX = m.getArgAsFloat(0);
      float oscY = m.getArgAsFloat(1);
      
      mappedOsc.y = ofMap(oscY, 0, 1, -20.0f, 30.0f);
      audioPlayer.setGain(mappedOsc.y);
    }
  }
}

void ofApp::exit(){
  gui.saveToFile("KinectInteractive.xml");
}

/*
/ We want to see the tracker bubbles in 3d.
    ofPushMatrix();
      
      ofScale(2, 2, 2);
      ofDrawAxis(20);
      texDepth.draw(0, 0);
      contourFinder.draw();
      gui.draw();
      //vector<TrackedRect>& followers = tracker.getFollowers();
      vector<cv::Rect> followers = contourFinder.getBoundingRects();
      for(int i = 0; i < followers.size(); i++) {
        ofRectangle boundingRect = toOf(followers[i]);
       
       // Get the brightness in center.
       ofVec2f center = boundingRect.getCenter();
       
       // Index of the center pixel of bounding rectangle.
       int pixelIndex = center.x + center.y * depthPixels.getWidth();
       int brightness = depthPixels.getColor(center.x, center.y).getBrightness();
       int z = ofMap (brightness, 255, 0, 300, -300, true);
       
       if (brightness > 0) {
         ofPushMatrix();
           ofTranslate(center.x, center.y, z);
           ofPushStyle();
            ofEnableAlphaBlending();
            ofSetColor(ofColor::white, 127);
            ofDrawBox(0, 0, 0, boundingRect.width, boundingRect.height, 15);
            ofDisableAlphaBlending();
           ofPopStyle();
         ofPopMatrix();
       }
      }
  
    ofPopMatrix();
    cam.end();
    //audioPlayer.drawGui();
    */
