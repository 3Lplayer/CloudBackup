#pragma once
#include <iostream>
#include <pthread.h>
#include <unordered_map>
#include "file_util.hpp"
#include "conf.hpp"

namespace cloud
{
    class BackupInfo
    {
    public:
        BackupInfo()
        {
        }

        BackupInfo(const std::string &real_path)
        {
            FileUtil fu(real_path);
            // 如果打开的文件不存在
            if (fu.Exists() == false)
            {
                return;
            }
            // 初始化各成员
            _pack_flag = false;
            _size = fu.Size();
            _mtime = fu.LastMTime();
            _atime = fu.LastATime();
            _real_path = real_path;
            _pack_path = Conf::GetInstance().GetPackDir() + fu.FileName() + Conf::GetInstance().GetPackSuffix(); // 压缩文件存储路径格式: ./PackDir/filename.lz
            _url = Conf::GetInstance().GetDownloadPrefix() + fu.FileName();                                      // 下载路径格式: ./Download/filename
        }

    public:
        bool _pack_flag;        // 文件压缩标志
        size_t _size;           // 文件大小
        time_t _mtime;          // 文件最后修改时间
        time_t _atime;          // 文件最后访问时间
        std::string _real_path; // 文件真实存储路径
        std::string _pack_path; // 文件压缩后存储路径
        std::string _url;       // 文件下载路径
    };

    class DataManager
    {
    public:
        DataManager()
            : _backup_file(Conf::GetInstance().GetBackupFile())
        {
            pthread_rwlock_init(&_rwlock, nullptr); // 初始化
            InitLoad(); //加载持久化文件
        }

        // 将bi插入_hash,映射关系url:bi
        void Insert(const BackupInfo &bi)
        {
            pthread_rwlock_wrlock(&_rwlock); // 加锁
            _hash[bi._url] = bi;
            pthread_rwlock_unlock(&_rwlock); // 解锁
            Storage();
        }

        // 更新_hash中的映射关系
        void Update(const BackupInfo &bi)
        {
            pthread_rwlock_wrlock(&_rwlock); // 加锁
            _hash[bi._url] = bi;
            pthread_rwlock_unlock(&_rwlock); // 解锁
            Storage();
        }

        // 将_hash中url的映射赋值给bi
        bool GetBackupInfoByURL(const std::string &url, BackupInfo &bi)
        {
            const auto &it = _hash.find(url);
            //_hash中不存在该url
            if (it == _hash.end())
            {
                std::cerr << "DataManager::GetBackupInfoByURL(): URL does not exist." << std::endl;
                return false;
            }
            bi = it->second;
            return true;
        }

        // 将real_path对应的BackupInfo赋值给bi
        bool GetBackupInfoByRealPath(const std::string &real_path, BackupInfo &bi)
        {
            for (const auto &[_, v] : _hash)
            {
                if (v._real_path == real_path)
                {
                    bi = v;
                    return true;
                }
            }
            return false;
        }

        // 获取_hash中的所有BackupInfo
        bool GetAllBackupInfo(std::vector<BackupInfo> &arr)
        {
            for (const auto &[_, v] : _hash)
            {
                arr.push_back(v);
            }
        }

        //持久化存储 _hash -> backup.dat
        void Storage()
        {
            Json::Value root;
            for (const auto &[_, v] : _hash)
            {
                Json::Value tmp;
                tmp["pack_flag"] = v._pack_flag;
                tmp["size"] = (Json::Int64)v._size;
                tmp["mtime"] = (Json::Int64)v._mtime;
                tmp["atime"] = (Json::Int64)v._atime;
                tmp["real_path"] = v._real_path;
                tmp["pack_path"] = v._pack_path;
                tmp["url"] = v._url;
                root.append(tmp);
            }
            
            std::string sequence;
            JsonUtil::Serialize(root, sequence); //将序列化的数据赋值给sequence
            FileUtil fu(Conf::GetInstance().GetBackupFile());
            fu.Write(sequence); //将持久化数据写入持久化文件中
        }

        //加载持久化文件backup.dat的内容
        bool InitLoad()
        {
            FileUtil fu(_backup_file);
            //打开文件不存在
            if(fu.Exists() == false)
            {
                return false;
            }
            std::string body;
            fu.Read(body); //将持久化数据读取到body中
            Json::Value root;
            JsonUtil::Deserialize(root, body);
            //将数据加载到_hash
            for(int i = 0; i < root.size(); ++i)
            {
                BackupInfo bi;
                bi._pack_flag = root[i]["pack_flag"].asBool();
                bi._size = root[i]["size"].asInt64();
                bi._mtime = root[i]["mtime"].asInt64();
                bi._atime = root[i]["atime"].asInt64();
                bi._real_path = root[i]["real_path"].asString();
                bi._pack_path = root[i]["pack_path"].asString();
                bi._url = root[i]["url"].asString();
                _hash[bi._url] = bi;
            }
            return true;
        }

        ~DataManager()
        {
            pthread_rwlock_destroy(&_rwlock);
        }

    private:
        std::string _backup_file;
        pthread_rwlock_t _rwlock;                          // 读写锁(读共享,写互斥)
        std::unordered_map<std::string, BackupInfo> _hash; // key:url val:BackupInfo
    };
}