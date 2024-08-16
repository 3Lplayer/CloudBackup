#pragma once
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "data.hpp"
#include "httplib.h"

#define SERVER_IP "110.41.83.189"
#define SERVER_PORT 8080

namespace cloud
{
	class Backup
	{
	public:
		Backup(const std::string& backup_dir, const std::string& backup_file)
			:_backup_dir(backup_dir)
			, _dm(new DataManager(backup_file))
		{
		}

		//任务就是将备份文件夹里的文件添加到dm的管理中,进行持久化文件
		void Handle()
		{
			FileUtil fu(_backup_dir); //打开备份目录
			fu.CreateDir(); //如果备份文件不存在就创建
			while (1)
			{
				std::vector<std::string> arr; //存储备份文件
				fu.ScanDirPath(arr); //获得备份文件
				//将备份文件夹里需要备份的文件写入持久化文件
				for (const auto& s : arr)
				{
					//判断该文件是否需要上传
					if (UploadCheck(s))
					{
						//如果上传服务端成功则新增_dm管理的数据
						if (Upload(s))
						{
							_dm->Insert(s, GetFileFlag(s)); //将备份数据新增到_dm
							std::cout << s << " upload success." << std::endl;
						}
					}
				}
				Sleep(1);
			}
		}

		//判断一个文件是否需要上传,满足以下两个条件之一就需要上传:
		//1.新文件
		//2.旧文件,但是经过修改
		bool UploadCheck(const std::string& file_name)
		{
			std::string old_flag; //获取该文件的唯一标识
			//如果返回值为false表示当前是一个新文件上传
			if (_dm->GetFlagByPath(file_name, old_flag))
			{
				//表示文件没有被修改过,不需要重新上传
				if (old_flag == GetFileFlag(file_name))
				{
					return false;
				}
			}
			//走到这表示需要上传
			//但是此时还存在一种特殊情况,就是待上传的文件太大了,这样它在写入备份文件夹期间唯一标识就会一直改变,因此会导致在写入期间一直上传到服务端,不合理
			//因此规定一个文件如果在5s内被修改过,就表示该文件可能因为体积太大还在持续写入备份文件夹中,不予以上传服务端
			if (time(nullptr) - FileUtil(file_name).LastMTime() < 5)
			{
				return false;
			}
			return true;
		}

		//上传需要备份的文件到服务器
		bool Upload(const std::string& file_name)
		{
			FileUtil fu(file_name); //打开上传文件
			std::string body;
			fu.Read(body); //读取待上传文件数据

			httplib::Client client(SERVER_IP, SERVER_PORT); //实例化客户端对象

			//组织上传数据
			httplib::MultipartFormData data;
			data.name = "file"; //标志位,与服务器获取的须一致
			data.filename = fu.FileName();
			data.content = body;
			data.content_type = "application/octet-stream";
			httplib::MultipartFormDataItems items;
			items.push_back(data);
			
			//上传的服务端
			auto ret = client.Post("/upload", items);
			//上传有问题
			if (!ret || ret->status != 200)
			{
				return false;
			}
			return true;
		}

		~Backup()
		{
			delete _dm;
		}
	private:
		//获取文件的唯一标识 -- 格式: 文件名-大小-最后修改时间
		std::string GetFileFlag(const std::string& file_name)
		{
			FileUtil fu(file_name);
			std::stringstream ss;
			ss << file_name << "-" << fu.Size() << "-" << fu.LastMTime();
			return ss.str();
		}

	private:
		std::string _backup_dir; //备份文件文件夹
		DataManager* _dm; //数据管理类
	};
}