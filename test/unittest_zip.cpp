/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       unittest_zip.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      学习 zip 的使用
 * @date       2023-08-18 20:14
 **************************************************************/
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logger/logger.hpp"
#include "command_flags.h"
#include "zip/zip.h"


using std::cout;
using std::endl;
using std::string;

using namespace testing;
using namespace tools;


class MyEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {   
        MyLogger::setup(FLAGS_log_fn, string_to_log_level(FLAGS_log_level));
        INFO("测试开始");
    }

    virtual void TearDown()
    {
        INFO( "测试结束." );
    }
};



TEST(Learn, testZip)
{
    struct zip_t *zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {   
        /* 带上路径就是压缩进某个目录了 */
        zip_entry_open(zip, "foo/README.md");
        zip_entry_fwrite(zip, "/home/wkangk/to_you/README.md");
        zip_entry_close(zip);

        zip_entry_open(zip, "foo/CMakeLists.txt");
        zip_entry_fwrite(zip, "/home/wkangk/to_you/CMakeLists.txt");
        zip_entry_close(zip);
    }
    zip_close(zip);
}

int main(int argc, char* argv[])
{   
    gflags::ParseCommandLineFlags(&argc, &argv, true); 
    
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}