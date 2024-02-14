# ChatRoom
一个多线程网络聊天室

打开项目后，选择TcpClient/ ClientMain.cpp 将main() 中的client.ConnectServer("192.168.0.106", 9999); 里的 ip地址改成本地的ip地址

先运行TcpServer启动服务端，然后运行TcpClient 启动客户端。 可以启动多个TcpClient。

show  展示当前存在的房间

create  创建新房间

join房间id  加入一个房间，join后直接跟房间id

talk  开始聊天

leave  离开房间

exit  退出程序
