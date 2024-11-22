#include "../../inc/OssUploader.h"
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>

OssUploader::OssUploader(const string& accessKeyId, const string& accessKeySecret, const string& endpoint) {
    ClientConfiguration config;
    client = new OssClient(endpoint, accessKeyId, accessKeySecret, config);
}

OssUploader::~OssUploader() {
    delete client;
}

int OssUploader::uploadDirectory(const string& localDir, const string& bucketName, const string& ossDir) {
    return uploadDirectoryRecursive(localDir, bucketName, ossDir);
}

int OssUploader::uploadDirectoryRecursive(const string& localDir, const string& bucketName, const string& ossDir) {
    DIR* dir = opendir(localDir.c_str());
    if (dir == nullptr) {
        cerr << "无法打开目录: " << localDir << endl;
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        string entryPath = localDir + "/" + entry->d_name;
        string objectName = ossDir + "/" + entry->d_name;

        if (entry->d_type == DT_DIR) {
            uploadDirectoryRecursive(entryPath, bucketName, objectName);  // 递归上传文件夹
        } else if (entry->d_type == DT_REG) {
            uploadFile(entryPath, bucketName, objectName);  // 上传文件
        }
    }

    closedir(dir);  // 关闭目录
    return 0;
}

void OssUploader::uploadFile(const string& localFilePath, const string& bucketName, const string& ossObjectName) {
    cout << "上传文件: " << localFilePath << " 到 OSS 路径: " << ossObjectName << endl;
    
    auto outcome = client->PutObject(bucketName, ossObjectName, localFilePath);
    if (outcome.isSuccess()) {
        cout << "上传成功: " << ossObjectName << endl;
    } else {
        cout << "上传失败: " << ossObjectName << endl;
        cout << "错误信息: " << outcome.error().Message() << endl;
    }
}
