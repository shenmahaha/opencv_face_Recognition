#ifndef OSS_UPLOADER_H
#define OSS_UPLOADER_H

#include <alibabacloud/oss/OssClient.h>
#include <alibabacloud/oss/OssError.h>
#include <alibabacloud/oss/ServiceRequest.h>
#include <string>

using namespace AlibabaCloud::OSS;
using namespace std;
class OssUploader {
private:
    // OSS 客户端
    OssClient* client;
    // 上传单个文件
    void uploadFile(const string& localFilePath, const string& bucketName, const string& ossObjectName);
    
    // 递归上传文件夹
    int uploadDirectoryRecursive(const string& localDir, const string& bucketName, const string& ossDir);
    
public:
    OssUploader(const string& accessKeyId, const string& accessKeySecret, const string& endpoint);
    ~OssUploader();

    // 上传文件夹到 OSS
    int uploadDirectory(const string& localDir, const string& bucketName, const string& ossDir);


};
#endif  // OSS_UPLOADER_H
