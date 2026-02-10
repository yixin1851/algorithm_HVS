project(AlgorithmLibrary VERSION 1.10.0.11)
# 使用的`CONFIGURE_DEPENDS` 选项(会自动检测文件变化, (CMake 3.12+ 支持)), 这样可以在添加/删除文件时自动Reload CMake Project.
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
# 添加一个选项来控制接口类型
option(BUILD_C_TYPE_INTERFACE "Build C Type Interface" OFF)

add_library(AlgorithmLibrary SHARED
        ${SRC}
)

target_sources(AlgorithmLibrary PRIVATE
        ${SRC}
)

# 没有额外依赖时，自动包含头文件
target_include_directories(AlgorithmLibrary PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# 指示在构建该库时导出符号
target_compile_definitions(AlgorithmLibrary PRIVATE API_EXPORTS)

# 根据选项设置接口类型和对应的头文件
if(BUILD_C_TYPE_INTERFACE)
    set(INTERFACE_TYPE "CTypeInterface")
    set(INTERFACE_HEADER "AlpMPAlgoCTypeInterface.h")
    target_compile_definitions(AlgorithmLibrary PRIVATE API_C_TYPE_INTERFACE)
else()
    set(INTERFACE_TYPE "CppTypeInterface")
    set(INTERFACE_HEADER "AlpMPAlgoInterface.h")
endif()

message(STATUS "\${CMAKE_SOURCE_DIR}:" ${CMAKE_SOURCE_DIR})
message(STATUS "\${CMAKE_CURRENT_LIST_DIR}:" ${CMAKE_CURRENT_LIST_DIR})
set(str_version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")
# 设置输出目录
set(OUTPUT_BASE_DIR "${CMAKE_SOURCE_DIR}/AlgorithmLibrary${str_version}/${INTERFACE_TYPE}/${CURRENT_PLATFORM}")
# 设置输出名称和版本信息
set(OUTPUT_DEBUG_DIR "${OUTPUT_BASE_DIR}/Debug/bin")
set(OUTPUT_RELEASE_DIR "${OUTPUT_BASE_DIR}/Release/bin")
set_target_properties(AlgorithmLibrary PROPERTIES
        OUTPUT_NAME "AlgorithmLibrary"
        VERSION "${FILE_VERSION}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DEBUG_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_RELEASE_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DEBUG_DIR}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_RELEASE_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DEBUG_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_RELEASE_DIR}"
)

# 定义头文件路径
#set(INCLUDE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/ReleaseAlgorithmLibrary/${CURRENT_PLATFORM}/include")
set(INCLUDE_OUTPUT_DIR "${OUTPUT_BASE_DIR}/include")
message(STATUS "\${INCLUDE_OUTPUT_DIR}:" ${INCLUDE_OUTPUT_DIR})
#[[# 定义要复制的头文件列表
set(PUBLIC_HEADERS
    include/AlpMPAlgoCTypeInterface.h
    include/AlpMPAlgoInterface.h
)
# 复制所有头文件
foreach(HEADER ${PUBLIC_HEADERS})
    add_custom_command(TARGET AlgorithmLibrary POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${INCLUDE_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}
            ${INCLUDE_OUTPUT_DIR}/
    )
endforeach()
]]
add_custom_command(TARGET AlgorithmLibrary POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INCLUDE_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${INTERFACE_HEADER}
        ${INCLUDE_OUTPUT_DIR}/
    COMMENT "Copying ${INTERFACE_HEADER} to ${INCLUDE_OUTPUT_DIR}"
)
add_version_info(AlgorithmLibrary)
#[[
# ============================================

# 如果需要覆盖某些变量，可以在调用前重新设置
# module3/CMakeLists.txt

# 覆盖产品名称
set(TARGET_PRODUCT_NAME "CustomName")

add_library(Module3 SHARED src/module3.cpp)
add_version_info(Module3)

# 或者为特定目标自定义版本信息
set(CUSTOM_DESCRIPTION "Special Module")
configure_file(
        ${CMAKE_SOURCE_DIR}/version.rc.in
        ${CMAKE_BINARY_DIR}/Module3_version.rc
        @ONLY
)
target_sources(Module3 PRIVATE ${CMAKE_BINARY_DIR}/Module3_version.rc)
]]
