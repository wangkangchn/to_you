set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

find_package(GTest REQUIRED)
find_package(gflags  REQUIRED)

set(zip_DIR "/home/wkangk/to_you/thirdparty/zip/lib/cmake/zip")
find_package(zip)     # 搜索第三方库

set(THIRDPARTY_DIR "${PROJECT_SOURCE_DIR}/thirdparty")
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}
                        "${THIRDPARTY_DIR}/zip/include"
                        "${THIRDPARTY_DIR}/sqlite3/include"
                        "${THIRDPARTY_DIR}/nlohmann/include"
                )

link_directories(       "${THIRDPARTY_DIR}/zip/lib"
                        "${THIRDPARTY_DIR}/sqlite3/lib"
)

set(Thirdparty_Libs     pthread
                        GTest::GTest GTest::Main
                        gflags gmock
                        zip
                        sqlite3
                    )
                    
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(EXTRA_SRCS "${WKANGK_TOOLS_DIR}/logger/logger.cpp")

macro(_unittest unittest_name)
    target_link_libraries( ${unittest_name}  ${Thirdparty_DIRECTORIES} ${Thirdparty_Libs} )
    add_test(  NAME  ${unittest_name}  COMMAND    ${CMAKE_BINARY_DIR}/test/${unittest_name}  )
endmacro(_unittest)

macro(add_unittest unittest_name)
    add_executable( ${unittest_name} "${unittest_name}.cpp"  ${EXTRA_SRCS})
    _unittest(${unittest_name})
endmacro(add_unittest)

macro(add_unittest_multi unittest_name src)
    add_executable( ${unittest_name} "${unittest_name}.cpp" ${src}   ${EXTRA_SRCS})
    _unittest(${unittest_name})
endmacro(add_unittest_multi)



add_unittest(unittest_gflags)
add_unittest(unittest_zip)
add_unittest(unittest_sqlite3)
add_unittest(unittest_json)