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

		//������ǽ������ļ�������ļ���ӵ�dm�Ĺ�����,���г־û��ļ�
		void Handle()
		{
			FileUtil fu(_backup_dir); //�򿪱���Ŀ¼
			fu.CreateDir(); //��������ļ������ھʹ���
			while (1)
			{
				std::vector<std::string> arr; //�洢�����ļ�
				fu.ScanDirPath(arr); //��ñ����ļ�
				//�������ļ�������Ҫ���ݵ��ļ�д��־û��ļ�
				for (const auto& s : arr)
				{
					//�жϸ��ļ��Ƿ���Ҫ�ϴ�
					if (UploadCheck(s))
					{
						//����ϴ�����˳ɹ�������_dm���������
						if (Upload(s))
						{
							_dm->Insert(s, GetFileFlag(s)); //����������������_dm
							std::cout << s << " upload success." << std::endl;
						}
					}
				}
				Sleep(1);
			}
		}

		//�ж�һ���ļ��Ƿ���Ҫ�ϴ�,����������������֮һ����Ҫ�ϴ�:
		//1.���ļ�
		//2.���ļ�,���Ǿ����޸�
		bool UploadCheck(const std::string& file_name)
		{
			std::string old_flag; //��ȡ���ļ���Ψһ��ʶ
			//�������ֵΪfalse��ʾ��ǰ��һ�����ļ��ϴ�
			if (_dm->GetFlagByPath(file_name, old_flag))
			{
				//��ʾ�ļ�û�б��޸Ĺ�,����Ҫ�����ϴ�
				if (old_flag == GetFileFlag(file_name))
				{
					return false;
				}
			}
			//�ߵ����ʾ��Ҫ�ϴ�
			//���Ǵ�ʱ������һ���������,���Ǵ��ϴ����ļ�̫����,��������д�뱸���ļ����ڼ�Ψһ��ʶ�ͻ�һֱ�ı�,��˻ᵼ����д���ڼ�һֱ�ϴ��������,������
			//��˹涨һ���ļ������5s�ڱ��޸Ĺ�,�ͱ�ʾ���ļ�������Ϊ���̫���ڳ���д�뱸���ļ�����,�������ϴ������
			if (time(nullptr) - FileUtil(file_name).LastMTime() < 5)
			{
				return false;
			}
			return true;
		}

		//�ϴ���Ҫ���ݵ��ļ���������
		bool Upload(const std::string& file_name)
		{
			FileUtil fu(file_name); //���ϴ��ļ�
			std::string body;
			fu.Read(body); //��ȡ���ϴ��ļ�����

			httplib::Client client(SERVER_IP, SERVER_PORT); //ʵ�����ͻ��˶���

			//��֯�ϴ�����
			httplib::MultipartFormData data;
			data.name = "file"; //��־λ,���������ȡ����һ��
			data.filename = fu.FileName();
			data.content = body;
			data.content_type = "application/octet-stream";
			httplib::MultipartFormDataItems items;
			items.push_back(data);
			
			//�ϴ��ķ����
			auto ret = client.Post("/upload", items);
			//�ϴ�������
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
		//��ȡ�ļ���Ψһ��ʶ -- ��ʽ: �ļ���-��С-����޸�ʱ��
		std::string GetFileFlag(const std::string& file_name)
		{
			FileUtil fu(file_name);
			std::stringstream ss;
			ss << file_name << "-" << fu.Size() << "-" << fu.LastMTime();
			return ss.str();
		}

	private:
		std::string _backup_dir; //�����ļ��ļ���
		DataManager* _dm; //���ݹ�����
	};
}