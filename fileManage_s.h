//
//  fileManage_s.h
//  TCP_Server
//
//  Created by 周满满 on 6/5/16.
//  Copyright © 2016 周满满. All rights reserved.
//
// 负责服务器文件业务方面的工作，主要是为了把文件的业务部分跟服务器逻辑分开
#ifndef fileManage_h
#define fileManage_h
#include <stdio.h>
#include <string>

class ZFileManage{
    typedef std::map<std::string, FILE*> MapName_File_t;
    
    std::string m_rootUpload;
    MapName_File_t m_mapNameFile;
    ZFileManage(){}
    ~ZFileManage(){}
public:
    static ZFileManage* getInstance()
    {
        static ZFileManage* pFileManage = new ZFileManage();
        return pFileManage;
    }
    static void destroy()
    {
        ZFileManage* pFileManage = ZFileManage::getInstance();
        delete pFileManage;
    }
    
    int setRootDirOfUpload(std::string path)
    {
        // 可以做些检查，如果路径不存在，则创建 或提示
        m_rootUpload = path;
        
        return 0;
    }
    
    int endWrite(std::string filename)
    {
        MapName_File_t::iterator iFind = m_mapNameFile.find(filename);
        FILE* fp = iFind->second;
        fclose(fp);
        m_mapNameFile.erase(iFind);
        return 0;
    }
    int write(std::string filename, unsigned long long currentpos, const unsigned char* data, unsigned long long datalength, std::string& errStr)
    {
        int iSucceed = 0;
        
        std::string strTemp;
        if (currentpos == 0)
        {
            printf("craeted file: %s\n", filename.c_str());
            strTemp = m_rootUpload + filename;
            
            FILE* fp = fopen(strTemp.c_str(), "wb");
            if (fp == NULL)
            {
                extern int errno;
                iSucceed = -1;
                errStr = errno;
                errStr += " ";
                errStr += strerror(errno);
            }
            else
            {
                m_mapNameFile[filename] = fp;
                fwrite(data, 1, datalength, fp);
            }
        }
        else
        {
            FILE* fp = m_mapNameFile[filename];
            fwrite(data, 1, datalength, fp);
        }
        
        return iSucceed;
    }
};
#endif /* fileManage_h */
