#include "../../inc/OssDeleter.h"
#include <iostream>


using namespace std;
using namespace AlibabaCloud::OSS;

OssDeleter::OssDeleter(const string& accessKeyId, const string& accessKeySecret, const string& endpoint) {
    // 初始化 OSS 客户端
    ClientConfiguration config;
    client = new OssClient(endpoint, accessKeyId, accessKeySecret, config);
}

OssDeleter::~OssDeleter() {
    // 销毁 OSS 客户端
    delete client;
}

void OssDeleter::deleteFolder(const string& bucketName, const string& ossDir) {
    // 调用列出文件并删除的函数
    listAndDeleteObjects(bucketName, ossDir);
}

void OssDeleter::listAndDeleteObjects(const string& bucketName, const string& ossDir) {
    ListObjectsRequest request(bucketName);
    request.setPrefix(ossDir);  // 设置文件夹前缀
    auto outcome = client->ListObjects(request);

    if (outcome.isSuccess()) {
        const auto& objectSummaries = outcome.result().ObjectSummarys();

        // 循环删除每一个对象
        for (const auto& objectSummary : objectSummaries) {
            string objectName = objectSummary.Key();
            deleteObject(bucketName, objectName);
        }
    } else {
        cout << "列出对象失败: " << outcome.error().Message() << endl;
    }
}

void OssDeleter::deleteObject(const string& bucketName, const string& objectKey) {
    auto outcome = client->DeleteObject(bucketName, objectKey);
    if (outcome.isSuccess()) {
        cout << "成功删除对象: " << objectKey << endl;
    } else {
        cout << "删除对象失败: " << objectKey << endl;
        cout << "错误信息: " << outcome.error().Message() << endl;
    }
}
