#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class TrackedRect : public ofxCv::RectFollower {
  protected:
    ofColor color;
    glm::vec3 smooth;
    float startedDying;
  
    // Following trail. We are not using this currently. 
    ofPolyline all;
  
    // Center of the bounding box.
    glm::vec3 center;
  
    // Actual coordinate in real world,
    glm::vec3 worldCoordinate;
  
    // Z distance based on the brightness of the
    // tracked object.
    int zDistance;
  
  public:
  
    // Constructor
	TrackedRect()
		:startedDying(0) {
	}
  
	void setup(const cv::Rect& track);
	void update(const cv::Rect& track);
	void kill();
	void draw();
    
  // Update depth only when it's actively getting tracked.
  void updateCenterWithZ(int zDistance);

  glm::vec3 getCenter();
  
  void setWorldCoordinate(glm::vec3 worldCoordinate);
  glm::vec3 getWorldCoordinate();
  
  // Kalman tracking.
  ofxCv::KalmanPosition kalman;
};
