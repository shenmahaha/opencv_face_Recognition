# 基于Opencv的人脸识别签到系统（客户端）（LVGL+C++）

## 使用技术
- LVGL9.1
- opencv
- TCP
- OSS

## 界面截图

![1](https://github.com/user-attachments/assets/0b67d004-6867-4732-b435-48b93baaff04)

## 第一步

下载源码到ubuntu20.04（已配置好opencv环境，阿里云OSS环境），并在编译器中打开。

## 第二步

修改相关配置

![image](https://github.com/user-attachments/assets/2837f2c6-2c73-4073-b71a-d40152e3cd22)

脸部特征模型路径可以不改

![image](https://github.com/user-attachments/assets/42ec633f-b6e4-4525-a5c2-74ddd16035b3)

## 第三步

```
make clean

make -j16
```




