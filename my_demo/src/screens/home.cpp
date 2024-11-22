#include "../../inc/home.h"
#include "../../lvgl/src/libs/freetype/lv_freetype.h"
extern mutex mtx;
struct tm * timeinfo; // 时间结构体
time_t now;
char date_str[128];
char time_str[128];
lv_obj_t * date_label;
lv_obj_t * time_label;
vector<Staff> staff_vec;
string sign_name;
string sign_id;
lv_obj_t * sign_name_label;

lv_obj_t * sign_container;
lv_obj_t * enter_btn;
lv_obj_t * sign_in_btn;
lv_obj_t * photo_btn;
lv_obj_t * back_btn;
lv_obj_t * delete_btn;
lv_obj_t * confirm_delete_btn;
lv_obj_t * delete_back_btn;
lv_obj_t * sign_success_label;

lv_obj_t * textBox_font;
static lv_obj_t * input_box;
// lv_obj_t * label_font;
lv_font_t * time_font;
lv_font_t * text_font;
lv_obj_t * keyboard;

FaceModel face_model;
Camera camera;

TCPClient client("192.168.73.48", 8080);

// 设置 AccessKeyId 和 AccessKeySecret
string accessKeyId     = "xxxxxx";       // 填入你的 Access Key
string accessKeySecret = "xxxxxx"; // 填入你的 Secret Key
string endpoint        = "xxxxxx";  // OSS endpoint

void create_num_keyboard(lv_obj_t * parent);

// 实时检测人面 ,任务线程demo
void * face_live_task(void * task)
{

    face_model.load_model(1);
    camera.face_live_detect(face_model); // 摄像头实时检测
    return NULL;
}

void * update_time_task(void * task)
{
    while(1) {
        sleep(1);
        now      = time(nullptr);
        timeinfo = localtime(&now);
        // 清空数组
        memset(date_str, 0, sizeof(date_str));
        memset(time_str, 0, sizeof(time_str));
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
        mtx.lock();
        lv_label_set_text(date_label, date_str);
        lv_label_set_text(time_label, time_str);
        mtx.unlock();
    }
    return NULL;
}
void * update_sign_name_task(void * task)
{
    while(1) {
        sleep(1);
        int flag = 0;
        sign_id  = FaceModel::get_id();
        cout << "sign_id:" << sign_id << endl;
        if(sign_id != "" && sign_id != "0") {
            // 按照签到人id搜索员工
            for_each(staff_vec.begin(), staff_vec.end(), [&](Staff & staff) {
                if(staff.get_id() == sign_id) {
                    sign_name = staff.get_name();
                    // 加锁
                    mtx.lock();
                    lv_label_set_text(sign_name_label, sign_name.c_str());
                    // 解锁
                    mtx.unlock();
                    flag = 1;
                }
            });
        }
        if(flag == 0) {
            // 加锁
            mtx.lock();
            lv_label_set_text(sign_name_label, "未识别");
            // 解锁
            mtx.unlock();
            sign_name = "未识别";
        }
        cout << "update sign name:" << sign_name << endl;
    }
}
void * photo_task(void * task)
{
    const char * e_text = lv_textarea_get_text(input_box);
    string name         = string(e_text);
    if(name != "" && name != " ") {
        string dir_path = "./my_demo/data/faceputo/";
        string dir_name = camera.photograph(dir_path);
        // 判断name不为空并且不为空格
        if(dir_name != "") {
            string path = dir_path + dir_name;
            face_model.train_recognizer(path);
            staff_vec.push_back(Staff(dir_name, name));
            cout << "训练成功！" << endl;
            sleep(1);
            // 发送人脸数据给OSS

            // 设置存储空间和本地目录路径
            string bucketName = "opencv-ly"; // OSS 存储空间名称
            string localDir   = path;        // 本地文件夹路径
            string ossDir     = dir_name;    // OSS 上的目标目录

            //  创建 OssUploader 对象
            OssUploader uploader(accessKeyId, accessKeySecret, endpoint);
            // 上传文件夹
            if(uploader.uploadDirectory(localDir, bucketName, ossDir) == -1) {
                cout << "上传失败" << endl;
            } else {
                // 给服务器发送消息
                string data = "data_id:4 id:" + dir_name + " name:" + name;
                client.send_data(data);
                lv_textarea_set_text(input_box, ""); // 清空输入框
            }
        }
    }
    return NULL;
}

