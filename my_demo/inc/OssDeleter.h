#ifndef OSS_DELETER_H
#define OSS_DELETER_H

#include <alibabacloud/oss/OssClient.h>
#include <alibabacloud/oss/OssError.h>
#include <alibabacloud/oss/ServiceRequest.h>
#include <string>
using namespace std;
using namespace AlibabaCloud::OSS;

class OssDeleter {
private:
    // OSS 客户端
    OssClient* client;

    // 列出并删除文件夹下的所有对象
    void listAndDeleteObjects(const string& bucketName, const string& ossDir);

    // 删除单个对象
    void deleteObject(const string& bucketName, const string& objectKey);

public:
    // 构造函数：初始化 OSS 客户端
    OssDeleter(const string& accessKeyId, const string& accessKeySecret, const string& endpoint);
    
    // 析构函数：清理 OSS 客户端
    ~OssDeleter();

    // 删除 OSS 文件夹（即删除所有具有指定前缀的对象）
    void deleteFolder(const string& bucketName, const string& ossDir);

};

#endif  // OSS_DELETER_H
