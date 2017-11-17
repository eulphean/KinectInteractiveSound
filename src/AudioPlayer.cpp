// Author: Amay Kataria
// Date: 11/03/2017

#include "AudioPlayer.h"

void AudioPlayer::patch() {
    //--------PATCHING-------
  
    // ADSR Trigger - Without this the sample wouldn't play.
    sampleTrig >> sampler >> amp;
    envGate    >> env >> amp.in_mod();
    env >> amp.in_mod();
  
    // Default frequency of the sample.
    decimator.set(20000);
  
    // Route the sampler to the output.
    sampleTrig >> sampler >> amp >> decimator >> engine.audio_out(0);
                             amp >> decimator >> engine.audio_out(1);
  
    // Default noise level. TODO: Wrap it with an envelope.
    killAmbientNoise();
  
    // Don't need the GUI for now.
    /*osc_seek_ctrl >> sampler.in_start();
    
    // Setup the GUI.
    gui.setup("Decimator");
    osc1_group.add( osc_seek_ctrl.set( "seek", 0.0f, 0.0f, 1.0f) );
    gui.add( osc1_group );
    gui.setPosition(30, 30);*/
  
    // Defaut state of the system.
    sampleState = stopped;
  
    // Default pitch mode.
    pitchMode = false;
  
    // Select the first sample by default.
    sampleIdx = 0;
    sampleIdx >> sampler.in_select();

    //------------SETUPS AND START AUDIO-------------
    engine.listDevices();
    engine.setDeviceID(0); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!
    engine.setup(44100, 512, 2);
}

void AudioPlayer::update() {
  float meter_position = getMeterPosition();
  if (meter_position > 1.0f) {
    // Audio is done playing.
    sampleState = stopped;
  
    // Select the next sample. 
    /*sampleIdx = (sampleIdx + 1);
    sampleIdx = sampleIdx % samples.size();
    sampleIdx >> sampler.in_select();
      cout << sampleIdx << endl;*/
      // Repeat the first track only.
      0.0f >> sampler.in_select();
    
    // Play the sample.
    play();
  }
  
  // Check if it's time to switch into pitch mode. Let's do it after 30 seconds right now for testing sake.
  if (ofGetElapsedTimeMillis() > 10000 && !pitchMode) { // 1 minute
    pitchMode = !pitchMode;
    ofResetElapsedTimeCounter();
  }
  
  if (ofGetElapsedTimeMillis() > 20000 && pitchMode) { // 1 minutes
    pitchMode = !pitchMode;
    ofResetElapsedTimeCounter();
  }
}

void AudioPlayer::updateSound(float avgBrightness) {
    if (pitchMode) {
      // Reset any decimation.
      20000 >> decimator.in_freq();
      1 >> noise.in_bits();
      killAmbientNoise();
      
      float newPitch = ofMap(avgBrightness, minBrightness, maxBrightness, -10.0f, 5.0f, true);
      newPitch >> sampler.in_pitch();
      
    } else {
      // Reset any pitch differences.
      0.0f >> sampler.in_pitch();
      
      // Calculate the new decimator frequency based on the brightness.
      float newDecimatorFrequency = ofMap(avgBrightness, minBrightness, maxBrightness, 10000, 500, true);
      
      // Feed it to the decimator patch.
      newDecimatorFrequency >> decimator.in_freq();
      
      // Modify noise bit.
      float noiseBit = ofMap(avgBrightness, minBrightness, maxBrightness, 1, 12, true);
      noiseBit >> noise.in_bits();
    }
  
    if (sampleState == playing && !pitchMode) {
      float noiseGain;
      // Modify noise gain.
      noiseGain = ofMap(avgBrightness, minBrightness, maxBrightness, -45.0f, 10.0f, true);
      noise * dB(noiseGain) >> engine.audio_out(0);
      noise * dB(noiseGain) >> engine.audio_out(1);
    //  noise * dB(noiseGain) >> amp;
    }
}

float AudioPlayer::getMeterPosition() {
   return sampler.meter_position();
}

void AudioPlayer::drawGui() {
  gui.draw();
}

void AudioPlayer::addSample(string path) {
  samples.push_back( new pdsp::SampleBuffer() );
  samples.back()->load(path);
  sampler.addSample(samples.back());
}

// Load an audio sample. 
void AudioPlayer::play() {
  if (sampleState == paused){
    envGate.trigger(1.0f);
  } else if (sampleState == stopped) {
    //setSeekPosition(0.0f);
    envGate.trigger(1.0f);
    sampleTrig.trigger(1.0f);
  }
  sampleState = playing;
}


void AudioPlayer::pause(){
  if (sampleState == playing) {
    sampleState = paused;
    envGate.off();
    killAmbientNoise();
  }
}

void AudioPlayer::stop(){
  if (sampleState == playing) {
    sampleState = stopped;
    envGate.off();
    killAmbientNoise();
  }
}

void AudioPlayer::setGain(float dbVal) {
  sampler * dB(dbVal) >> amp;
  //noise * dB(dbVal/2) >> engine.audio_out(0);
  //noise * dB(dbVal/2) >> engine.audio_out(1);
}

void AudioPlayer::setPitchMode(bool enablePitchMode) {
  pitchMode = enablePitchMode;
}

bool AudioPlayer::getPitchMode() {
  return pitchMode;
}

void AudioPlayer::setSeekPosition(float seekPosition) {
  seekPosition >> sampler.in_start();
}

void AudioPlayer::killAmbientNoise() {
  float noiseGain = -150;
  noise * dB(noiseGain) >> engine.audio_out(0);
  noise * dB(noiseGain) >> engine.audio_out(1);
}
