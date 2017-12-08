#include "ofApp.h"
#include "CameraParams.h"

using namespace std;

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetVerticalSync(true);
    //cout << "Target: " << cam.getTarget() << endl;
  ofBackground(ofColor::black);
  receive.setup(PORT);
  
  // Load GUI from a pre-saved XML file.
  gui.setup();
    
    cam.setNearClip(15.0f);
    cam.setFarClip(12000.0f);

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
  cvGroup.add(blurVal.setup("Blur", 10, 0, 20));
  gui.add(&cvGroup);
  
  // tracker GUI.
  trackerGroup.setup("Tracker");
  trackerGroup.add(persistence.setup("Persistence", 60, 0, 300));
  trackerGroup.add(maxDistance.setup("Max Distance", 300, 100, 1200));
  trackerGroup.add(smoothingRate.setup("Smoothing Rate", 0.5f, 0, 100.0f));
  gui.add(&trackerGroup);

  // Restore the GUI from XML file.
  gui.loadFromFile("KinectInteractive.xml");

  // "What do you desire" by Alan Watts sample to be played. 
  audioPlayer.addSample("/Users/amay/Documents/of_v20170714_osx_release/apps/myApps/KinectInteractiveSound/bin/data/1.wav");
}

//--------------------------------------------------------------
void ofApp::update(){

  // Set contourFinder with sliders to get the right value for contour finding.
  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);
  
  // Set a smoothing rate for the tracked rectangle.
  contourFinder.getTracker().setSmoothingRate(smoothingRate);
  
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
      blur((*currentVidPlayer), blurVal);
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
        // Get depth pixels.
        depthPixels = kinect -> getDepthPixels();
        blur(depthPixels, blurVal);
        rawDepthPixels = kinect -> getRawDepthPixels();
        
        // Create the depth texture from Kinect pixels.
        texDepth.loadData(depthPixels);
        depthImgMat = ofxCv::toCv(depthPixels);

        // Find contours.
        contourFinder.findContours(depthImgMat);
        tracker.track(contourFinder.getBoundingRects());
        // Update bounded rectangles' centers' world coordinate.
        updateWorldCoordinates();
      }
    }
  
  #endif
  
  // Handle touch OSC messages. 
  processOSCMessages();
  
  audioPlayer.update();
  
  // Process tracked objects.
  // Map the perimeter of their connections to sound.
  processTrackedObjects();
}

void ofApp::processTrackedObjects() {

  vector<TrackedRect>& followers = tracker.getFollowers();
  int objectCount = followers.size();

  // Current audio playback state.
  State playbackState = audioPlayer.getPlaybackState();
  
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
    glm::vec3 worldCoordinate = followers[i].getWorldCoordinate();
    // Filter out vertices that might be 0, 0, 0.
    if (worldCoordinate != glm::vec3(0, 0, 0)) {
      trackedPoly.addVertex(followers[i].getWorldCoordinate());
    }
  }
  trackedPoly.close();
  
  // Map this polyline's parameters to sound.
  int perimeter = trackedPoly.getPerimeter();

  audioPlayer.updateSound(perimeter, objectCount);
}

