#include "ofApp.h"
#include "CameraParams.h"

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
    vidPlayer5.load("5.mov");
  
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
  cvGroup.setup("Contour");
  cvGroup.add(minArea.setup("Min area", 10, 1, 100));
  cvGroup.add(maxArea.setup("Max area", 200, 1, 500));
  cvGroup.add(threshold.setup("Threshold", 128, 0, 255));
  gui.add(&cvGroup);
  
  // tracker GUI.
  trackerGroup.setup("Tracker");
  trackerGroup.add(persistence.setup("Persistence", 120, 30, 800));
  trackerGroup.add(maxDistance.setup("Max Distance", 300, 100, 1200));
  gui.add(&trackerGroup);

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
  
  // Set Tracker properties.
  tracker.setPersistence(persistence);
  tracker.setMaximumDistance(maxDistance);
  
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
      
      // Find contours.
      contourFinder.findContours(depthImgMat);
      tracker.track(contourFinder.getBoundingRects());
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
  
  // Process tracked objects = Map to sound. 
  processTrackedObjects();
}

void ofApp::processTrackedObjects() {
  // Current audio playback state.
  State playbackState = audioPlayer.getPlaybackState();
  
  vector<TrackedRect>& followers = tracker.getFollowers();
  // Somebody entered the room, play audio.
  if (followers.size() > 0) {
    if (playbackState != playing) {
      audioPlayer.play();
    }
  } else {
    // Nobody is in the room, stop the audio.
    audioPlayer.stop();
  }
  
  // Clear the poly and recreate it.
  trackedPoly.clear();
  for (int i = 0; i < followers.size(); i++) {
    trackedPoly.addVertex(followers[i].getCenter());
  }
  trackedPoly.close();
  
  // Map this polyline's parameters to sound.
  int perimeter = trackedPoly.getPerimeter();

  audioPlayer.updateSound(perimeter);
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
      int brightness = depthPixels.getColor(center.x, center.y).getBrightness();
      
      // Map the distance in Z coordinate.
      int z = ofMap (brightness, 255, 0, 0, -300, true);
      followers[i].updateCenterWithZ(z);
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  gui.draw();
  
  cam.begin();
    // Texture.
    texDepth.draw(0, 0);
    int imageHeight = depthPixels.getHeight();
  
    ofDrawAxis(10);
    ofPushMatrix();
      ofScale(1, -1, 1);
      ofTranslate(0, -imageHeight, 0);
      // Contours.
      contourFinder.draw();
      ofPushStyle();
        vector<TrackedRect>& followers = tracker.getFollowers();
        // Followers.
        for (int i = 0; i < followers.size(); i++) {
          followers[i].draw();
        }
        ofSetColor(ofColor::white);
        // Poly between followers.
        trackedPoly.draw();
      ofPopStyle();
    ofPopMatrix();
  
    ofDrawBitmapString(trackedPoly.getPerimeter(), 100, 100, 0);
  
    if (showPointCloud) {
      drawPointCloud();
    }
    
  cam.end();
}

void ofApp::drawPointCloud() {
  ofDrawAxis(10);
  int w = depthPixels.getWidth();
	int h = depthPixels.getHeight();
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
      int brightness = depthPixels.getColor(x, y).getBrightness();
      
      // Map the distance in Z coordinate.
      int z = ofMap (brightness, 255, 0, 0, 4500, true);
      if (brightness > 100) {
        glm::vec3 vertex = depthToPointCloudPos(x, y, z);
        mesh.addColor(brightness);
        mesh.addVertex(vertex);
      }
		}
	}
  
	glPointSize(1);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards' 
	ofScale(1, -1, -1);
	//ofTranslate(0, 0, -1000); // center the points a bit
	ofEnableDepthTest();
	mesh.draw();
	ofDisableDepthTest();
	ofPopMatrix();
}

//calculte the xyz camera position based on the depth data
glm::vec3 ofApp::depthToPointCloudPos(int x, int y, float depthValue) {
  glm::vec3 point;
  point.z = (depthValue);// / (1.0f); // Convert from mm to meters
  point.x = (x - CameraParams::cx) * point.z / CameraParams::fx;
  point.y = (y - CameraParams::cy) * point.z / CameraParams::fy;
  return point;
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
    // Empty - Not mapped to anything currently.
    showPointCloud = !showPointCloud;
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
      currentVidPlayer = &vidPlayer3;
      currentVidPlayer -> play();
    }
  }
  
  // 8
  if (key == 56) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer4;
      currentVidPlayer -> play();
    }
  }
  
  // 9
  if (key == 57) {
    if (currentVidPlayer -> isPlaying()) {
      currentVidPlayer -> stop();
      currentVidPlayer = &vidPlayer5;
      currentVidPlayer -> play();
    }
  }
  
   // 0
  if (key == 48) {
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
      // Empty. Not mapped to anything.
    }
    
    if (m.getAddress() == "/3/xy") {
      float oscX = m.getArgAsFloat(0);
      float oscY = m.getArgAsFloat(1);
      
      mappedOsc.y = ofMap(oscY, 0, 1, -20.0f, 30.0f);
      audioPlayer.setSampleGain(mappedOsc.y);
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
