#pragma once
#include <iostream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include "data.hpp"
#include "file_util.hpp"
#include "conf.hpp"

extern cloud::DataManager dm;

namespace cloud
{
    class Hotspot
    {
    public:
        void Handle()
        {
            //如果备份文件文件夹和压缩文件文件夹不存在则创建
            FileUtil backup_dir(Conf::GetInstance().GetBackupDir());
            backup_dir.CreateDir();
            FileUtil pack_dir(Conf::GetInstance().GetPackDir());
            pack_dir.CreateDir();

            while(1)
            {
                //获取备份文件文件夹里面的备份文件
                std::vector<std::string> backup_files;
                backup_dir.ScanDirPath(backup_files);
                //检查获取到的备份文件中是否存在非热点文件,对非热点文件进行压缩处理
                for(int i = 0; i < backup_files.size(); ++i)
                {
                    if(Check(backup_files[i]))
                    {
                        FileUtil pack_file(backup_files[i]);
                        BackupInfo bi(backup_files[i]);
                        pack_file.Pack(bi._pack_path); //对非热点文件进行压缩
                        pack_file.Remove(); //删除原备份文件
                        bi._pack_flag = true; //更新压缩标志
                        dm.Update(bi); 
                    }
                }
                sleep(1); //每间隔1s检测一次备份文件文件夹
            }
        }
    private:
        //检查file是否为非热点文件 -- 非热点文件: 当前时间 - 最后访问时间 > 配置文件中的hot_time
        bool Check(const std::string& file)
        {   
            FileUtil fu(file);
            if(time(nullptr) - fu.LastATime() > Conf::GetInstance().GetHotTime())
            {
                return true;
            }
            return false;
        }
    };
}