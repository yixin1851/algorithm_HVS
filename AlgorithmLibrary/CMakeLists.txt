# 使用的`CONFIGURE_DEPENDS` 选项(会自动检测文件变化, (CMake 3.12+ 支持)), 这样可以在添加/删除文件时自动Reload CMake Project.
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
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
target_compile_definitions(AlgorithmLibrary PRIVATE API_C_TYPE_INTERFACE)

# 可选：设置输出名称和版本信息
set_target_properties(AlgorithmLibrary PROPERTIES
        OUTPUT_NAME "AlgorithmLibrary"
        VERSION "${FILE_VERSION}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CURRENT_PLATFORM}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CURRENT_PLATFORM}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CURRENT_PLATFORM}/bin"
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
