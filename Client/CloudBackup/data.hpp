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

		//数据持久化
		void Storage()
		{
			FileUtil fu(_backup_file); //打开持久化文件
			//文件存储格式: key + 空格 + val + '\n'
			std::stringstream ss;
			for (const auto& [k, v] : _hash)
			{
				ss << k << " " << v << '\n';
			}
			fu.Write(ss.str()); //将数据写入持久化文件
		}
		
		//加载持久化文件数据到_hash
		void InitLoad()
		{
			FileUtil fu(_backup_file); //打开持久化文件
			std::vector<std::string> arr1;
			std::string body;
			fu.Read(body); //获取持久化文件数据
			Split(body, "\n", arr1);
			for (const auto& s : arr1)
			{
				std::vector<std::string> tmp;
				Split(s, " ", tmp);
				_hash[tmp[0]] = tmp[1];
			}
		}
		
		//插入
		bool Insert(const std::string& path, const std::string& flag)
		{
			_hash[path] = flag;
			Storage();
			return true;
		}

		//更新
		bool Update(const std::string& path, const std::string& flag)
		{
			_hash[path] = flag;
			Storage();
			return true;
		}

		//根据path获得_hash中映射的flag
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
		//数据分割
		void Split(const std::string& body, const std::string& sep, std::vector<std::string>& arr)
		{
			size_t begin = 0;
			size_t pos = body.find(sep, begin);
			while (pos != std::string::npos)
			{
				//多个sep相邻的情况
				if (pos == begin)
				{
					begin = pos + sep.size();
					pos = body.find(sep, begin);
					continue;
				}
				arr.push_back(body.substr(begin, pos - begin));
				//更新下标
				begin = pos + sep.size();
				pos = body.find(sep, begin);
			}
			if (begin < body.size())
			{
				arr.push_back(body.substr(begin));
			}
		}

	private:
		std::string _backup_file; //持久化文件
		std::unordered_map<std::string, std::string> _hash; //key:文件路径 val:key的唯一标识
	};
}