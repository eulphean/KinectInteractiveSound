//camera information based on the Kinect v2 hardware
class CameraParams {
  public:
    static float cx;
    static float cy;
    static float fx;
    static float fy;
    static float k1;
    static float k2;
    static float k3;
    static float p1;
    static float p2;
};

float CameraParams::cx = 254.878f;
float CameraParams::cy = 205.395f;
float CameraParams::fx = 365.456f;
float CameraParams::fy = 365.456f;
float CameraParams::k1 = 0.0905474;
float CameraParams::k2 = -0.26819;
float CameraParams::k3 = 0.0950862;
float CameraParams::p1 = 0.0;
float CameraParams::p2 = 0.0;
