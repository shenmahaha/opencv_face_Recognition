#ifndef HOME_H // 防止重复包含
#define HOME_H

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
#include <algorithm>

#include "face_model.h"
#include "camera.h"
#include "freetype.h"
#include "staff.h"
#include "TCP_client.h"
#include "OssDownloader.h"
#include "OssUploader.h"
#include "OssDeleter.h"

using namespace std;
using namespace cv;
using namespace cv::face;

// 设置 AccessKeyId 和 AccessKeySecret
extern string accessKeyId    ;
extern string accessKeySecret;
extern string endpoint   ;

extern vector<Staff> staff_vec;
extern lv_obj_t * sign_success_label;
extern void home_screen_init();


#endif 