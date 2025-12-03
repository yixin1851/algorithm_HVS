# cmake/VersionInfo.cmake
# 这是一个可复用的CMake模块，用于为目标添加版本信息

# 为目标添加版本信息的函数
function(add_version_info TARGET_NAME)
    # 检查是否为Windows平台
    if(NOT WIN32)
        message(WARNING "add_version_info is only supported on Windows")
        return()
    endif()

    # 检查目标是否存在
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
        return()
    endif()

    # 在函数内部重新获取所有版本信息
    # 获取当前日期作为文件版本
    string(TIMESTAMP YEAR "%Y")
    string(TIMESTAMP MONTH "%m")
    string(TIMESTAMP DAY "%d")
    string(TIMESTAMP HOUR "%H")

    # 移除月份和日期前导零（如果有）
    string(REGEX REPLACE "^0" "" MONTH_NO_ZERO "${MONTH}")
    string(REGEX REPLACE "^0" "" DAY_NO_ZERO "${DAY}")

    # 生成显示用的版本号（带点号）
    set(FILE_VERSION "${YEAR}.${MONTH_NO_ZERO}.${DAY_NO_ZERO}")

    # 生成RC文件用的版本号（用逗号分隔，必须是纯数字）
    set(FILE_VERSION_COMMA "${YEAR},${MONTH},${DAY},${HOUR}")

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
    set(TARGET_PRODUCT_NAME ${TARGET_NAME})

    # 打印调试信息
    message(STATUS "Generating version info for ${TARGET_NAME}:")
    message(STATUS "  FILE_VERSION = ${FILE_VERSION}")
    message(STATUS "  FILE_VERSION_COMMA = ${FILE_VERSION_COMMA}")
    message(STATUS "  PRODUCT_VERSION = ${PRODUCT_VERSION}")
    message(STATUS "  COMPANY_NAME = ${COMPANY_NAME}")

    # 直接生成RC文件内容（不使用模板）
    set(RC_CONTENT
            "#include <windows.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION     ${FILE_VERSION_COMMA}
PRODUCTVERSION  1,0,0,0
FILEFLAGSMASK   VS_FFI_FILEFLAGSMASK
#ifdef _DEBUG
FILEFLAGS       VS_FF_DEBUG
#else
FILEFLAGS       0x0L
#endif
FILEOS          VOS_NT_WINDOWS32
FILETYPE        VFT_DLL
FILESUBTYPE     VFT2_UNKNOWN
BEGIN
    BLOCK \"StringFileInfo\"
    BEGIN
        BLOCK \"040904b0\"
        BEGIN
            VALUE \"CompanyName\",      \"${COMPANY_NAME}\\0\"
            VALUE \"FileDescription\",  \"${TARGET_PRODUCT_NAME}\\0\"
            VALUE \"FileVersion\",      \"${FILE_VERSION}\\0\"
            VALUE \"InternalName\",     \"${TARGET_PRODUCT_NAME}\\0\"
            VALUE \"LegalCopyright\",   \"Copyright (C) ${COMPANY_NAME}\\0\"
            VALUE \"OriginalFilename\", \"${TARGET_PRODUCT_NAME}.dll\\0\"
            VALUE \"ProductName\",      \"${TARGET_PRODUCT_NAME}\\0\"
            VALUE \"ProductVersion\",   \"${PRODUCT_VERSION}\\0\"
        END
    END
    BLOCK \"VarFileInfo\"
    BEGIN
        VALUE \"Translation\", 0x0409, 1200
    END
END
")

    # 创建输出目录
    set(RC_OUTPUT "${CMAKE_BINARY_DIR}/version_info/${TARGET_NAME}_version.rc")
    get_filename_component(RC_OUTPUT_DIR "${RC_OUTPUT}" DIRECTORY)
    file(MAKE_DIRECTORY "${RC_OUTPUT_DIR}")

    # 直接写入RC文件
    file(WRITE "${RC_OUTPUT}" "${RC_CONTENT}")

    # 将版本资源添加到目标
    target_sources(${TARGET_NAME} PRIVATE ${RC_OUTPUT})

    message(STATUS "Version RC file generated: ${RC_OUTPUT}")
endfunction()

# 批量为多个目标添加版本信息
function(add_version_info_to_targets)
    foreach(TARGET_NAME ${ARGN})
        add_version_info(${TARGET_NAME})
    endforeach()
endfunction()