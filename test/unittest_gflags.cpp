/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       unittest_gflags.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      测试 gflags 学习
 * @date       2023-08-02 21:13
 **************************************************************/
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logger/logger.hpp"
#include "command_flags.h"


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


using namespace gflags;

// TEST(Learn, testFlags)
// {
//     INFO("123");
//     INFO("VersionString: {}", gflags::VersionString());
//     gflags::SetUsageMessage("13 要这么使用");
//     INFO("ProgramUsage(): {}", gflags::ProgramUsage());

//     std::string output;
//     EXPECT_TRUE(GetCommandLineOption("batch", &output));
//     INFO("output: {}", output);
//     /* 
//     struct CommandLineFlagInfo {
//   std::string name;            // the name of the flag
//   std::string type;            // the type of the flag: int32, etc
//   std::string description;     // the "help text" associated with the flag
//   std::string current_value;   // the current value, as a string
//   std::string default_value;   // the default value, as a string
//   std::string filename;        // 'cleaned' version of filename holding the flag
//   bool has_validator_fn;       // true if RegisterFlagValidator called on this flag
//   bool is_default;             // true if the flag has the default value and
//                                // has not been set explicitly from the cmdline
//                                // or via SetCommandLineOption
//   const void* flag_ptr;        // pointer to the flag's current value (i.e. FLAGS_foo) 这里就是我们定义的具体变量
// };
//      */

//     /* 可以可以 */
//     CommandLineFlagInfo info;
//     EXPECT_TRUE(GetCommandLineFlagInfo("batch", &info));
//     INFO("name: {}", info.name);
//     INFO("type: {}", info.type);
//     INFO("description: {}", info.description);
//     INFO("current_value: {}", info.current_value);
//     INFO("default_value: {}", info.default_value);
//     INFO("filename: {}", info.filename);
//     INFO("has_validator_fn: {}", info.has_validator_fn);
//     INFO("is_default: {}", info.is_default);
//     INFO("CommandlineFlagsIntoString(): {}", CommandlineFlagsIntoString());

//     INFO("Int32FromEnv(test, 123);: {}", Int32FromEnv("test", 123));
//     INFO("StringFromEnv(PATH, PATH);: {}", StringFromEnv("PATH", "PATH"));


//     EXPECT_TRUE(GetCommandLineOption("hahah", &output));
//     INFO("output: {}", output);

// }

int main(int argc, char* argv[])
{   
    SetVersionString("v1.1");   /* 要在解析之前用 */
    gflags::ParseCommandLineFlags(&argc, &argv, true); 
    
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}