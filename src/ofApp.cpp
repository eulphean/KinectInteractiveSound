#include "ofApp.h"

using namespace std;

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(30, 30, 30);
  receive.setup(PORT);
  
  // See if we have a valid Kinect hooked to the system.
  ofxKinectV2 tmp;
  vector <ofxKinectV2::KinectDeviceInfo> deviceList = tmp.getDeviceList();
  
  panel.setup("", "settings.xml", 10, 100);
  
  // Modify this if there are multiple Kinects hooked into the system.
  if (deviceList.size() > 0) {
    kinect = new ofxKinectV2();
    kinect -> open(deviceList[0].serial);
    panel.add(kinect->params);
  }
  
  panel.loadFromFile("settings.xml");
  
  // Add samples to the Audio Player to sequence through.
  audioPlayer.addSample("/Users/amay/Documents/of_v20170714_osx_release/apps/myApps/KinectInteractiveSound/bin/data/1.wav");
  
  gui.setup();
  gui.add(brightness.setup("brightness", 80, 70, 100));
  gui.setPosition(10, 10);
}

//--------------------------------------------------------------
void ofApp::update(){
  int sumBrightness = 0;
  int numBrightPixels = 0;
  
  // Kinect updates
  if (kinect != NULL){
    kinect->update();
    if( kinect->isFrameNew() ){
      
      // Get depth pixels.
      ofPixels pixels = kinect -> getDepthPixels();
      // Create the depth texture from Kinect pixels.
      depthTexture.loadData(pixels);
      
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
    }
  }
  
  // TODO: Enable this, when GUI is used.
  //audioPlayer.updateSound(brightness);

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
  
  // Update Audio player.
  audioPlayer.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Depth texture at 10,10.
  depthTexture.draw(0, 0);
  
  // Write the average brightness on the screen.
  ofDrawBitmapString(avgBrightness, 300, 300);
  
  // Threshold panel.
  panel.draw();
  gui.draw();
  
  //audioPlayer.drawGui();
}

void ofApp::keyPressed(int key) {
  if (key == 49) {
    audioPlayer.play();
  }
  
  if (key == 50) {
    audioPlayer.pause();
  }
  
  if (key == 51) {
    audioPlayer.stop();
  }
  
  if (key == 52) {
    bool pitchMode = audioPlayer.getPitchMode();
    audioPlayer.setPitchMode(!pitchMode);
  }
}
