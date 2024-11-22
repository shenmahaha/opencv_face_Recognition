#include "../../inc/TCP_client.h"
using namespace std;
TCPClient::TCPClient(const std::string& server_ip, int server_port)
    : server_ip(server_ip), server_port(server_port), client_socket(-1), receiving(false)
{
    // 创建客户端套接字
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址信息
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接服务器失败");
        close(client_socket);  // 确保连接失败时关闭套接字
        exit(EXIT_FAILURE);
    }

    std::cout << "已连接到服务器 " << server_ip << ":" << server_port << std::endl;
}

TCPClient::~TCPClient() {
    stop_receiving();  // 停止接收线程
    if (client_socket >= 0) {
        close(client_socket);  // 关闭套接字
    }
}

int TCPClient::send_data(const std::string& data) {
    std::lock_guard<std::mutex> lock(socket_mutex);  // 使用互斥锁保护套接字
    if (client_socket < 0) {
        std::cerr << "套接字无效，无法发送数据" << std::endl;
        return -1;
    }

    ssize_t bytes_sent = send(client_socket, data.c_str(), data.length(), 0);
    if (bytes_sent < 0) {
        perror("发送数据失败");
        return -1;
    }
    std::cout << "成功发送数据: " << data << std::endl;
    return 0;
}

void TCPClient::start_receiving() {
    receiving = true;  // 设置接收线程标志为 true
    receive_thread = std::thread(&TCPClient::receive_data, this);  // 启动接收线程
}

void TCPClient::stop_receiving() {
    receiving = false;  // 设置接收线程标志为 false
    if (receive_thread.joinable()) {
        receive_thread.join();  // 等待接收线程结束
    }
}

void distinguish_data(char * buf);
void processing_staff_data(char* buf);
void checkInSuccessfulAndReturn(char * buf);
void TCPClient::receive_data() {
    char buffer[1024] = {0};  // 用于接收数据的缓冲区

    while (receiving) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("接收数据失败");
            break;
        } else if (bytes_received == 0) {
            std::cout << "服务器关闭了连接" << std::endl;
            break;
        }

        buffer[bytes_received] = '\0';  // 确保接收到的数据以NULL结尾
        std::cout << "接收到的数据: " << buffer << std::endl;
        distinguish_data(buffer);
    }

    std::cout << "接收线程停止" << std::endl;
}

void distinguish_data(char * buf)
{

    string data(buf);
    size_t pos = data.find("data_id:");
    if(pos != string::npos) {
        pos += 8;                                            // Move past "data_id:"
        size_t end_pos = data.find(' ', pos);                // Find the next space after the data_id
        if(end_pos == string::npos) end_pos = data.length(); // Handle case where data_id is at the end
        string data_id_str = data.substr(pos, end_pos - pos);
        int data_id        = stoi(data_id_str); // Convert the string to an integer

        // 根据 data_id 判断处理类型
        if(data_id == 3) {
            processing_staff_data(buf);
        }else if(data_id == 5) {
            checkInSuccessfulAndReturn(buf);
        }
    }
}
// 获取人员信息
void processing_staff_data(char* buf) {
    // 定义需要解析的变量
    int data_id = 0;
    long long id = 0;
    char name[100] = {0};  // 假设名字长度不超过 100

    // 使用 sscanf 来解析字符串中的数据
    // 格式：data_id:<data_id> id:<id> name:<name>
    sscanf(buf, "data_id:%d id:%lld name:%99[^\n]", &data_id, &id, name);

    // 输出解析后的数据（可以根据实际需要进行处理）
    char id_str[50];
    sprintf(id_str,"%lld",id);
    staff_vec.push_back(Staff(string(id_str), string(name)));
}
//签到成功返回
void checkInSuccessfulAndReturn(char * buf) {
       // 定义需要解析的变量
    int data_id = 0;
    long long id = 0;
    char name[100] = {0};  // 假设名字长度不超过 100

    // 使用 sscanf 来解析字符串中的数据
    // 格式：data_id:<data_id> id:<id> name:<name>
    sscanf(buf, "data_id:%d id:%lld name:%99[^\n]", &data_id, &id, name);

    // 输出解析后的数据（可以根据实际需要进行处理）
    char id_str[50];
    sprintf(id_str,"%lld",id);
    string str=string(name)+"\n"+"签到成功!";
    lv_label_set_text(sign_success_label, str.c_str());
    lv_obj_clear_flag(sign_success_label, LV_OBJ_FLAG_HIDDEN);
}
