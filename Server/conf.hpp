#pragma once
#include <iostream>
#include "file_util.hpp"

#define CONF_FILE "./cloud.conf" // 配置文件

namespace cloud
{
    class Conf
    {
    private:
        Conf()
        {
            FileUtil fu(CONF_FILE); // 打开配置文件
            std::string body;
            if(fu.Read(body) == false); // 将配置文件读到body中
            Json::Value root;
            JsonUtil::Deserialize(root, body); // 将配置文件数据反序列化

            // 初始化各成员变量
            _hot_time = root["hot_time"].asInt();
            _ip = root["ip"].asString();
            _port = root["port"].asInt();
            _download_prefix = root["download_prefix"].asString();
            _pack_suffix = root["pack_suffix"].asString();
            _backup_dir = root["backup_dir"].asString();
            _pack_dir = root["pack_dir"].asString();
            _backup_file = root["backup_file"].asString();
        }

        Conf(const Conf &conf) = delete;
        Conf &operator=(const Conf &conf) = delete;

    public:
        static Conf &GetInstance()
        {
            static Conf conf; // C++11起,静态变量将能够在满⾜线程安全的前提下唯⼀地被构造和析构
            return conf;
        }

        // 获取各成员变量
        size_t GetHotTime()
        {
            return _hot_time;
        }

        std::string GetIp()
        {
            return _ip;
        }

        size_t GetPort()
        {
            return _port;
        }

        std::string GetDownloadPrefix()
        {
            return _download_prefix;
        }

        std::string GetPackSuffix()
        {
            return _pack_suffix;
        }

        std::string GetBackupDir()
        {
            return _backup_dir;
        }

        std::string GetPackDir()
        {
            return _pack_dir;
        }

        std::string GetBackupFile()
        {
            return _backup_file;
        }

    private:
        size_t _hot_time; // 热点判断时间
        std::string _ip;
        size_t _port;
        std::string _download_prefix; // 请求下载前缀路径
        std::string _pack_suffix;     // 压缩格式
        std::string _backup_dir;      // 备份文件文件夹
        std::string _pack_dir;        // 压缩文件文件夹
        std::string _backup_file;     // 持久化备份文件
    };
}