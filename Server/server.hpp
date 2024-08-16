#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "data.hpp"
#include "httplib.h"
#include "conf.hpp"

extern cloud::DataManager dm;

namespace cloud
{
    class Service
    {
    public:
        void Handle()
        {
            _server.Post("/upload", Upload);
            _server.Get("/listshow", ListShow);
            _server.Get("/", ListShow);                                                  // 如果用户没有携带请求资源,默认返回备份文件列表
            std::string download_req = Conf::GetInstance().GetDownloadPrefix() + "(.*)"; //(.*): 正则表达式,表示匹配任意字符
            _server.Get(download_req, Download);
            if (_server.listen(Conf::GetInstance().GetIp(), Conf::GetInstance().GetPort()) == false)
            {
                std::cerr << "listen error." << std::endl;
            }
        }

    private:
        // 上传处理函数
        static bool Upload(const httplib::Request &req, httplib::Response &res)
        {
            auto ret = req.has_file("file"); // 判断是否有name为file的上传文件
            // 没有
            if (ret == false)
            {
                res.status = 400;
                return false;
            }
            const auto &file = req.get_file_value("file"); // 获取该上传文件字段
            std::string backup_file = Conf::GetInstance().GetBackupDir() + FileUtil(file.filename).FileName();
            FileUtil fu(backup_file);
            fu.Write(file.content);     // 将上传文件数据写入备份文件
            BackupInfo bi(backup_file); // 用备份目录+文件名构造bi
            dm.Insert(bi);              // 上传到_hash
        }

        // 备份文件列表展示
        static void ListShow(const httplib::Request &req, httplib::Response &res)
        {
            // 获取所有备份文件
            std::vector<BackupInfo> arr;
            dm.GetAllBackupInfo(arr);
            // 组织html格式一一展示备份文件
            std::stringstream ss;
            ss << "<html><head><title>Download</title></head><body><h1>Download</h1><table>";
            // 需要根据备份文件变动的html部分
            for (int i = 0; i < arr.size(); ++i)
            {
                ss << "<tr><td><a href='" << arr[i]._url << "'>" << FileUtil(arr[i]._real_path).FileName() << "</a></td>"; // 备份文件下载链接
                ss << "<td align='right'>" << ctime(&arr[i]._mtime) << "</td>";                                            // 最后修改日期时间
                ss << "<td align='right'>" << arr[i]._size / 1024 << "k</td></tr>";                                        // 文件大小
            }
            ss << "</table></body></html>";
            res.body = ss.str(); // 填充有效载荷
            res.set_header("Content-Type", "text/html");
            res.status = 200;
        }

        // 生成当前文件的ETag
        // ETag:用于唯一标识一个资源,当客户端请求一个资源时,服务端会响应一个ETag,当客户端再次请求一个资源时,会比对ETag如果相同表示请求的资源没有被修改,此时服务端不用再真的传回一份资源,
        // 客户端会使用自己缓存的该份资源
        // 自定义ETag命名: 文件名-大小-最后修改时间
        static std::string GetETag(const BackupInfo &bi)
        {
            return FileUtil(bi._real_path).FileName() + std::to_string(bi._size) + std::to_string(bi._mtime);
        }

        // 下载处理函数
        static void Download(const httplib::Request &req, httplib::Response &res)
        {
            // 获取备份文件
            BackupInfo bi;
            dm.GetBackupInfoByURL(req.path, bi);
            // 判断该文件是否被压缩
            if (bi._pack_flag)
            {
                FileUtil backup_file(bi._real_path); // 打开要解压的备份文件
                backup_file.Unpack(bi._pack_path);   // 将压缩文件解压到打开的备份文件中
                FileUtil pack_file(bi._pack_path); //打开压缩文件
                pack_file.Remove();                // 删除压缩文件
                bi._pack_flag = false;             // 修改压缩标志
                dm.Update(bi);                     // 更新_hash
            }
            FileUtil backup_file(bi._real_path);
            std::string etag = GetETag(bi);
            bool flag = true;    // 用于判断此次请求是断点续传还是正常请求
            // 如果当前请求包含"If-Range",表示是断点续传
            if (req.has_header("If-Range"))
            {
                // 请求文件没有被修改过,进行区间请求(断点续传)
                if (req.get_header_value("If-Range") == etag)
                {
                    flag = false;
                }
            }

            //重新响应文件数据
            if (flag)
            {
                backup_file.Read(res.body); // 将备份文件数据写入响应
                res.set_header("ETag", etag);             // 设置ETag
                res.set_header("Accept-Ranges", "bytes"); // 设置断点续传
                res.set_header("Content-Type", "application/octet-stream"); //设置了才能下载
                res.status = 200;
            }
            //断点续传
            else
            {
                backup_file.Read(res.body); // 将备份文件数据写入响应
                res.set_header("ETag", etag);             // 设置ETag
                res.set_header("Accept-Ranges", "bytes"); // 设置断点续传
                res.set_header("Content-Type", "application/octet-stream");
                res.status = 206;
            }
        }

    private:
        httplib::Server _server;
    };
}