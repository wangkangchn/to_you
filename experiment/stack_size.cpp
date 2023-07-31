/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: stack_size.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 测试栈大小
时间	   	: 2023-07-20 21:44
***************************************************************/
#include <stdint.h>

int main()
{   
    int64_t size = ((int64_t)1 << 10) * 2048 ;
    char a[size] = {'\0'}; 
    return 0;
}
