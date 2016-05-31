# MyFileServer
#跨Windows/Linux平台文件传输服务
#作    者：周满满
#E-Mail  ：zhoumanman888@126.com
#版    本: 2016-5-31
#授    权：在使用时请保留此段文字说明
#
#
该项目是我在学习了邵发C++课程之后，自己做的一个综合项目。
本项目是在osapi类库、json库的基础上开发的。
其中osapi是邵发对线程、socket、锁、信号量等简单封装的跨平台（windows、linux）的类库，在项目开发的过程中
， 我也对其进行了一些小的修改、增加了一些功能（比如udp的广播设置）。
json版本我选用的是0.10.6，因为在windows xp 上我是用vs2008编写client， 在macbook用xcode编写server。我试了
vs2008只能用版本1.0.0以下的版本，xcode倒是支持最新版本。所以为了兼顾两者， 我选用了0.10.6版本。

#基于UDP的文件传输：
    (注意，如果用vs2008编译，文件编码尽量改成gb2312或者ascii码，否则有些地方可能不能够完全编译)
#1》需求分析：
    可以实现上传，下载，查看（即查看有哪些文件可以下载），删除功能。\n 支持断点传输,支持一次上传或下载多个文件。client端增加广播功能，即发现局域网内有哪些server。要求server\n在linux，或mac上， 客户端在windows上。
a.多文件同时上传，开了5个线程，每个线程单独开一个socket（需分配单独的端口）下载一个文件， 当下载完成后
， 需要自动开始剩余文件的下载， 所以这里需要一个消息队列， 用于提供给线程文件名字。（功能已经实现）\n
b.断点续传，这种情况主要是client在传输中突然掉线，这时候server端的该文件指针应该还没有关闭（如果已经关闭，则可以读取当前文件的大小，以app－and方式打开文件），此时client如果再次重现请求上传该文件，这时候server可以返回当前位置给client。所以这里需要client在开始的时候，先发送一个“开始”的请求包，确定文件从哪里开始传输。（该功能尚实现）。
#2》文件数据包格式分析：
    通信协议的整个net包划分2个部分：
    －－－－－－－－－－－－－－－－－－－－－－
    ｜head ｜ data                             ｜
    －－－－－－－－－－－－－－－－－－－－－－
    head 数据包的头， 长度不固定，用json格式表示， 以后如果移植到手机，可以改成protobuf格式，节省流量
    data是二进制部分， 头和尾由head确定
    这样的格式，在通信双方，接收到字符串后，可以直接用Json解析
    （后面如果与TCP/IP统一文件包格式， 前面可能会再增加4个字节表示head的长度，因为TCP/IP是流式协议，每次接
    收到的字节数量不固定）
    定义见filePackage.h

#  格式分解
#   发送时：
    {                  // head部分
       msgId      : 1, // 默认为0，如果不是多包同时发送， 则使用默认值即可
       code       : 0, // 发送时 正常：0 文件结束1； 接收时 正常：0  错误：－1
       msg        : "" // 发生错误时候，消息的返回
       cammnd     : "upload",
       filename   : "everthing.rar",
       filelength : 1111,
       currentpos : 0, 
       datalen    : 123456
    }
    0b0000000000...... // data部分

#  接收时：
    {                                 // head部分
       msgId      : 1, // 默认为0，如果不是多包同时发送， 则使用默认值即可
       code       : 1, // 发送时 正常：0 文件结束1； 接收时 正常：0  错误：－1
       msg        : "xxxxxxx" // 发生错误时候，消息的返回
       cammnd     : "upload",
       filename   : "everthing.rar",
       filelength : 0,
       currentpos : 0,
       datalen    : 0
    }


#3》对上传过程中的客户端和服务端进行封装， 方便使用
    ZFileClient 基本用法设计：
    ZFileClient* fs = new ZUDPFileClient(self_port); // 如果是TCP，则new ZTCPFileServer(self_port)
    fs->connect(peer_ip, peer_port); // 目的是与Tcp／ip 接口统一
    fs->regeditProgress(...); //注册进度显示函数
    fs->setFileList(...);
    fs->upload();
    
    ZFileServer 基本用法设计
    ZFileClient* fc = new ZUDPFileServer(self_port); // 如果是TCP，则new ZTCPFileClient(self_port)
    fc->setPath(...); // 设置路径
    fc->start();
   
