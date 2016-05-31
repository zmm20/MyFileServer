
#ifndef _AF_BYTEBUFFER_H
#define _AF_BYTEBUFFER_H

/* AfByteBuffer
   字节编码器。

   作者：邵发
   版本：2016-01-31
   最新版本请在官网 http://afanihao.cn 上获取
*/

#include <stdio.h>
#include <string.h>

#include <string>
using std::string;

class AfByteBuffer
{
public:
	AfByteBuffer(int size)
	{
		m_buffer = new char[size];
		m_offset = 0;
		m_length = size;
		m_owner = true;

		m_start = (unsigned char*)m_buffer + m_offset;
		m_written = 0; // 写计数
		m_read = 0; // 读计数
	}
	AfByteBuffer(char* buf, int off, int length)
	{
		m_buffer = buf;
		m_offset = off;
		m_length = length;
		m_owner = false;

		m_start = (unsigned char*)m_buffer + m_offset;
		m_written = 0; // 写计数
		m_read = 0; // 读计数
	}

	~AfByteBuffer()
	{
		if(m_owner && m_buffer)
		{
			delete [] m_buffer;
		}
	}

	unsigned char* start()
	{
		return m_start;
	}
	int sizeWritten()
	{
		return m_written;
	}
	int sizeRead() const
	{
		return m_read;
	}

	////////////////////////////////////////
	void putUint8(unsigned char val)
	{
		unsigned char* p = m_start + m_written;
		p[0] = val;
		m_written += 1;
	}
	void putUint16(unsigned short val)
	{
		unsigned char* p = m_start + m_written;
		p[0] = val>>8;
		p[1] = val;
		m_written += 2;
	}
	void putUint32(unsigned int val)
	{
		unsigned char* p = m_start + m_written;
		p[0] = val>>24;
		p[1] = val>>16;
		p[2] = val>>8;
		p[3] = val;
		m_written += 4;
	}
	void putUint64(unsigned __int64 val)//long long
	{
		unsigned char* p = m_start + m_written;
		p[0] = val>>56;
		p[1] = val>>48;
		p[2] = val>>40;
		p[3] = val>>32;
		p[4] = val>>24;
		p[5] = val>>16;
		p[6] = val>>8;
		p[7] = val;
		m_written += 8;
	}
	void putString(const string& val)
	{
		unsigned short length = val.length();
		putUint16(length);

		unsigned char* p = m_start + m_written;
		memcpy(p, val.c_str(), length);
		m_written += length;
	}

	void putBytes(const char* data, int length)
	{
		putUint16(length);

		unsigned char* p = m_start + m_written;
		memcpy(p, data, length);
		m_written += length;
	}

	/////////////////////////////////////////
	unsigned char getUint8()
	{
		unsigned char* p = m_start + m_read;
		unsigned char result = p[0];
		m_read += 1;
		return result;
	}
	unsigned short getUint16()
	{
		unsigned char* p = m_start + m_read;
		unsigned short result = p[0];
		result = (result<<8) + p[1];
		m_read += 2;
		return result;
	}
	unsigned int getUint32()
	{
		unsigned char* p = m_start + m_read;
		unsigned char result = p[0];
		result = (result<<8) + p[1];
		result = (result<<8) + p[2];
		result = (result<<8) + p[3];
		m_read += 4;
		return result;
	}
	unsigned __int64 getUint64() // long long
	{
		unsigned char* p = m_start + m_read;
		unsigned char result = p[0];
		result = (result<<8) + p[1];
		result = (result<<8) + p[2];
		result = (result<<8) + p[3];
		result = (result<<8) + p[4];
		result = (result<<8) + p[5];
		result = (result<<8) + p[6];
		result = (result<<8) + p[7];
		m_read += 8;
		return result;
	}
	string getString()
	{
		unsigned short length = getUint16();

		unsigned char* p = m_start + m_read;
		string result;
		result.append((char*)p, length);
		m_read+= length;

		return result;
	}

	// string类也可以放“不以0结束的无规则数据”
	int getBytes(char* buf, int maxsize)
	{
		unsigned short length = getUint16();

		unsigned char* p = m_start + m_read;
		memcpy(buf, p, length);
		return length;
	}

private:
	char* m_buffer;
	int   m_offset;
	int   m_length;
	bool  m_owner;

	unsigned char* m_start;
	int   m_written; // 已经写入的数据个数
	int   m_read;    // 已经读取的数据个数
};




#endif

