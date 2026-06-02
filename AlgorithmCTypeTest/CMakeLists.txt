cmake_minimum_required(VERSION 3.21)
project(AlgorithmCTypeTest VERSION 1.0.0 LANGUAGES CXX)
set(ALGO_VERSION "AlgorithmLibrary1.10.0.13")
# 强制固定在VS2012环境之下
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CL_64)
    set(CURRENT_PLATFORM "x64")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
else (CMAKE_CL_64)
    set(CURRENT_PLATFORM "x86")
    message(STATUS "Current Platform is ${CURRENT_PLATFORM}")
endif (CMAKE_CL_64)

# 使用的`CONFIGURE_DEPENDS`选项, 自动检测文件变化
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE THIRD_PARTY_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/third_lib/*.h")

add_executable(${PROJECT_NAME}
        ${THIRD_PARTY_HEADERS}
        ${HEADERS}
        ${SRC}
)
set(THIRD_PARTY_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_lib/${ALGO_VERSION}/CTypeInterface/${CURRENT_PLATFORM}")
# 将头文件添加到target
target_sources(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        AlgorithmLibrary$<$<CONFIG:Debug>:d>
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_lib/${ALGO_VERSION}/CTypeInterface/${CURRENT_PLATFORM}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/DVS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/APS
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_lib/${ALGO_VERSION}/CTypeInterface/${CURRENT_PLATFORM}/Release/bin
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
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/BUILD/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/BUILD/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/BUILD/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_BIN_DIR}/$<CONFIG>/bin/AlgorithmLibrary$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_LIB_DIR}/$<CONFIG>/bin/AlgorithmLibrary$<CONFIG:Debug>:d>.lib"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_LIB_DIR}/$<CONFIG>/bin/AlgorithmLibrary$<CONFIG:Debug>:d>.exp"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying third-party library files"
)
add_version_info(${PROJECT_NAME})