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
#include "./my_demo/inc/face_model.h"
#include "./my_demo/inc/camera.h"
#include "./my_demo/inc/freetype.h"
#include "./my_demo/inc/home.h"
using namespace std;
using namespace cv;
using namespace cv::face;

// static lv_obj_t * face;
mutex mtx;

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width  = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "800");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "480");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

#define SDL_INPUT 1
#define LINUX_INPUT 0



// 将 OpenCV 图像转换为 LVGL 图像缓冲区
/**
 * 将OpenCV图像转换为LVGL图形库可以显示的格式
 *
 * @param frame const cv::Mat& 类型的输入参数，代表OpenCV中的图像数据，使用BGR颜色空间
 * @param buffer lv_color_t* 类型的输出参数，指向存储转换后图像数据的缓冲区
 * @param width int 类型，代表图像的宽度
 * @param height int 类型，代表图像的高度
 *
 * 此函数的作用是将OpenCV中的BGR图像数据转换为LVGL图形库所需的ARGB颜色格式，
 * 以便在LVGL中正确显示图像LVGL使用ARGB颜色格式，其中A代表alpha通道，即透明度，
 * 而OpenCV默认使用BGR颜色格式因此，需要进行颜色空间的转换
 */

std::vector<cv::Rect> facerects;
// 当前视频帧
Mat frame;

void btn_event_cb(lv_event_t * e) // 按钮事件函数
{
    cout << "点击按钮    " << endl;

    static int pic = 0;

    // 保存人面的照片
    // 把检测到的人面图截图保存

    for(int i = 0; i < facerects.size(); i++) {
        // 获取大图中的小图 ，区域中的 Rect 像素值
        Mat face = frame(facerects.at(i));
        // 修改图片的大小 为 160 X 160
        cv::resize(face, face, cv::Size(160, 160));

        // 保存到本地
        char path[1024] = {0};
        sprintf(path, "./my_demo/data/faceputo/photograh01/%d.jpg", pic++);
        cv::imwrite(path, face);
    }
}
void train_event_cb(lv_event_t * e)
{
   
}




int main(void)
{
    lv_init();
    /*Linux display device init*/
    lv_linux_disp_init();

#if LINUX_INPUT
    // init LINUX_INPUT
    lv_indev_t * ts = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_evdev_set_calibration(ts, 0, 0, 1024, 600); // 黑色边框的屏幕校准
    // lv_evdev_set_calibration(ts,0,0,800,480);  //蓝色边框的屏幕校准
#endif

    // init SDL_INPUT
#if SDL_INPUT
    lv_sdl_mouse_create();
    lv_sdl_keyboard_create();
    lv_sdl_mousewheel_create();
    lv_sdl_mousewheel_create();
#endif
    // Freetype f1("./my_demo/data/ttf/DingTalk-JinBuTi.ttf", 24);
    // // 创建一个按钮
    // lv_obj_t * photograph_btn = lv_btn_create(lv_scr_act());
    // lv_obj_set_size(photograph_btn, 120, 70);
    // lv_obj_align(photograph_btn, LV_ALIGN_BOTTOM_RIGHT, 0, -30);
    // lv_obj_add_event_cb(photograph_btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_t * label = lv_label_create(photograph_btn);
    // f1.set_font_style(label, "拍照");
    // lv_obj_center(label);

    // 创建一个训练按钮

    // lv_obj_t * train_btn = lv_btn_create(lv_scr_act());
    // lv_obj_set_size(train_btn, 120, 70);
    // lv_obj_align(train_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    // lv_obj_add_event_cb(train_btn, train_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_t * label2 = lv_label_create(train_btn);
    // f.set_font_style(label2, "训练");
    // lv_obj_center(label2);

    // // 创建一个签到比对按钮
    // lv_obj_t * signin_btn = lv_btn_create(lv_scr_act());
    // lv_obj_set_size(signin_btn, 120, 70);
    // lv_obj_align(signin_btn, LV_ALIGN_RIGHT_MID, 0, -80);
    // lv_obj_add_event_cb(signin_btn, signin_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_t * label3 = lv_label_create(signin_btn);
    // f.set_font_style(label3, "签到");
    // lv_obj_center(label3);

    //  FaceModel face;
     
    //     string path="./my_demo/data/faceputo/1731565505";
    //     DIR * dir = opendir(path.c_str());
    //     face.train_recognizer(path);
    //     path="./my_demo/data/faceputo/1731565669";
    //     dir = opendir(path.c_str());
    //     face.train_recognizer(path);

    //     path="./my_demo/data/faceputo/1731565650";
    //     dir = opendir(path.c_str());
    //     face.train_recognizer(path);
   home_screen_init();
    
    /*Handle LVGL tasks*/
    while(1) {
        mtx.lock();         // 手动锁定互斥锁
        lv_timer_handler(); // 刷新界面
        mtx.unlock(); // 手动解锁互斥锁
        usleep(5000);
    }
    return 0;
}