// 拍照回调函数
void photo_btn_event_cb(lv_event_t * e)
{
    pthread_t photo_pid;
    pthread_create(&photo_pid, NULL, photo_task, NULL);
}

void sign_in_btn_event_cb(lv_event_t * e)
{
    if(sign_name != "未识别") {
        string time = string(date_str) + " " + string(time_str);
        cout << "sign in time:" << time << endl;
        // TODO
        string data = "data_id:1 date:" + string(date_str) + " id:" + sign_id + " name:" + sign_name + " time:" + time;
        client.send_data(data);
    }
}
// 进入录入界面回调函数
void enter_btn_event_cb(lv_event_t * e)
{
    // 进入录入界面
    lv_obj_add_flag(sign_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(enter_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sign_in_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sign_success_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(photo_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(input_box, LV_OBJ_FLAG_HIDDEN);
}
// 返回回调函数
void back_btn_event_cb(lv_event_t * e)
{
    // 返回主界面
    lv_obj_clear_flag(sign_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(enter_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sign_in_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(photo_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(input_box, LV_OBJ_FLAG_HIDDEN);
    // 清空输入框
    lv_textarea_set_text(input_box, "");
}
// 删除回调函数
void delete_btn_event_cb(lv_event_t * e)
{
    // 删除
    // TODO
    lv_obj_add_flag(sign_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(enter_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sign_in_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sign_success_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(confirm_delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(delete_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(input_box, LV_OBJ_FLAG_HIDDEN);

}
void * delete_task(void * arg)
{
    string name = lv_textarea_get_text(input_box);
    if(name != "" && name != " ") {
        for_each(staff_vec.begin(), staff_vec.end(), [name](Staff & s) {
            string id;
            if(s.get_name() == name) {

                id = s.get_id();
                // 删掉本地文件
                string face_puto_path = "./my_demo/data/faceputo/" + id;
                ifstream file(face_puto_path); // 检查文件是否存在
                if(file.good()) {
                    const string command = "rm -rf " + face_puto_path;
                    system(command.c_str());
                }
                fstream model_file(face_model.get_model_path()); // 检查文件是否存在
                if(model_file.good()) {
                    system(("rm -rf " + face_model.get_model_path()).c_str());
                }
                // 删掉OSS数据
                // 创建 OssDeleter 对象
                OssDeleter deleter(accessKeyId, accessKeySecret, endpoint);

                // 设置要删除的 OSS 存储桶名称和目录路径
                std::string bucketName = "opencv-ly";
                std::string ossDir     = id + "/"; // 例如："my-folder/" 或者 "images/2023/"

                // 删除目录及其中的所有文件
                deleter.deleteFolder(bucketName, ossDir);
                // 删除成员数组
                staff_vec.erase(
                    remove_if(staff_vec.begin(), staff_vec.end(), [id](Staff & s) { return s.get_id() == id; }));
                client.send_data("data_id:6 id:" + id);
                
            }
        });
    face_model.load_model(0);
    // 清空输入框
    lv_textarea_set_text(input_box, "");
    }
    return nullptr;
}
void confirm_delete_btn_event_cb(lv_event_t * e)
{
    // 确认删除
    // TODO
    pthread_t photo_pid;
    pthread_create(&photo_pid, NULL, delete_task, NULL);
      
}
void delete_back_btn_event_cb(lv_event_t * e)
{
    // 返回
    // TODO
    lv_obj_clear_flag(sign_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(enter_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sign_in_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(confirm_delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(delete_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(input_box, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(input_box, "");
}
void home_screen_init()
{
    // 获取服务器员工信
    // 启动接收线程
    client.start_receiving();
    string data = "data_id:2";
    client.send_data(data);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // 创建一个线程
    pthread_t camera_thread_id;
    pthread_create(&camera_thread_id, NULL, face_live_task, NULL);

    pthread_t time_thread_id;
    pthread_create(&time_thread_id, NULL, update_time_task, NULL);
    fstream model_file(face_model.get_model_path());
    if(model_file.good()) {
        cout << "model file not exist" << endl;
        face_model.set_flag(1);
    }

    time_font = lv_freetype_font_create("./my_demo/data/ttf/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                        25, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!time_font) {
        printf("Failed to create font\n");
    }
    text_font = lv_freetype_font_create("./my_demo/data/ttf/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                        20, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!text_font) {
        printf("Failed to create font\n");
    }
    lv_obj_remove_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF); // 关闭滚动条
    date_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(date_label, time_font, 0);
    lv_label_set_text(date_label, date_str);
    lv_obj_align(date_label, LV_ALIGN_TOP_RIGHT, -10, 10);

    time_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(time_label, time_font, 0);
    lv_label_set_text(time_label, time_str);
    lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, -29, 50);

    pthread_t update_sign_name_thread_id;
    pthread_create(&update_sign_name_thread_id, NULL, update_sign_name_task, NULL);

    // 创建一个容器
    sign_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(sign_container, 150, 50);                  // 设置容器大小
    lv_obj_align(sign_container, LV_ALIGN_TOP_RIGHT, -5, 100); // 设置容器位置
    lv_obj_set_flex_flow(sign_container, LV_FLEX_FLOW_ROW);    // 设置容器为横向布局
    lv_obj_set_flex_align(sign_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);               // 上下左右居中对齐
    lv_obj_set_style_pad_all(sign_container, 5, 0);            // 设置内部间距
    lv_obj_clear_flag(sign_container, LV_OBJ_FLAG_SCROLLABLE); // 禁用滚动

    // 员工标签
    lv_obj_t * sign_label = lv_label_create(sign_container);
    lv_label_set_text(sign_label, "员工:");
    lv_obj_set_style_text_font(sign_label, time_font, 0);

    // 签到人姓名标签
    sign_name_label = lv_label_create(sign_container);
    lv_label_set_text(sign_name_label, "未识别");
    lv_obj_set_style_text_font(sign_name_label, time_font, 0);
    lv_obj_set_style_text_color(sign_name_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_decor(sign_name_label, LV_TEXT_DECOR_UNDERLINE, 0);
    lv_label_set_long_mode(sign_name_label, LV_LABEL_LONG_SCROLL);

    // 签到按钮
    sign_in_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(sign_in_btn, 100, 50); // 设置容器大小
    lv_obj_align(sign_in_btn, LV_ALIGN_TOP_RIGHT, -29, 175);
    lv_obj_add_event_cb(sign_in_btn, sign_in_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 设置背景颜色为浅绿并添加透明效果
    lv_obj_set_style_bg_opa(sign_in_btn, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);                // 50% 透明
    lv_obj_set_style_bg_color(sign_in_btn, lv_color_hex(0x0000FF), LV_PART_MAIN | LV_STATE_DEFAULT); // 浅绿

    // 签到标签
    lv_obj_t * sign_in_label = lv_label_create(sign_in_btn);
    lv_label_set_text(sign_in_label, "签到");
    lv_obj_set_style_text_font(sign_in_label, time_font, 0);
    lv_obj_center(sign_in_label);

    // 录入按钮enter
    enter_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(enter_btn, 100, 50); // 设置容器大小
    lv_obj_align(enter_btn, LV_ALIGN_TOP_RIGHT, -29, 250);
    lv_obj_add_event_cb(enter_btn, enter_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 录入标签
    lv_obj_t * enter_label = lv_label_create(enter_btn);
    lv_label_set_text(enter_label, "录入");
    lv_obj_set_style_text_font(enter_label, time_font, 0);
    lv_obj_center(enter_label);

    // 拍照按钮
    photo_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(photo_btn, 100, 50); // 设置容器大小
    lv_obj_align(photo_btn, LV_ALIGN_TOP_RIGHT, -29, 175);
    lv_obj_add_flag(photo_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(photo_btn, photo_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 拍照标签
    lv_obj_t * photo_label = lv_label_create(photo_btn);
    lv_label_set_text(photo_label, "拍照");
    lv_obj_set_style_text_font(photo_label, time_font, 0);
    lv_obj_center(photo_label);

    // 返回按钮

    back_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(back_btn, 100, 50); // 设置容器大小
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -29, 250);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 返回标签
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "返回");
    lv_obj_set_style_text_font(back_label, time_font, 0);
    lv_obj_center(back_label);

    // 输入框
    create_num_keyboard(lv_scr_act());

    // 删除按钮
    delete_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(delete_btn, 100, 50); // 设置容器大小
    lv_obj_align(delete_btn, LV_ALIGN_TOP_RIGHT, -29, 325);
    lv_obj_add_event_cb(delete_btn, delete_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 设置背景颜色为浅绿并添加透明效果
    lv_obj_set_style_bg_opa(delete_btn, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);                // 50% 透明
    lv_obj_set_style_bg_color(delete_btn, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // 浅红
    // 删除标签
    lv_obj_t * delete_label = lv_label_create(delete_btn);
    lv_label_set_text(delete_label, "删除");
    lv_obj_set_style_text_font(delete_label, time_font, 0);
    lv_obj_center(delete_label);

    //  确认删除按钮
    confirm_delete_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(confirm_delete_btn, 100, 50); // 设置容器大小
    lv_obj_align(confirm_delete_btn, LV_ALIGN_TOP_RIGHT, -29, 175);
    lv_obj_add_flag(confirm_delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(confirm_delete_btn, confirm_delete_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 设置背景颜色为浅绿并添加透明效果
    lv_obj_set_style_bg_opa(confirm_delete_btn, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT); // 50% 透明
    lv_obj_set_style_bg_color(confirm_delete_btn, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // 浅红
    // 删除标签
    lv_obj_t * confirm_delete_label = lv_label_create(confirm_delete_btn);
    lv_label_set_text(confirm_delete_label, "确认删除");
    lv_obj_set_style_text_font(confirm_delete_label, time_font, 0);
    lv_obj_center(confirm_delete_label);

    // 返回按钮
    delete_back_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(delete_back_btn, 100, 50); // 设置容器大小
    lv_obj_align(delete_back_btn, LV_ALIGN_TOP_RIGHT, -29, 250);
    lv_obj_add_flag(delete_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(delete_back_btn, delete_back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 返回标签
    lv_obj_t * delete_back_label = lv_label_create(delete_back_btn);
    lv_label_set_text(delete_back_label, "返回");
    lv_obj_set_style_text_font(delete_back_label, time_font, 0);
    lv_obj_center(delete_back_label);

    // 签到数据成功标签
    sign_success_label = lv_label_create(lv_scr_act());
    lv_label_set_text(sign_success_label, "XX\n签到成功!");
    lv_obj_set_style_text_font(sign_success_label, time_font, 0);
    lv_obj_align(sign_success_label, LV_ALIGN_TOP_RIGHT, -20, 400);
    lv_obj_add_flag(sign_success_label, LV_OBJ_FLAG_HIDDEN);
}

void keyboard_event_handler(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
void create_num_keyboard(lv_obj_t * parent)
{

    // 创建键盘对象
    keyboard = lv_keyboard_create(parent);
    lv_keyboard_set_textarea(keyboard, input_box); // 绑定输入框

    // 自定义按键布局
    lv_keyboard_set_textarea(keyboard, NULL); // 禁用建议栏

    // 自定义QWERTY按键布局
    static const char * kb_map[] = {
        "Q", "W",  "E",  "R", "T", "Y", "U", "I",
        "O", "P",  "\n", // 第一行
        "A", "S",  "D",  "F", "G", "H", "J", "K",
        "L", "\n", // 第二行
        "Z", "X",  "C",  "V", "B", "N", "M", LV_SYMBOL_BACKSPACE,
        "\n" // 第三行
    };

    /*Set the relative width of the buttons and other controls*/
    static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    };

    // 设置自定义键盘布局
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);

    // 设置键盘位置
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_pos(keyboard, 0, 50);     // 调整位置
    lv_obj_set_size(keyboard, 500, 215); // 设置键盘大小

    // 事件处理
    lv_obj_add_event_cb(keyboard, keyboard_event_handler, LV_EVENT_ALL, NULL);

    // 创建输入框
    input_box = lv_textarea_create(parent);
    lv_obj_set_size(input_box, 140, 50);
    lv_obj_align(input_box, LV_ALIGN_TOP_RIGHT, -10, 100);
    lv_obj_set_style_text_font(input_box, text_font, 0);
    lv_obj_add_event_cb(input_box, ta_event_cb, LV_EVENT_ALL, keyboard);
    lv_obj_add_flag(input_box, LV_OBJ_FLAG_HIDDEN);
    // 输入框提示
    lv_textarea_set_placeholder_text(input_box, "姓名");
}

void keyboard_event_handler(lv_event_t * e)
{
    lv_obj_t * kb = static_cast<lv_obj_t *>(lv_event_get_target(e));
    // 处理键盘事件，例如获取输入
}
// 输入框回调函数
static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta        = static_cast<lv_obj_t *>(lv_event_get_target(e));
    lv_obj_t * kb        = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        // 让键盘永远在最上层
        lv_obj_move_foreground(keyboard);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}
