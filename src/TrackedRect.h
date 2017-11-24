#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class TrackedRect : public ofxCv::RectFollower {
  protected:
    ofColor color;
    ofVec2f smooth;
    float startedDying;
    ofPolyline all;
  public:
    ofVec2f center;
    int brightness; 
    // Bounding rectangle that we are following.
    cv::Rect boundingRect;
  
  // Constructor
	TrackedRect()
		:startedDying(0) {
	}
  
	void setup(const cv::Rect& track);
	void update(const cv::Rect& track);
	void kill();
	void draw();
};
