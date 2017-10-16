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
  
  // Load track1.
  track1.load("howtostillmind.mp3");
  track1.setVolume(0.75f);
  track1.setLoop(true);
  
  // Load track2.
  track2.load("metronome.mp3");
  track2.setVolume(0.75f);
  track2.setLoop(true);
  
  panel.loadFromFile("settings.xml");
  
  //--------PATCHING-------
  // Sample 0, 1 are taking the sample buffer.
  sampler0.addSample( &sample, 0 );
  sampler1.addSample( &sample, 1 );

  // ADSR Trigger - Without this the sample wouldn't play.
  envGate >> env >> amp0.in_mod();
  env >> amp1.in_mod();
  
  // Setup the trigger to the sampler to the audio output. 
  sampleTrig >> sampler0 >> amp0 >> decimator >> engine.audio_out(0);
  sampleTrig >> sampler1 >> amp1 >> decimator >> engine.audio_out(1);

  //------------SETUPS AND START AUDIO-------------
  engine.listDevices();
  engine.setDeviceID(1); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!
  engine.setup( 44100, 512, 3);
}

void ofApp::updateSound() {
  if (!isDecimatorMode) {
    // If I'm in speed mode, update the speed of the track.
    float newSpeed = ofMap(avgBrightness, 10, 150, 1.0f, 0.5f, true);
    
    // Current track's speed.
    if (currentTrack != NULL) {
      currentTrack -> setSpeed(newSpeed);
    }
    
    // Reset noise
    noise * dB(-150.0f) >> engine.audio_out(0);
    noise * dB(-150.0f) >> engine.audio_out(1);
  } else {
    // Calculate the new decimator frequency based on the brightness.
    float newDecimatorFrequency = ofMap(avgBrightness, 10, 100, 20000, 1000, true);
    
    // Feed it to the decimator patch.
    newDecimatorFrequency >> decimator.in_freq();
    
    // Noise control. We will keep default pitch.
    
    // Modify noise bit.
    float noiseBit = ofMap(avgBrightness, 10, 100, 1, 12, true);
    noiseBit >> noise.in_bits();
    
    // Modify gain.
    noiseGain = ofMap(avgBrightness, 10, 100, -60.0f, -10.0f, true);
    
    noise * dB(noiseGain) >> engine.audio_out(0);
    noise * dB(noiseGain) >> engine.audio_out(1);
  }
}

void ofApp::setCurrentTrackAndPlay(int val, ofSoundPlayer * newCurrentTrack) {
  isDecimatorMode = false;
  
  if (isPlaying) {
    envGate.off();
    isPlaying = false;
  }
  
  if (val) {
    // Check if there is alread a currentTrack that's playing.
    // Stop that track.
    if (currentTrack != NULL) {
      currentTrack -> stop();
      currentTrack = NULL;
    }
    
    currentTrack = newCurrentTrack;
    currentTrack -> play();
  } else {
    
    // Stop the track if there is a valid current track.
    if (currentTrack != NULL) {
      currentTrack -> stop();
      currentTrack = NULL;
    }
  }
}

void ofApp::playWithPDSP(int val, int trackNum) {
  if (val == 0) {
    isPlaying = false;
    envGate.off();
    return;
  }
  
  isDecimatorMode = true;
  
  if (currentTrack != NULL) {
      currentTrack -> stop();
      currentTrack = NULL;
  }
  
  // Check if a track is already being played with sampleBuffer. Stop that
  // before loading and playing another track.
  
  if (isPlaying) {
      envGate.off();
  }
  
  switch (trackNum) {
      case 1:
        sample.load("/Users/amaykataria/Documents/of_v0.9.8_osx_release/apps/Silo/KinectInteractiveSound/bin/data/howtostillmind.mp3");
        break;
      
      case 2:
        sample.load(" /Users/amaykataria/Documents/of_v0.9.8_osx_release/apps/Silo/KinectInteractiveSound/bin/data/metronome.mp3");
        break;
      
      default:
        break;
  }

  
  // Trigger the sample and let it go.
  sampleTrig.trigger(1.0f);
  envGate.trigger(1.0f);
  isPlaying = true;
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
        // Update the sound.
        updateSound();
      }
    }
  }
  
  // Touch OSC updates.
  while (receive.hasWaitingMessages()) {
    ofxOscMessage m;
    // Set the next message.
    #pragma warning(disable: WARNING_CODE)
    receive.getNextMessage(&m);
    
    // Pause/play Track1.
    if (m.getAddress() == "/3/toggle1") {
      int val = m.getArgAsInt(0);
      setCurrentTrackAndPlay(val, &track1);
    }
    
    // Pause/play Track2.
    if (m.getAddress() == "/3/toggle2") {
      int val = m.getArgAsInt(0);
      setCurrentTrackAndPlay(val, &track2);
    }
    
    // Play Track1 with decimation mode.
    if (m.getAddress() == "/3/toggle3") {
      int val = m.getArgAsInt(0);
      playWithPDSP(val, 1);
    }
    
    // Play Track2 with decimation mode.
    if (m.getAddress() == "/3/toggle4") {
      int val = m.getArgAsInt(0);
      playWithPDSP(val, 2);
      
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Depth texture at 10,10.
  depthTexture.draw(0, 0);
  
  // Write the average brightness on the screen.
  ofDrawBitmapString(avgBrightness, 300, 300);
  
  // Threshold panel.
  panel.draw();
}
