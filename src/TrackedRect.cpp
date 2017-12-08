#include "TrackedRect.h"

using namespace std;

using namespace ofxCv;
using namespace cv;

const float dyingTime = 1;

void TrackedRect::setup(const cv::Rect& track) {
	kalman.init(1/10000., 1/10.); // inverse of (smoothness, rapidness)
  
	color.setHsb(ofRandom(0, 255), 255, 255);
  
  // Only get X and Y coordinate from bounded rectangle.
	ofVec2f c = toOf(track).getCenter();
  center.x = c.x; center.y = c.y;
  
	smooth = center;
}

void TrackedRect::updateCenterWithZ(int zDistance) {
  center.z = zDistance;
}

glm::vec3 TrackedRect::getCenter() {
  return center;
}

void TrackedRect::update(const cv::Rect& track) {
  // Only get X and Y coordinate from bounded rectangle.
	ofVec2f c = toOf(track).getCenter();
  center.x = c.x; center.y = c.y;
  
  smooth = glm::mix(glm::vec3(0, 0, 0), center, 0.5);
	all.addVertex(smooth.x, smooth.y, smooth.z);
}

void TrackedRect::kill() {
	float curTime = ofGetElapsedTimef();
	if(startedDying == 0) {
		startedDying = curTime;
	} else if(curTime - startedDying > dyingTime) {
		dead = true;
	}
}

void TrackedRect::setWorldCoordinate(glm::vec3 world) {
    kalman.update(world);
    worldCoordinate = kalman.getEstimation();
}

glm::vec3 TrackedRect::getWorldCoordinate() {
  return worldCoordinate;
}

void TrackedRect::draw() {
	ofPushStyle();
    float size = 60;
  
    // When the follower is dying, linearly reduce
    // the size of sphere.
    if(startedDying) {
      ofSetColor(ofColor::red);
      size = ofMap(ofGetElapsedTimef() - startedDying, 0, dyingTime, size, 0, true);
    }
  
    ofPushMatrix();
      ofTranslate(worldCoordinate);
        ofSetColor(color);
    stringstream ss;
      ofDrawSphere(glm::vec3(0, 0, 0), size);
    ofPopMatrix();
	ofPopStyle();
}
