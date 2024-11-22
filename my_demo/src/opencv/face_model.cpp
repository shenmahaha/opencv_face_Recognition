#include "../../inc/face_model.h"

extern mutex mtx;
int FaceModel::flag  = 0;
string FaceModel::id = "";
FaceModel::FaceModel(string path) : model_path(path)
{
    // 创建一个训练模块
    this->model = LBPHFaceRecognizer::create();
};
void FaceModel::get_flag()
{
    cout << "flag = " << flag << endl;
}
void FaceModel::train_recognizer(string & path)
{

    cout << "训练按钮" << endl;
    // 找到最后一个 '/' 字符的位置
    int pos = path.find_last_of('/');
    // 从 '/' 后面的位置截取文件名
    string dir_name = path.substr(pos + 1);

    // 定义一个人面容器
    vector<cv::Mat> face_s;
    // 定义人面对应的标签ID
    vector<int> face_id; //(10-czj)

    // 打开目录
    cout << "打开目录" << path.c_str() << endl;
    cout << "dir_name" << dir_name << endl;
    DIR * dir = opendir(path.c_str());
    if(dir == nullptr) {
        cerr << "Error opening directory." << endl;
        return;
    }
    vector<string> jpg_files;
    struct dirent * entry;
    while((entry = readdir(dir)) != nullptr) {
        // 排除 '.' 和 '..'
        if(entry->d_name[0] != '.') {
            string filename = entry->d_name;
            // 检查文件扩展名是否为 .jpg
            if(filename.size() >= 4 && filename.substr(filename.size() - 4) == ".jpg") {
                cout << "JPG File: " << filename << endl;
                jpg_files.push_back(path + "/" + filename);
            }
        }
    }

    closedir(dir);

    for(int i = 0; i < jpg_files.size(); i++) {
        // 读取人面数据  IMREAD_GRAYSCALE 灰度图
        cv::Mat face = cv::imread(jpg_files[i], IMREAD_GRAYSCALE);
        if(face.empty()) {
            cerr << "Error: Could not load image " << path << endl;
            continue; // 跳过当前图像，继续读取下一个
        }
        // 把数据放入容器待会训练
        face_s.push_back(face);            // 人面
        face_id.push_back(stoi(dir_name)); // 人面对应的标签

        // cout << "face_id = " << face_id[i] << endl;
    }
    cout << "dir_name" << dir_name << endl;
    cout << "jpg_files.size() = " << jpg_files.size() << endl;
    // 开始训练
    cout << "star tran" << endl;
    cout << "this->flag = " << this->flag << endl;
    if(this->flag == 0) {
        // 开始训练
        model->train(face_s, face_id);
    } else {
        model->update(face_s, face_id);
    }
    // 保存训练结果
    model->save(this->model_path);
    this->flag = 1;
    cout << "end train" << endl;
}

// 加载模型
void FaceModel::load_model(int select)
{
    // 加载训练好的模型
    ifstream file(this->model_path); // 检查文件是否存在
    cout << "加载模型:" << this->model_path << endl;
    if(file.good()) {
        cout << "文件存在" << endl;
        this->model->read(this->model_path);
        cout << "模型加载成功" << endl;
    } else {
        cout << "文件不存在: " << endl;

        FaceModel face;
        DIR * dir = opendir("./my_demo/data/faceputo");
        if(dir == nullptr) {
            std::cerr << "Error opening directory: " << strerror(errno) << std::endl;
            return;
        }

        struct dirent * entry;
        while((entry = readdir(dir)) != nullptr) {
            // 排除 . 和 ..
            if(entry->d_name[0] != '.') {
                std::cout << entry->d_name << std::endl;
                string path = "./my_demo/data/faceputo/" + string(entry->d_name);
                cout << "path = " << path << endl;
                face.train_recognizer(path);
                if(select == 1) {
                    // 设置存储空间和本地目录路径
                    string bucketName = "opencv-ly";   // OSS 存储空间名称
                    string localDir   = path;          // 本地文件夹路径
                    string ossDir     = entry->d_name; // OSS 上的目标目录

                    //  创建 OssUploader 对象
                    OssUploader uploader(accessKeyId, accessKeySecret, endpoint);
                    // 上传文件夹
                    if(uploader.uploadDirectory(localDir, bucketName, ossDir) == -1) {
                        cout << "上传失败" << endl;
                    }
                }
            }
        }
        closedir(dir);
        this->model->read(this->model_path);
    }
}
void FaceModel::comparing_faces(int max_index, vector<Rect> facerects, Mat frame)
{
    // sleep(1);
    // cout << "签到比对按钮点击！" << endl;

    // 检查 facerects 是否为空，确保有检测到人脸
    if(facerects.empty()) {
        cout << "没有检测到人脸，无法进行签到！" << endl;
        return;
    }

    // 有人面数据

    Mat face = frame(facerects.at(max_index));
    // 修改图片的大小 为 160 X 160
    cv::resize(face, face, cv::Size(160, 160));

    // 灰度face
    cv::cvtColor(face, face, cv::COLOR_BGR2GRAY);

    // Mat m = facerects.at(max_index);
    int id;
    double confidence;
    model->predict(face, id, confidence);

    if(confidence < 50) {
        this->id = to_string(id);
        cout << "识别出的人脸ID为：" << id << endl;
        cout << "置信度：" << confidence << endl;
    } else {
        this->id = "0";
    }
}

string FaceModel::get_id()
{
    return id;
}

string FaceModel::get_model_path()
{
    return this->model_path;
}

void FaceModel::set_flag(int flag)
{
    this->flag = flag;
}