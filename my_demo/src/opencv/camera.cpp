#include "../../inc/camera.h"
#include "../../inc/face_model.h"

extern mutex mtx;
Camera::Camera(string feature_detector_path) : feature_detector_path(feature_detector_path)
{}
void Camera::convert_opencv_to_lvgl(const Mat & frame, lv_color_t * buffer, int width, int height)
{
    // 遍历图像的每一行和每一列，即遍历每一个像素点
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            // 获取当前像素点的BGR颜色值
            Vec3b pixel = frame.at<Vec3b>(y, x);
            // 分别提取BGR颜色值中的红色、绿色和蓝色分量
            uint8_t r = pixel[2];
            uint8_t g = pixel[1];
            uint8_t b = pixel[0];
            // uint8_t a = 0;
            // 将BGR颜色值转换为LVGL所需的ARGB格式，并存储到缓冲区中
            // 由于OpenCV图像没有alpha通道，所以在这里忽略alpha值
            buffer[y * width + x] = lv_color_make(r, g, b);
        }
    }
}
void Camera::face_live_detect(FaceModel & face_model)
{
    // 初始化 OpenCV 视频捕获
    VideoCapture cap(0); // 使用默认摄像头
    if(!cap.isOpened()) {
        cerr << "Error: Failed to open camera." << endl;
        return;
    }

    // 设置摄像头分辨率为 640x480
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    // 定义一个特征检测器
    CascadeClassifier face;
    // 导入人面特征文件
    face.load(this->feature_detector_path);
    // 创建一个 LVGL 画布对象
    lv_obj_t * canvas = lv_canvas_create(lv_scr_act());
    if(canvas == nullptr) {
        cerr << "Error: Failed to create LVGL canvas object." << endl;
        return;
    }
    cout << "LVGL canvas object created successfully." << endl;

    // 创建 LVGL 图像缓冲区
    lv_color_t * img_buffer = static_cast<lv_color_t *>(malloc(640 * 480 * sizeof(lv_color_t)));
    if(img_buffer == nullptr) {
        cerr << "Error: Failed to allocate memory for image buffer." << endl;
        return;
    }
    cout << "Image buffer allocated successfully." << endl;

    // 设置画布缓冲区
    lv_canvas_set_buffer(canvas, img_buffer, 640, 480, LV_COLOR_FORMAT_RGB888); // 确认格式是否正确
    // 不断采集摄像头数据

    while(1) {
        // 读取一帧
        cap >> this->frame; // 获取一帧图像到 frame 中
        if(this->frame.empty()) {
            cerr << "Error: Failed to read frame from camera." << endl;

            continue;
        }

        // 输出图像大小
        //  cout << "frame size: " << frame.cols << "x" << frame.rows << endl;
        flip(this->frame, this->frame, 1); // 1 表示水平翻转
        // 把图像转换位灰图 ，提高检测效率
        Mat grayim;
        cvtColor(this->frame, grayim, COLOR_BGR2GRAY); // COLOR_BGR2GRAY 灰图
        // 把检测到的人面存放到容器中
        face.detectMultiScale(grayim, this->facerects, 1.1, 3, 0, Size(30, 30), Size(200, 200));
        // 计算人脸区域的大小并动态调整框的大小
        if(this->facerects.size() > 0) {
            int max_area  = 0;
            int max_index = 0;
            for(int i = 0; i < this->facerects.size(); i++) {
                int area = this->facerects[i].width * this->facerects[i].height;
                if(area > max_area) {
                    max_area  = area;
                    max_index = i;
                }
            }

            Rect face_rect = this->facerects.at(max_index);
            face_model.comparing_faces(max_index, this->facerects, this->frame);

            // 根据人脸的面积比例调整框的大小
            float scale_factor = 1.2f; // 根据需要放大或缩小
            face_rect.width    = static_cast<int>(face_rect.width * scale_factor);
            face_rect.height   = static_cast<int>(face_rect.height * scale_factor);

            // 确保框不会超出图像边界
            face_rect.x      = max(0, face_rect.x - (face_rect.width - facerects.at(max_index).width) / 2);
            face_rect.y      = max(0, face_rect.y - (face_rect.height - facerects.at(max_index).height) / 2);
            face_rect.width  = min(this->frame.cols - face_rect.x, face_rect.width);
            face_rect.height = min(this->frame.rows - face_rect.y, face_rect.height);

            // 绘制调整后的人脸框
            rectangle(this->frame, face_rect, Scalar(255, 0, 0), 1, LINE_8, 0);
        }
        mtx.lock(); // 手动锁定互斥锁

        // 转换图像格式以便于在LVGL上显示
        convert_opencv_to_lvgl(this->frame, img_buffer, 640, 480);

        // 刷新LVGL画布
        lv_obj_invalidate(canvas);
        mtx.unlock(); // 手动解锁互斥锁
    }
}
void augment_and_save(Mat face, int & pic, const string & save_dir);
void create_directory(const string & dir_name);
string Camera::photograph(string &dir_path)
{
    cout << "点击按钮    " << endl;

    int pic = 1; // 文件名从 1 开始

    // 获取当前时间戳
    time_t now = time(0);
    tm * ltm   = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%d%H%M%S", ltm); // 格式化时间戳

    // 创建时间戳目录
    string save_dir = dir_path + string(time_str);

    // 保存人面的照片
    // 把检测到的人面图截图保存

    for(int i = 0; i < this->facerects.size(); i++) {
        // 获取大图中的小图 ，区域中的 Rect 像素值
        Mat face = this->frame(this->facerects.at(i));
        // 修改图片的大小 为 160 X 160
        resize(face, face, Size(160, 160));

        // 调用增强函数，保存多张图像
        augment_and_save(face, pic, save_dir);
    }
    return string(time_str);
}

