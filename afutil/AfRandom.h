#ifndef _AF_RANDOM_H
#define _AF_RANDOM_H

/* AfRandom
	随机打乱

	作者 邵发
	官网 http://afanihao.cn
	最新版本请从官网下载!

	用法:
	srand(time(NULL));

	int arr[9];
	AfRandom::disorder(arr, 9);
	for(int i=0; i<9; i++)
	{
	    printf("%d ", arr[i]);
	}


*/

#include <stdio.h>
#include <stdlib.h>

class AfRandom
{
public:
	static  void disorder (int  arr[],  int n)
	{
		// 初始化为-1
		char* flags = (char*)malloc(n);
		for(int i=0; i<n; i++)
			flags[i] = 0;

		for(int i=0; i<n; i++)
		{
			// 取一个随机数
			int remain = n - i;
			int sel = rand() % remain;

			int count = 0;
			for(int k=0; k<n; k++)
			{
				// 已经分配过的不算
				if(flags[k]) continue;

				if(count == sel)
				{
					arr[i] = k;
					flags[k] = 1;
					break;
				}
				count ++;			
			}
		}
		// 释放内存
		free ( flags);
	}
};



#endif 

