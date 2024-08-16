#pragma once
#include <iostream>
#include <unordered_map>
#include <sstream>
#include "file_util.hpp"

namespace cloud
{
	class DataManager
	{
	public:
		DataManager(const std::string& backup_file)
			:_backup_file(backup_file)
		{
			InitLoad();
		}

		//���ݳ־û�
		void Storage()
		{
			FileUtil fu(_backup_file); //�򿪳־û��ļ�
			//�ļ��洢��ʽ: key + �ո� + val + '\n'
			std::stringstream ss;
			for (const auto& [k, v] : _hash)
			{
				ss << k << " " << v << '\n';
			}
			fu.Write(ss.str()); //������д��־û��ļ�
		}
		
		//���س־û��ļ����ݵ�_hash
		void InitLoad()
		{
			FileUtil fu(_backup_file); //�򿪳־û��ļ�
			std::vector<std::string> arr1;
			std::string body;
			fu.Read(body); //��ȡ�־û��ļ�����
			Split(body, "\n", arr1);
			for (const auto& s : arr1)
			{
				std::vector<std::string> tmp;
				Split(s, " ", tmp);
				_hash[tmp[0]] = tmp[1];
			}
		}
		
		//����
		bool Insert(const std::string& path, const std::string& flag)
		{
			_hash[path] = flag;
			Storage();
			return true;
		}

		//����
		bool Update(const std::string& path, const std::string& flag)
		{
			_hash[path] = flag;
			Storage();
			return true;
		}

		//����path���_hash��ӳ���flag
		bool GetFlagByPath(const std::string& path, std::string& flag)
		{
			auto it = _hash.find(path);
			if (it == _hash.end())
			{
				return false;
			}
			flag = it->second;
			return true;
		}
	private:
		//���ݷָ�
		void Split(const std::string& body, const std::string& sep, std::vector<std::string>& arr)
		{
			size_t begin = 0;
			size_t pos = body.find(sep, begin);
			while (pos != std::string::npos)
			{
				//���sep���ڵ����
				if (pos == begin)
				{
					begin = pos + sep.size();
					pos = body.find(sep, begin);
					continue;
				}
				arr.push_back(body.substr(begin, pos - begin));
				//�����±�
				begin = pos + sep.size();
				pos = body.find(sep, begin);
			}
			if (begin < body.size())
			{
				arr.push_back(body.substr(begin));
			}
		}

	private:
		std::string _backup_file; //�־û��ļ�
		std::unordered_map<std::string, std::string> _hash; //key:�ļ�·�� val:key��Ψһ��ʶ
	};
}