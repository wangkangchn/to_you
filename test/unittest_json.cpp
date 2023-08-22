/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       unittest_zip.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      学习 zip 的使用
 * @date       2023-08-18 20:14
 **************************************************************/
#include <vector>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logger/logger.hpp"
#include "command_flags.h"
#include "nlohmann/json.hpp"


using std::cout;
using std::endl;
using std::string;

using namespace testing;
using namespace tools;
using nlohmann::json;


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


// example enum type declaration
enum TaskState {
    TS_STOPPED,
    TS_RUNNING,
    TS_COMPLETED,
    TS_INVALID=-1,
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM( TaskState, {
    {TS_INVALID, nullptr},
    {TS_STOPPED, "stopped"},
    {TS_RUNNING, "running"},
    {TS_COMPLETED, "completed"},
})


TEST(Learn, testJson)
{
    // std::string path{"../../test/test_file/test_json.json"};
    // std::ifstream fin(path);
    json j;
    // fin >> j;

    INFO("{}", j.dump() );

    std::vector<int> aaa{1,2,3,4,5};
    j["aaa"] = aaa;

    j["aaa"].emplace_back(213);

    INFO("{}", j["aaa"].dump());

}

int main(int argc, char* argv[])
{   
    gflags::ParseCommandLineFlags(&argc, &argv, true); 
    
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}