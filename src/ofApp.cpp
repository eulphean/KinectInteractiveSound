#include "ofApp.h"

using namespace std;

using namespace ofxCv;
using namespace cv;

const float dyingTime = 1;

void Glow::setup(const cv::Rect& track) {
	color.setHsb(ofRandom(0, 255), 255, 255);
	cur = toOf(track).getCenter();
	smooth = cur;
  brightness = smooth;
}

void Glow::update(const cv::Rect& track) {
	cur = toOf(track).getCenter();
	smooth.interpolate(cur, .5);
	all.addVertex(smooth.x, smooth.y);
}

void Glow::kill() {
	float curTime = ofGetElapsedTimef();
	if(startedDying == 0) {
		startedDying = curTime;
	} else if(curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void Glow::draw() {
	ofPushStyle();
	float size = 16;
	ofSetColor(255);
	if(startedDying) {
		ofSetColor(ofColor::red);
		size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
	}
	ofNoFill();
	ofDrawCircle(cur, size);
	ofSetColor(color);
	all.draw();
	ofSetColor(255);
	ofDrawBitmapString(ofToString(label), cur);
	ofPopStyle();
}

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
  
  // wait for half a frame before forgetting something
	tracker.setPersistence(300);
	// an object can move up to 50 pixels per frame
	tracker.setMaximumDistance(400);

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
  
  // Handle touch OSC messages. 
  processOSCMessages();
  
  // Update Audio player.
  audioPlayer.update();
  
  // TODO: Enable this, when GUI is used.
  //audioPlayer.updateSound(brightness);
}

//--------------------------------------------------------------
void ofApp::draw(){
    // Draw the depth texture.
    texDepth.draw(0, 0);
    gui.draw();
  
    vector<Glow>& followers = tracker.getFollowers();
    cout << "Tracking - " << followers.size() << endl;
    for(int i = 0; i < followers.size(); i++) {
     followers[i].draw();
     int pixelIndex = followers[i].brightness.x + followers[i].brightness.y * depthPixels.getWidth();
     ofVec2f centerPos = followers[i].brightness;
     int brightness = depthPixels.getColor(centerPos.x, centerPos.y).getBrightness();
     ofDrawCircle(centerPos.x, centerPos.y, 10);
    }
    //audioPlayer.drawGui();
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
      
      /*mappedOsc.x = ofMap(oscX, 0, 1, 0.1f, 0.9f);
      audioPlayer.setSeekPosition(mappedOsc.x);*/
      
      mappedOsc.y = ofMap(oscY, 0, 1, -20.0f, 30.0f);
      audioPlayer.setGain(mappedOsc.y);
    }
  }
}

void ofApp::exit(){
  gui.saveToFile("KinectInteractive.xml");
}

/*

    We will ditch this logic completely now. We don't care about 
    somebody moving close to the Kinect and getting this average brightness. 
    First we want to calculate the bounding box of the person moving around the
    Kinect. Then we can probably use this logic. 
    
        
    int sumBrightness = 0;
    int numBrightPixels = 0;
 
 
        int height = depthTexture.getHeight();
        int width = depthTexture.getWidth();
        
        for (int x = 0; x < width; x += pixelSkip) {
          for (int y = 0; y < height; y += pixelSkip) {
            int pixelIndex = x + y * width;
            // Every pixel value is the brightness in a depth texture.
            int pixelVal = (int) pixels[pixelIndex];
            if (pixelVal > 0) {
              numBrightPixels++;
              sumBrightness += pixelVal;
            }
          }
        }
        
        // Potential to update sound if something got visible in the range of the
        // Kinect.
        if (sumBrightness > 0) {
          // Average brightness of the image.
          avgBrightness = sumBrightness / numBrightPixels;
          
          // Update sound.
          audioPlayer.updateSound(avgBrightness);
        }
*/

