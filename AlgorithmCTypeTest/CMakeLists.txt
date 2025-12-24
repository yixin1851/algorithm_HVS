cmake_minimum_required(VERSION 3.21)
project(AlgorithmCTypeTest VERSION 1.0.0 LANGUAGES CXX)
# 强制固定在VS2012环境之下
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用的`CONFIGURE_DEPENDS`选项, 自动检测文件变化
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME}
        ${SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        AlgorithmLibrary
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_lib/ReleaseAlgorithmLibrary/CTypeInterface/${CURRENT_PLATFORM}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_lib/CTypeInterface/${CURRENT_PLATFORM}/bin
)

# Windows特定设置
if (WIN32)
    # 设置为Windows应用程序
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE OFF    # ON为不显示控制台
    )

    # 编译选项
    target_compile_options(${PROJECT_NAME} PRIVATE
            /MP  # 多处理器编译
    )

    # Debug配置
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:_DEBUG>
    )

    # Release配置优化
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/O2>
    )
endif ()
target_compile_definitions(${PROJECT_NAME} PRIVATE API_C_TYPE_INTERFACE)
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION "${FILE_VERSION}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
)
add_version_info(${PROJECT_NAME})