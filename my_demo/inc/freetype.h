#ifndef FREETYPE_H // 防止重复包含
#define FREETYPE_H

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
using namespace std;
using namespace cv;
using namespace cv::face;

class Freetype {
  private:
    lv_font_t * font;
    lv_style_t style;

  public:
    Freetype(string ttf_path, int size);

    void set_font_style(lv_obj_t * obj, string txt);
};
#endif // MYCLASS_H