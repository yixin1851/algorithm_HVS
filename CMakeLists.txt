cmake_minimum_required(VERSION 3.21)

project(AlgorithmLibrary LANGUAGES CXX C)
if (${MSVC})
    message(STATUS "Generator: MSVC")
    message(STATUS "  \${MSVC_TOOLSET_VERSION} = " ${MSVC_TOOLSET_VERSION})
    message(STATUS "  \${MSVC_VERSION} = " ${MSVC_VERSION})
else (${UNIX})
    message(STATUS "UNIX = " ${UNIX})
endif ()
if (${WIN32})
    message(STATUS "WIN32 = " ${WIN32})
endif ()
message(STATUS "CMAKE_SYSTEM_NAME = " ${CMAKE_SYSTEM_NAME})
# 设定 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CL_64)
    set(CURRENT_PLATFORM "x64")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
else (CMAKE_CL_64)
    set(CURRENT_PLATFORM "x86")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
endif (CMAKE_CL_64)

# 将cmake模块路径添加到搜索路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# 包含版本信息模块（只需包含一次）
include(VersionInfo)

## 打印调试信息，确认变量已设置
#message(STATUS "=== Version Info Debug ===")
#message(STATUS "    FILE_VERSION: ${FILE_VERSION}")
#message(STATUS "    FILE_VERSION_COMMA: ${FILE_VERSION_COMMA}")
#message(STATUS "    PRODUCT_VERSION: ${PRODUCT_VERSION}")
#message(STATUS "    COMPANY_NAME: ${COMPANY_NAME}")
#message(STATUS "========================")

#[[
# 获取当前日期作为文件版本
string(TIMESTAMP FILE_VERSION "%Y.%m.%d")
string(TIMESTAMP FILE_VERSION_COMMA "%Y,%m,%d,0")

# 获取Git信息作为产品版本
execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
)

execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
)

# 如果Git命令失败，设置默认值
if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

if(NOT GIT_BRANCH)
    set(GIT_BRANCH "unknown")
endif()

set(PRODUCT_VERSION "${GIT_HASH}@${GIT_BRANCH}")

set(COMPANY_NAME "AlpsenTek Company")

# 配置版本资源文件
configure_file(
        ${CMAKE_SOURCE_DIR}/version.rc.in
        ${CMAKE_BINARY_DIR}/version.rc
        @ONLY
)
# 为多个目标添加版本信息的辅助函数
function(add_version_info TARGET_NAME)
    # 动态设置产品名称为目标名称
    set(TARGET_PRODUCT_NAME ${TARGET_NAME})

    # 创建目标特定的版本资源文件
    configure_file(
            ${CMAKE_SOURCE_DIR}/version.rc.in
            ${CMAKE_BINARY_DIR}/${TARGET_NAME}_version.rc
            @ONLY
    )

    # 将版本资源添加到目标
    target_sources(${TARGET_NAME} PRIVATE
            ${CMAKE_BINARY_DIR}/${TARGET_NAME}_version.rc
    )
endfunction()
# 使用示例：
# add_library(AnotherLib SHARED src/another.cpp)
# add_version_info(AnotherLib)]]
# 子目录
add_subdirectory(AlgorithmLibrary)
add_subdirectory(AlgorithmTest)
add_subdirectory(AlgorithmCTypeTest)


## 子目录
#add_subdirectory(AlgorithmDemo)
#
## 子目录
#add_subdirectory(AlgorithmTest)
#
## 查看生成的RC文件内容（用于调试）
#add_custom_target(show_rc_file
#        COMMAND ${CMAKE_COMMAND} -E echo "Generated RC file content:"
#        COMMAND ${CMAKE_COMMAND} -E cat "${CMAKE_BINARY_DIR}/version_info/AlgorithmLibrary_version.rc"
#        DEPENDS AlgorithmLibrary
#        VERBATIM
#)