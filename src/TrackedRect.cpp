#include "TrackedRect.h"

using namespace std;

using namespace ofxCv;
using namespace cv;

const float dyingTime = 1;

void TrackedRect::setup(const cv::Rect& track) {
	color.setHsb(ofRandom(0, 255), 255, 255);
	center = toOf(track).getCenter();
  boundingRect = track;
	smooth = center;
}

void TrackedRect::update(const cv::Rect& track) {
	center = toOf(track).getCenter();
  boundingRect = track;
	smooth.interpolate(center, .5);
	all.addVertex(smooth.x, smooth.y);
}

void TrackedRect::kill() {
	float curTime = ofGetElapsedTimef();
	if(startedDying == 0) {
		startedDying = curTime;
	} else if(curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void TrackedRect::draw() {
	ofPushStyle();
    float size = 16;
    ofSetColor(255);
  
    if(startedDying) {
      ofSetColor(ofColor::red);
      size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
    }
  
    ofNoFill();
    ofDrawCircle(center, size);
    ofSetColor(color);
    //all.draw();
    ofSetColor(255);
    ofDrawBitmapString(ofToString(label), center);
	ofPopStyle();
}
