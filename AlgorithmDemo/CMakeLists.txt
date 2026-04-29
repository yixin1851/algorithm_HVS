cmake_minimum_required(VERSION 3.21)

project(AlgorithmDemo VERSION 1.0.0 LANGUAGES CXX)

set(ALGO_VERSION "AlgorithmLibrary2.0.0.0")
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CL_64)
    set(CURRENT_PLATFORM "x64")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
else (CMAKE_CL_64)
    set(CURRENT_PLATFORM "x86")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
endif (CMAKE_CL_64)

set(THIRD_PARTY_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_lib/${ALGO_VERSION}/CppTypeInterface/${CURRENT_PLATFORM}")
message(STATUS "THIRD_PARTY_BIN_DIR: ${THIRD_PARTY_BIN_DIR}")
# 使用的`CONFIGURE_DEPENDS`选项, 自动检测文件变化
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE THIRD_PARTY_HEADERS CONFIGURE_DEPENDS "${THIRD_PARTY_BIN_DIR}/include/*.h")
message(STATUS "SRC: ${SRC}")
message(STATUS "HEADERS: ${HEADERS}")
message(STATUS "THIRD_PARTY_HEADERS: ${THIRD_PARTY_HEADERS}")

# 创建可执行文件
add_executable(${PROJECT_NAME}
        ${THIRD_PARTY_HEADERS}
        ${HEADERS}
        ${SRC}
)

# Windows 特定设置
if (WIN32)
    # 设置 Unicode 字符集
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

    # 设置子系统为控制台
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE FALSE
    )
endif ()

# Debug 配置的预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        _CONSOLE
)

# 编译选项
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            /W3  # Warning Level 3
            /sdl # SDL checks
            $<$<CONFIG:Release>:/O2 /Oi>  # 优化选项
    )

    # 链接选项
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/OPT:REF /OPT:ICF>
    )
endif ()

# 链接库路径(根据配置)
# 注意: 你需要根据实际情况调整库的路径
if (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${THIRD_PARTY_BIN_DIR}/include
    )
    # 如果需要指定库文件的路径,可以使用:
    target_link_directories(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:${THIRD_PARTY_BIN_DIR}/Debug/bin>
            $<$<CONFIG:Release>:${THIRD_PARTY_BIN_DIR}/Release/bin>
    )

    # Debug 配置链接 AlgorithmLibraryd.lib
    target_link_libraries(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:AlgorithmLibraryd>
            $<$<CONFIG:Release>:AlgorithmLibrary>
    )
endif ()

# 生成调试信息
set_target_properties(${PROJECT_NAME} PROPERTIES
        DEBUG_POSTFIX "d"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION "${FILE_VERSION}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_BIN_DIR}/Release/bin/AlgorithmLibrary.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${THIRD_PARTY_LIB_DIR}/AlgorithmLibrary.lib"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${THIRD_PARTY_LIB_DIR}/AlgorithmLibrary.exp"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying third-party library files"
)
add_version_info(${PROJECT_NAME})