void create_directory(const string & dir_name)
{
    // 创建文件夹
    struct stat info;
    if(stat(dir_name.c_str(), &info) != 0) { // 检查文件夹是否存在
        if(mkdir(dir_name.c_str(), 0777) == -1) {
            cout << "创建目录失败: " << dir_name << endl;
        }else{
            cout << "创建目录成功: " << dir_name << endl;
        }
    }
}

void augment_and_save(Mat face, int &pic, const string &save_dir) {
    // 定义保存目录
    create_directory(save_dir);

    // 保存原始图像（命名为 1.jpg）
    char path[1024] = {0};
    sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
    imwrite(path, face);

    // 1. 随机旋转 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat rotated;
        Point2f center((face.cols - 1) / 2.0, (face.rows - 1) / 2.0);
        double angle = rand() % 360; // 随机旋转角度
        Mat rot_mat = getRotationMatrix2D(center, angle, 1.0);
        warpAffine(face, rotated, rot_mat, face.size());
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, rotated);
    }

    // 2. 随机缩放 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat resized;
        double scale = 0.8 + (rand() % 5) * 0.1; // 随机缩放因子
        resize(face, resized, Size(), scale, scale);
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, resized);
    }

    // 3. 镜像 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat flipped;
        flip(face, flipped, 1); // 水平翻转
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, flipped);
    }

    // 4. 添加随机噪声 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat noisy_face = face.clone();
        int noise_level = 10 + rand() % 30; // 随机噪声强度
        for (int j = 0; j < noisy_face.rows; j++) {
            for (int k = 0; k < noisy_face.cols; k++) {
                if (rand() % 100 < noise_level) { // 随机噪声点
                    noisy_face.at<Vec3b>(j, k) = Vec3b(rand() % 256, rand() % 256, rand() % 256);
                }
            }
        }
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, noisy_face);
    }

    // 5. 随机亮度调整 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat bright_face;
        double brightness = 0.8 + (rand() % 5) * 0.1; // 随机亮度
        face.convertTo(bright_face, -1, brightness, 0);
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, bright_face);
    }

  // 6. 随机裁剪 - 生成 3 张
for (int i = 0; i < 3; i++) {
    // 随机裁剪的区域位置和大小
    int crop_width = face.cols / 2 + rand() % (face.cols / 2); // 最大裁剪宽度为图像宽度的一半
    int crop_height = face.rows / 2 + rand() % (face.rows / 2); // 最大裁剪高度为图像高度的一半
    
    // 随机生成裁剪的起始坐标，确保不会超出图像边界
    int crop_x = rand() % (face.cols - crop_width);  // 确保裁剪框的右侧不会超出图像宽度
    int crop_y = rand() % (face.rows - crop_height); // 确保裁剪框的底部不会超出图像高度

    Rect crop_region(crop_x, crop_y, crop_width, crop_height);
    
    // 执行裁剪操作
    Mat cropped_face = face(crop_region);
    char path[1024] = {0};
    sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
    imwrite(path, cropped_face);
}

    // 7. 高斯模糊 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat blurred_face;
        int ksize = (rand() % 5) * 2 + 1; // 随机选择一个奇数作为高斯核大小
        GaussianBlur(face, blurred_face, Size(ksize, ksize), 0);
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, blurred_face);
    }

    // 8. 色调和饱和度调整 - 生成 3 张
    for (int i = 0; i < 3; i++) {
        Mat hsv_face;
        cvtColor(face, hsv_face, COLOR_BGR2HSV);
        int hue_shift = rand() % 50 - 25; // 随机色调偏移
        int saturation_shift = rand() % 50 - 25; // 随机饱和度偏移
        for (int j = 0; j < hsv_face.rows; j++) {
            for (int k = 0; k < hsv_face.cols; k++) {
                Vec3b &pixel = hsv_face.at<Vec3b>(j, k);
                pixel[0] = (pixel[0] + hue_shift + 180) % 180;
                pixel[1] = min(max(pixel[1] + saturation_shift, 0), 255);
            }
        }
        Mat final_face;
        cvtColor(hsv_face, final_face, COLOR_HSV2BGR);
        sprintf(path, "%s/%d.jpg", save_dir.c_str(), pic++);
        imwrite(path, final_face);
    }


}