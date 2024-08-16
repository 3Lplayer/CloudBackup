#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <experimental/filesystem>
#include <jsoncpp/json/json.h>
#include <memory>
#include <sstream>
#include "bundle.h"

namespace cloud
{
    class FileUtil
    {
    public:
        FileUtil(const std::string &file_path) : _file_path(file_path)
        {
        }
        // 获取文件大小
        size_t Size()
        {
            struct stat st; // 文件属性结构体
            // 获取指定文件的属性
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::Size(): stat() error." << std::endl;
                return -1;
            }
            return st.st_size;
        }

        // 获取最后修改时间
        time_t LastMTime()
        {
            struct stat st; // 文件属性结构体
            // 获取指定文件的属性
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::LastMTime(): stat() error." << std::endl;
                return -1;
            }
            return st.st_mtime;
        }

        // 获取最后访问时间
        time_t LastATime()
        {
            struct stat st; // 文件属性结构体
            // 获取指定文件的属性
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::LastATime(): stat() error." << std::endl;
                return -1;
            }
            return st.st_atime;
        }

        // 获取路劲中的文件名 -- ./a/b/c/d.txt -> d.txt
        std::string FileName()
        {
            size_t pos = _file_path.rfind('/'); // 从后往前开始找
            // 找不到,表示当前就只有文件名
            if (pos == std::string::npos)
            {
                return _file_path;
            }
            // 找到了截取返回
            return _file_path.substr(pos + 1);
        }

        bool Remove()
        {   
            //删除失败
            if(remove(_file_path.c_str()) == -1)
            {
                std::cerr << "FileUtil::Remove(): remove is error." << std::endl;
                return false;
            }
            return true;
        }

        //传入一个文件body,获取文件_file_path的指定大小的数据内容
        bool Substr(std::string& body, size_t pos, size_t len)
        {
            //所要获取大小大于文件大小
            if(pos + len > Size())
            {
                std::cerr << "FileUtil::Substr(): pos+len is greater than the file size." << std::endl;
                return false;
            }
            std::ifstream ifs;
            ifs.open(_file_path, std::ios::binary);
            //打开失败
            if(ifs.is_open() == false)
            {
                std::cerr << "FileUtil::Substr(): ifstream open error." << std::endl;
                ifs.close();
                return false;
            }
            ifs.seekg(pos, std::ios::beg); //偏移文件指针到pos位置
            body.resize(len);
            ifs.read(&body[0], len); //读取指定大小数据到body
            //读取出错
            if (ifs.good() == false)
            {
                std::cerr << "FileUtil::Substr(): ifstream read error." << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        //传入一个文件body,读取文件_file_path的全部数据 -- _file_path -> body
        bool Read(std::string& body)
        {
            return Substr(body, 0, Size());
        }

        //传入一个文件body,将其数据全部写入文件_file_path -- body -> _file_path
        bool Write(const std::string& body)
        {
            std::ofstream ofs;
            ofs.open(_file_path, std::ios::binary);
            //打开失败
            if(ofs.is_open() == false)
            {
                std::cerr << "FileUtil::Write(): ofstream open error." << std::endl;
                ofs.close();
                return false;
            }
            _file_path.resize(body.size());
            ofs.write(&body[0], body.size());
            //写入出错
            if (ofs.good() == false)
            {
                std::cerr << "FileUtil::Write(): ofstream write error." << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        //将文件_file_path的数据压缩,压缩后的文件名为pack_name -- _file_path -> pack_name
        bool Pack(std::string& pack_name)
        {
            std::string body; //存放需要被压缩的数据
            Read(body);
            std::string pack = bundle::pack(bundle::LZIP, body); //获取文件压缩后的数据
            std::ofstream ofs;
            ofs.open(pack_name, std::ios::binary);
            //打开失败
            if(ofs.is_open() == false)
            {
                std::cerr << "FileUtil::Pack(): ofstream open error." << std::endl;
                ofs.close();
                return false;
            }
            ofs.write(&pack[0], pack.size()); //将压缩数据写入pack_name
            //写入出错
            if (ofs.good() == false)
            {
                std::cerr << "FileUtil::Pack(): ofstream write error." << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        //对压缩文件pack_name进行解压,解压后的数据存储在_file_path -- pack_name -> _file_path
        bool Unpack(std::string& pack_name)
        {
            std::string body; //存放压缩后的数据
            FileUtil fu(pack_name);
            fu.Read(body);
            std::string unpack = bundle::unpack(body); //获取解压后的文件数据
            std::ofstream ofs;
            ofs.open(_file_path, std::ios::binary);
            //打开失败
            if(ofs.is_open() == false)
            {
                std::cerr << "FileUtil::Unpack(): ofstream open error." << std::endl;
                ofs.close();
                return false;
            }
            _file_path.resize(unpack.size());
            ofs.write(&unpack[0], unpack.size()); //将解压数据写入unpack_name
            //读取出错
            if (ofs.good() == false)
            {
                std::cerr << "FileUtil::Unpack(): ofstream read error." << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        //判断_file_path是否存在
        bool Exists()
        {
            return std::experimental::filesystem::exists(_file_path);
        }

        //创建目录
        bool CreateDir()
        {
            //需要创建的目录已经存在
            if(Exists())
            {
                return true;
            }
            return std::experimental::filesystem::create_directories(_file_path);
        }

        //将一个路径中的最后一个子目录的文件以相对路径的形式添加到arr
        void ScanDirPath(std::vector<std::string>& arr)
        {
            for(const auto& p : std::experimental::filesystem::directory_iterator(_file_path))
            {
                //如果是单纯的目录就不添加到arr中
                if(std::experimental::filesystem::is_directory(p))
                {
                    continue;
                }
                //p并不是一个string,不能直接添加到arr中,需要经过处理
                arr.push_back(std::experimental::filesystem::path(p).relative_path().string());
            }
        }
    private:
        std::string _file_path;
    };

    class JsonUtil
    {
    public:
        //序列化
        static void Serialize(const Json::Value& root, std::string& sequence)
        {
            std::unique_ptr<Json::StreamWriter> sw(Json::StreamWriterBuilder().newStreamWriter()); //智能指针管理StreamWriterBuilder建造者建造出来的StreamWriter对象
            std::stringstream ss; //字符流
            sw->write(root, &ss);
            sequence = ss.str();
        }

        //反序列化
        static void Deserialize(Json::Value& root, const std::string& sequence)
        {
            std::unique_ptr<Json::CharReader> cr(Json::CharReaderBuilder().newCharReader()); //智能指针管理CharReaderBuilder建造者建造出来的CharReader对象
            std::string err; //存储错误信息
            cr->parse(sequence.c_str(), sequence.c_str() + sequence.size(), &root, &err);
        }
    };
}