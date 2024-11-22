#ifndef CAMERA_H // 防止重复包含
#define CAMERA_H

extern "C" {
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
}

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <lvgl/lvgl.h>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <fstream>
#include <filesystem>
// #include "face_model.h"
// #include "home.h"
using namespace std;
using namespace cv;
using namespace cv::face;

class FaceModel; 
class Camera {
  private:
    vector<Rect> facerects;       // 脸部容器
    Mat frame;                    // 当前视频帧
    string feature_detector_path; // 特征检测器路径

  public:
    Camera(string feature_detector_path = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
    void convert_opencv_to_lvgl(const Mat & frame, lv_color_t * buffer, int width, int height);
    void face_live_detect(FaceModel & face_model);
    string photograph(string & dir_path);
};
#endif // MYCLASS_H