// Calculate world coordinates for tracked objects' center.
void ofApp::updateWorldCoordinates() {
  vector <TrackedRect>& followers = tracker.getFollowers();
  for (int i = 0; i < followers.size(); i++) {
    int label = followers[i].getLabel();
    
    // Only update the Z distance if it's recently seen.
    // Else, we won't update the Z distance.
    if (tracker.getLastSeen(label) == 0) {
      // Center of the bounding rectangle.
      ofVec2f center = followers[i].getCenter();
        
      int pixelIndex = center.x + center.y * depthPixels.getWidth();
      float depth = rawDepthPixels[pixelIndex];
      if (depth > 0) {
        glm::vec3 worldCoordinate = depthToPointCloudPos(center.x, center.y, depth);
        cout << "Center: " << ofToString(center) << " World: " << ofToString(worldCoordinate) << " Raw Depth : " << depth << endl;
           followers[i].setWorldCoordinate(worldCoordinate);
      }
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  if (hideGui) {
    gui.draw();
  }
    
  ofDrawBitmapString(trackedPoly.getPerimeter(), 10, 500);
  
  cam.begin();
    // Texture.
    if (showTexture) {
        texDepth.draw(0, 0);
    }
    
    int imageHeight = depthPixels.getHeight();

    //ofDrawAxis(10);
    
    ofPushMatrix();
    
      ofScale(1, -1, 1);
      ofTranslate(0, -imageHeight, 0);
    
      // Contours.
      if (showContours) {
        contourFinder.draw();
      }
  
    ofPopMatrix();
  
    ofPushMatrix();
        ofTranslate(glm::vec3(0, 0, 300));
        ofScale(3, 3, 2);
        // Point cloud. Translate and draw the point cloud at 0, 0, 0
        if (showPointCloud) {
            drawPointCloud();
        }
        
        // Followers
        if (showFollowers) {
            ofPushMatrix();
                ofScale(1, -1, -1);
                ofTranslate(0, 0, 0);
                ofPushStyle();
                    ofSetLineWidth(3.0f);
                    ofSetColor(ofColor::white);
                    // Poly between followers.
                    trackedPoly.draw();
                    vector<TrackedRect>& followers = tracker.getFollowers();
                    for (int i = 0; i < followers.size(); i++) {
                        glm::vec3 worldCoordinate = followers[i].getWorldCoordinate();
                        ofEnableDepthTest();
                        // Filter out these followers popping up at 0, 0, 0
                        if (worldCoordinate != glm::vec3(0, 0, 0)) {
                          followers[i].draw();
                        }
                        ofDisableDepthTest();
                    }
                ofPopStyle();
            ofPopMatrix();
        }
    ofPopMatrix();
    
  cam.end();
}

void ofApp::drawPointCloud() {
  int w = depthPixels.getWidth();
	int h = depthPixels.getHeight();
  
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
          int pixelIndex = x + y * w;
          
          // Use raw depth pixels to find the actual depth seen by the
          // camera.
          float depth = rawDepthPixels[pixelIndex];
          glm::vec3 vertex = depthToPointCloudPos(x, y, depth);
          mesh.addVertex(vertex);
		}
	}
  
    // Size of the point.
	glPointSize(1);
	ofPushMatrix();
        // Projected points are 'upside down' and 'backwards'
        ofScale(1, -1, -1);
        ofTranslate(0, 0, 0); // center the points a bit
        ofEnableDepthTest();
        mesh.draw();
        ofDisableDepthTest();
	ofPopMatrix();
}

// Calculate the actual point based
glm::vec3 ofApp::depthToPointCloudPos(int x, int y, float depthValue) {
  glm::vec3 point;
  point.z = (depthValue);// / (1.0f); // Convert from mm to meters
  point.x = (x - CameraParams::cx) * point.z / CameraParams::fx;
  point.y = (y - CameraParams::cy) * point.z / CameraParams::fy;
  return point;
}

void ofApp::keyPressed(int key) {
  // 1
  if (key == 49) {
    // Empty - Not mapped to anything currently.
    showPointCloud = !showPointCloud;
  }
  
  // 2
  if (key == 50) {
    // Hide texture
    showTexture = !showTexture;
  }
  
  // 3
  if (key == 51) {
    // Hide contours
    showContours = !showContours;
  }
  
  // 4
  if (key == 52) {
    // Hide trackers
    showFollowers = !showFollowers;
  }
  
  // 5
  if (key == 53) {
    hideGui = !hideGui;
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
      showFollowers = !showFollowers;
    }
    
    if (m.getAddress() == "/3/toggle2") {
      showPointCloud = !showPointCloud;
    }
    
    if (m.getAddress() == "/3/toggle3") {
      showTexture = !showTexture;
    }
    
    if (m.getAddress() == "/3/toggle4") {
      showContours = !showContours;
    }
  }
}

void ofApp::exit(){
  gui.saveToFile("KinectInteractive.xml");
}
