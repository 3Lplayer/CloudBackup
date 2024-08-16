#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sstream>

namespace cloud
{
    class FileUtil
    {
    public:
        FileUtil(const std::string &file_path) : _file_path(file_path)
        {
        }
        size_t Size()
        {
            struct stat st; 
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::Size(): stat() error." << std::endl;
                return -1;
            }
            return st.st_size;
        }

        time_t LastMTime()
        {
            struct stat st;     
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::LastMTime(): stat() error." << std::endl;
                return -1;
            }
            return st.st_mtime;
        }

        time_t LastATime()
        {
            struct stat st; 
            if (stat(_file_path.c_str(), &st) == -1)
            {
                std::cout << "FileUtil::LastATime(): stat() error." << std::endl;
                return -1;
            }
            return st.st_atime;
        }
      
        std::string FileName()
        {
            return std::filesystem::path(_file_path).filename().string();
        }

        bool Remove()
        {   
            if(remove(_file_path.c_str()) == -1)
            {
                std::cerr << "FileUtil::Remove(): remove is error." << std::endl;
                return false;
            }
            return true;
        }

        bool Substr(std::string& body, size_t pos, size_t len)
        {
            if(pos + len > Size())
            {
                std::cerr << "FileUtil::Substr(): pos+len is greater than the file size." << std::endl;
                return false;
            }
            std::ifstream ifs;
            ifs.open(_file_path, std::ios::binary);
            
            if(ifs.is_open() == false)
            {
                std::cerr << "FileUtil::Substr(): ifstream open error." << std::endl;
                ifs.close();
                return false;
            }
            ifs.seekg(pos, std::ios::beg); 
            body.resize(len);
            ifs.read(&body[0], len); 
           
            if (ifs.good() == false)
            {
                std::cerr << "FileUtil::Substr(): ifstream read error." << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        bool Read(std::string& body)
        {
            return Substr(body, 0, Size());
        }

        bool Write(const std::string& body)
        {
            std::ofstream ofs;
            ofs.open(_file_path, std::ios::binary);
            
            if(ofs.is_open() == false)
            {
                std::cerr << "FileUtil::Write(): ofstream open error." << std::endl;
                ofs.close();
                return false;
            }
            _file_path.resize(body.size());
            ofs.write(&body[0], body.size());
            
            if (ofs.good() == false)
            {
                std::cerr << "FileUtil::Write(): ofstream write error." << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        bool Exists()
        {
            return std::filesystem::exists(_file_path);
        }

        bool CreateDir()
        {
            if(Exists())
            {
                return true;
            }
            return std::filesystem::create_directories(_file_path);
        }
  
        void ScanDirPath(std::vector<std::string>& arr)
        {
            for(const auto& p : std::filesystem::directory_iterator(_file_path))
            {
                if(std::filesystem::is_directory(p))
                {
                    continue;
                }
                arr.push_back(std::filesystem::path(p).relative_path().string());
            }
        }
    private:
        std::string _file_path;
    };
}