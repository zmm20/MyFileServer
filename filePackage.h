//
// filePackage.h
//  UDP_HostB
//
//  Created by 周满满 on 5/25/16.
//  Copyright © 2016 周满满. All rights reserved.
//
#ifndef FILE_PACKAGE_H
#define FILE_PACKAGE_H

#include "json/json.h" // '/' 斜杠各平台通用
#include <string>
#include <assert.h>

// 格式分解
//  发送时：
//  {                  // head部分
//     msgId      : 1, // 默认为0，如果不是多包同时发送， 则使用默认值即可
//     code       : 0, // 发送时 正常：0 文件结束1； 接收时 正常：0  错误：－1
//     msg        : "" // 发生错误时候，消息的返回
//     cammnd     : "upload",
//     filename   : "everthing.rar",
//     filelength : 1111,
//     currentpos : 0,
//     datalen    : 123456
//  }
//  0b0000000000...... // data部分

//  接收时：
// {                                 // head部分
//     msgId      : 1, // 默认为0，如果不是多包同时发送， 则使用默认值即可
//     code       : 1, // 发送时 正常：0 文件结束1； 接收时 正常：0  错误：－1
//     msg        : "xxxxxxx" // 发生错误时候，消息的返回
//     cammnd     : "upload",
//     filename   : "everthing.rar",
//     filelength : 0,
//     currentpos : 0,
//     datalen    : 0
//  }

class ZFilePackage{
    int m_size;
    std::string m_headBuffer;//
    unsigned char* m_totalBuffer;
public:
    enum Comand
    {
        UPLOAD = 0X01,
        UPDATE,
        REMOVE
    };
public:
    // head部分
    int msgId; // 默认值是0， 如果有多个包同时发送， 则需要用msgId来区分不同的包
    int code;
    std::string msg;
    Comand cmd;
    std::string filename;
    Json::UInt64 filelength;
    Json::UInt64 currentpos;
    int datalength;
    
    // data部分
    unsigned char* data;
public:
    ZFilePackage() : m_size(0), msgId(0), m_totalBuffer(NULL), data(NULL){}
    ~ZFilePackage()
    {
        if (m_totalBuffer)
        {
            delete[] m_totalBuffer;
            m_totalBuffer = NULL;
        }
    }
    
    void Serialize()
    {
        Json::Value item;
        item["msgId"]	   = msgId;
        item["code"]	   = code;
        item["msg"]		   = msg.c_str();
        item["command"]    = (int)cmd;
        item["currentpos"] = currentpos;
        item["filename"]   = filename.c_str();
        item["filelength"] = filelength;
        
        m_headBuffer =  item.toStyledString();
        m_size = m_headBuffer.size() + datalength;
        
        if (m_totalBuffer)
        {
            delete[] m_totalBuffer;
        }
        m_totalBuffer = new unsigned char[m_size];
        memset(m_totalBuffer, 0, m_size);
        memcpy(m_totalBuffer, m_headBuffer.c_str(), m_headBuffer.size());
        
        if (data != NULL && datalength > 0)
            memcpy(m_totalBuffer + m_headBuffer.size(), data, datalength);
    }
    
    void UnSerialize(unsigned char* buf, int buflen)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse((char*)buf, root);
        
        msgId      =  root["msgId"].asInt();
        code	   =  root["code"].asInt();
        msg 	   =  root["msg"].asString();
        cmd = (Comand)root["command"].asInt();
        currentpos =  root["currentpos"].asUInt64();
        filelength =  root["filelength"].asUInt64();
        filename   =  root["filename"].asString();
        
        m_headBuffer = root.toStyledString();
        datalength = buflen - m_headBuffer.size();
        m_size = buflen;
        
        if (datalength > 0)
            data = buf + m_headBuffer.size();
    }
    unsigned char* GetBuffer()
    {
        return m_totalBuffer;
    }
    
    int GetSize()
    {
        return m_size;
    }
    
};

#endif