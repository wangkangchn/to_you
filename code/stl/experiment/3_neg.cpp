/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: 3_neg.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 测试 -1 转为无符号是不是最大值
时间	   	: 2023-07-23 21:19
***************************************************************/
#include <iostream>//输入输出
#include <stdint.h>//atoi函数
#include <limits.h>

const int N = 100010;

void ShortDivOutputBin(uint32_t input)
{
    uint8_t temp[33] = {0};  
    int i = 0;
    printf("短除法得到的二进制为：");
    while(input) {
        temp[i] = input % 2;    //取余数存放到数组中，此为得到的二进制数
        input = (uint32_t)input / 2;  //短除，while中判断是否除尽
        i++;  //存储了一个二进制数，自加存储下一个
    }
    for(i--; i>=0; i--)  //由于最后一次input为0无效，i还是自加了，因此最后一次自加的值是无用的，所以先自减，然后将余数从后往前读取
    {
        printf("%d",temp[i]);
    }
    printf("\r\n");
}

int main() 
{
    ShortDivOutputBin(char(-1));

    std::cout << "UINT_MAX: " << UINT_MAX << std::endl;
    std::cout << "(uint32_t)(-1): " << (uint32_t)(-1) << std::endl;

    int a[3] = {1, 2, 3};
    printf("a: %p\n", a);
    printf("a + 1: %p\n", a + 1);
    int* b = a + 1;
    printf("b - a: %d\n", b - a);
    return 0;
}
