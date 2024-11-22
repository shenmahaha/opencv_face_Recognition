#ifndef FACE_MODEL_H    // 防止重复包含
#define FACE_MODEL_H

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
#include "OssDownloader.h"
#include "OssUploader.h"
#include "home.h"


using namespace std;
using namespace cv;
using namespace cv::face;


class FaceModel {
  private:
    Ptr<FaceRecognizer> model;
    string model_path;
    static int flag; // 0表示未训练，1表示已训练
    static string id;

  public:
    FaceModel(string path = "./my_demo/data/faceputo/face_model.yml");
    // 训练模型函数
    void train_recognizer(string & path);
    void get_flag();
    void set_flag(int flag);
    void load_model(int select);
    void comparing_faces(int max_index,vector<Rect> facerects,Mat frame);
    static string get_id();
    string get_model_path();
    
};
#endif // MYCLASS_H