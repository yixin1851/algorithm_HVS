# Helper for CLion/Ninja + MSVC setups where vcvars environment variables are
# not propagated to compile/link commands.

function(alp_collect_msvc_toolchain_paths OUT_INCLUDE_DIRS OUT_LIBRARY_DIRS)
    set(INCLUDE_DIRS "")
    set(LIBRARY_DIRS "")

    if(NOT MSVC)
        set(${OUT_INCLUDE_DIRS} "" PARENT_SCOPE)
        set(${OUT_LIBRARY_DIRS} "" PARENT_SCOPE)
        return()
    endif()

    if(CMAKE_CL_64)
        set(MSVC_TARGET_ARCH "x64")
    else()
        set(MSVC_TARGET_ARCH "x86")
    endif()

    if(CMAKE_CXX_COMPILER)
        get_filename_component(CXX_BIN_ARCH_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        get_filename_component(CXX_BIN_HOST_DIR "${CXX_BIN_ARCH_DIR}" DIRECTORY)
        get_filename_component(CXX_BIN_DIR "${CXX_BIN_HOST_DIR}" DIRECTORY)
        get_filename_component(MSVC_TOOLSET_DIR "${CXX_BIN_DIR}" DIRECTORY)

        if(EXISTS "${MSVC_TOOLSET_DIR}/include")
            list(APPEND INCLUDE_DIRS "${MSVC_TOOLSET_DIR}/include")
        endif()
        if(EXISTS "${MSVC_TOOLSET_DIR}/lib/${MSVC_TARGET_ARCH}")
            list(APPEND LIBRARY_DIRS "${MSVC_TOOLSET_DIR}/lib/${MSVC_TARGET_ARCH}")
        endif()
    endif()

    if(CMAKE_RC_COMPILER)
        get_filename_component(RC_ARCH_DIR "${CMAKE_RC_COMPILER}" DIRECTORY)
        get_filename_component(RC_VERSION_BIN_DIR "${RC_ARCH_DIR}" DIRECTORY)
        get_filename_component(RC_BIN_DIR "${RC_VERSION_BIN_DIR}" DIRECTORY)
        get_filename_component(WINDOWS_SDK_ROOT "${RC_BIN_DIR}" DIRECTORY)
        get_filename_component(WINDOWS_SDK_VERSION "${RC_VERSION_BIN_DIR}" NAME)

        set(WINDOWS_SDK_INCLUDE_BASE "${WINDOWS_SDK_ROOT}/Include/${WINDOWS_SDK_VERSION}")
        foreach(SDK_INCLUDE_SUBDIR ucrt shared um winrt cppwinrt)
            if(EXISTS "${WINDOWS_SDK_INCLUDE_BASE}/${SDK_INCLUDE_SUBDIR}")
                list(APPEND INCLUDE_DIRS "${WINDOWS_SDK_INCLUDE_BASE}/${SDK_INCLUDE_SUBDIR}")
            endif()
        endforeach()

        set(WINDOWS_SDK_LIB_BASE "${WINDOWS_SDK_ROOT}/Lib/${WINDOWS_SDK_VERSION}")
        foreach(SDK_LIB_SUBDIR ucrt um)
            if(EXISTS "${WINDOWS_SDK_LIB_BASE}/${SDK_LIB_SUBDIR}/${MSVC_TARGET_ARCH}")
                list(APPEND LIBRARY_DIRS "${WINDOWS_SDK_LIB_BASE}/${SDK_LIB_SUBDIR}/${MSVC_TARGET_ARCH}")
            endif()
        endforeach()
    endif()

    list(REMOVE_DUPLICATES INCLUDE_DIRS)
    list(REMOVE_DUPLICATES LIBRARY_DIRS)

    set(${OUT_INCLUDE_DIRS} "${INCLUDE_DIRS}" PARENT_SCOPE)
    set(${OUT_LIBRARY_DIRS} "${LIBRARY_DIRS}" PARENT_SCOPE)
endfunction()

function(alp_apply_msvc_toolchain_paths TARGET_NAME)
    if(NOT MSVC OR NOT TARGET ${TARGET_NAME})
        return()
    endif()

    alp_collect_msvc_toolchain_paths(MSVC_INCLUDE_DIRS MSVC_LIBRARY_DIRS)

    if(MSVC_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${MSVC_INCLUDE_DIRS})
    endif()

    if(MSVC_LIBRARY_DIRS)
        target_link_directories(${TARGET_NAME} PRIVATE ${MSVC_LIBRARY_DIRS})
    endif()

    message(STATUS "MSVC include dirs for ${TARGET_NAME}: ${MSVC_INCLUDE_DIRS}")
    message(STATUS "MSVC library dirs for ${TARGET_NAME}: ${MSVC_LIBRARY_DIRS}")
endfunction()
