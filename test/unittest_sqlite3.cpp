/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       unittest_sqlite3.cpp
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      学习 sqlite3 的使用
 * @date       2023-08-18 20:54
 **************************************************************/
#include <vector>
#include <stdio.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "logger/logger.hpp"
#include "command_flags.h"
#include "sqlite3.h"


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


static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{   
    /* oooo 是每一列都会调用一次 callback !!! */
    printf("callback\n");
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}



TEST(Learn, testSqlite3)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    char *sql;
    int rc = sqlite3_open("test.db", &db);

    if ( rc ) {
        ERROR("Can't open database:: {}", sqlite3_errmsg(db));
    }else{
        INFO("Opened database successfully");
    }

    /* Create SQL statement */
    sql = "CREATE TABLE COMPANY("  \
            "ID INT PRIMARY KEY     NOT NULL," \
            "NAME           TEXT    NOT NULL," \
            "AGE            INT     NOT NULL," \
            "ADDRESS        CHAR(50)," \
            "SALARY         REAL );";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        fprintf(stdout, "Table created successfully\n");
    }
    /* Create SQL statement */
    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
            "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
            "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
            "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
            "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
            "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
            "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
            "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        fprintf(stdout, "Records created successfully\n");
    }

    // 这个回调不是异步!!! 只不过是将数据的处理为我们分离了出来, 不是异步接口!!!
    /* Create SQL statement */
   sql = "SELECT * from COMPANY";

       const char* data = "Callback function called";
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
    if ( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else{
        fprintf(stdout, "Operation done successfully\n");
    }

    sqlite3_close(db);
}

// #include <string>
// #include <sstream>
// #include <fstream>
// bool read_file_to_string(const std::string& file_name, std::string& data)
// {
//     std::ifstream t(file_name);
//     if (t.fail()) 
//         return false;
//     std::stringstream buffer;
//     buffer << t.rdbuf();
//     data = buffer.str();
//     return true;
// }

// std::string read_file_to_string(const std::string& file_name) 
// {
//     std::string content;
//     if (read_file_to_string(file_name, content)) {
//         return content;
//     } else {
//         return "";
//     }
// }

// bool write_string_to_file(const std::string& data, const std::string& file_name) {
//     std::ofstream out(file_name);
//     if (out.fail()) 
//         return false; 
//     out << data;
//     out.close();
//     return true;
// }


// #include <chrono>

// #define ELAPSED(name) \
//     for (auto _start_ = std::chrono::high_resolution_clock::now(), _end_ = _start_;\
//         _start_ == _end_;\
//         _end_ = std::chrono::high_resolution_clock::now(),\
//         INFO("{}: {} ms", name, std::chrono::duration_cast<std::chrono::milliseconds>(_end_ - _start_).count()))

// TEST(TestSQLite, testSavePic)
// {
//     char* cmdCreatBlobTable = "create table SqliteBlobTest (id integer , pic blob)";  //首先创建一个可插入blob类型的表 。
//     sqlite3* db = NULL;
//     char * errorMessage = NULL;
//     int iResult = sqlite3_open("SqliteTest.db", &db);
//     sqlite3_exec(db,"drop table if exists SqliteBlobTest",0,0,0);  

//     iResult = sqlite3_exec(db, cmdCreatBlobTable, NULL, NULL, &errorMessage);
//     if (SQLITE_OK != iResult)
//     {
//         cout<<"创建表SqliteBlobTest失败"<<endl;
//     }

//     sqlite3_stmt *stmt;                                            //声明
//     const char* sql = "insert into SqliteBlobTest values(1,?)";  
//     char* pPicData = "this is a pic data" ;
    

//     sqlite3_prepare(db,sql,strlen(sql),&stmt,0);                   //完成对sql语句的解析
//     {  
//         std::string img;
//         ELAPSED("read_file_to_string") {
//             img = read_file_to_string("/home/wkangk/test_data/111.png");
//         }
//         INFO("img.size(): {}", img.size());
//         sqlite3_bind_blob(stmt, 1, img.data(), img.size(), NULL);//1代表第一个？
//         sqlite3_step(stmt);                                        //将数据写入数据库中
//     } 

//     ELAPSED("sqlite3_prepare") {
//     sqlite3_prepare(db, "select * from SqliteBlobTest", -1, &stmt, 0);
//     int result = sqlite3_step(stmt);
//     int id = 0,len = 0; 
//     if (result == SQLITE_ROW)                                     //查询成功返回的是SQLITE_ROW
//     {
//         // cout<<"read success from sqlite"<<endl;
//         id = sqlite3_column_int(stmt, 0);                         //从0开始计算，id为0，picdata 为1；
//         const void * pReadPicData = sqlite3_column_blob(stmt, 1); //读取数据，返回一个指针
//         len = sqlite3_column_bytes(stmt, 1);                      //返回数据大小

//         std::string img(static_cast<const char*>(pReadPicData), len);
//         // INFO("img.size(): {}", img.size());
//         // EXPECT_TRUE(write_string_to_file(img, "img.png"));
//     }
//     else
//     {
//         // cout<<"read fail from sqlite"<<endl;
//     }
//     }
//     sqlite3_finalize(stmt);                                      //把刚才分配的内容析构掉

// }


TEST(testSqlite3, tableExists)
{
    sqlite3  *db = 0;
    int rc = sqlite3_open("my.db", &db);

    if ( rc ) {
        ERROR("Can't open database:: {}", sqlite3_errmsg(db));
    }else{
        INFO("Opened database successfully");
    }

    /* Create SQL statement */
    sql = "create table table_create_time("  \
            "name INT PRIMARY KEY     NOT NULL," \
            "time           TEXT    NOT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        fprintf(stdout, "Table created successfully\n");
    }

}

int main(int argc, char* argv[])
{   
    gflags::ParseCommandLineFlags(&argc, &argv, true); 
    
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}