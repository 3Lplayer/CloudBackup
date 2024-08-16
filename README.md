# CloudBackup
云备份服务端(Linux)/客户端(Windows)

服务端功能:
1. 接收备份文件
2. 对非热点文件(设置时间内没有对文件进行访问)进行压缩存储
3. 提供备份文件下载
4. 支持断点续传
客户端功能:
1. 上传文件
2. 下载文件

使用:
1. 在Server/cloud.conf配置文件中修改你的服务端的ip/端口
  "ip":"你的服务器ip",
  "port":你服务器开放的端口,
3. 在Client/CloudBackup/backup.hpp中照样修改你的服务端的ip/端口
  #define SERVER_IP "你的服务器ip"
  #define SERVER_PORT 你服务器开放的端口
4. make编译后执行可执行程序,运行服务端
5. 将需要备份的文件传到Client/CloudBackup/BackupDir即可完成文件备份
6. 在浏览器输入 你的服务器ip:你服务器开放的端口/ 即可访问下载界面
