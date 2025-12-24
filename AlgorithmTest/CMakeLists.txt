cmake_minimum_required(VERSION 3.21)
project(AlgorithmTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    PrintSupport
)

# src
set(PROJECT_SOURCES
    # src
    main.cpp
    AlgorithmTest.cpp

    # APS Dialog
    DialogAPSBadPixel.cpp
    DialogAPSBLC.cpp
    DialogAPSDataMean.cpp
    DialogAPSDSNU.cpp
    DialogAPSFunction.cpp
    DialogAPSHotPixel.cpp
    DialogAPSImportData.cpp
    DialogAPSInit.cpp
    DialogAPSLinearity.cpp
    DialogAPSLinearityO.cpp
    DialogAPSOETC.cpp
    DialogAPSOverallSystemGain.cpp
    DialogAPSPedestal.cpp
    DialogAPSPTCTest.cpp
    DialogAPSReadNoise.cpp
    DialogAPSSaturation.cpp
    DialogAPSShading.cpp
    DialogAPSShow.cpp
    DialogAPSSNoise.cpp
    DialogAPSTNoise.cpp
    DialogAPSDarkCurrent.cpp

    # DVS Dialog
    DialogDVSAccompaniedPeakAndDelayedPeak.cpp
    DialogDVSBadPixel.cpp
    DialogDVSHotPixel.cpp
    DialogDVSImageContrastSensitivity.cpp
    DialogDVSShow.cpp
    DialogDVSSpatialResponseUniformity.cpp
    DialogDVSStationaryUniformity.cpp
    DialogDVSFindPeak.cpp
    DialogDVSCountEvents.cpp
    DialogDVSFunction.cpp
    DialogDVSImportData.cpp
    DialogDVSStationaryNoise.cpp

    # 自定义控件
    DoubleSlider.cpp
    qcustomplot.cpp
    WidgetChartView.cpp
    WidgetDVSInit.cpp
    WidgetImageView.cpp
    WidgetTableView.cpp
    WidgetWelcome.cpp
)

# .h
set(PROJECT_HEADERS
    AlgorithmTest.h

    # APS.h
    DialogAPSBadPixel.h
    DialogAPSBLC.h
    DialogAPSDataMean.h
    DialogAPSDSNU.h
    DialogAPSFunction.h
    DialogAPSHotPixel.h
    DialogAPSImportData.h
    DialogAPSInit.h
    DialogAPSLinearity.h
    DialogAPSLinearityO.h
    DialogAPSOETC.h
    DialogAPSOverallSystemGain.h
    DialogAPSPedestal.h
    DialogAPSPTCTest.h
    DialogAPSReadNoise.h
    DialogAPSSaturation.h
    DialogAPSShading.h
    DialogAPSShow.h
    DialogAPSSNoise.h
    DialogAPSTNoise.h
    DialogAPSDarkCurrent.h

    # DVS.h
    DialogDVSAccompaniedPeakAndDelayedPeak.h
    DialogDVSBadPixel.h
    DialogDVSCountEvents.h
    DialogDVSFunction.h
    DialogDVSHotPixel.h
    DialogDVSImageContrastSensitivity.h
    DialogDVSImportData.h
    DialogDVSShow.h
    DialogDVSSpatialResponseUniformity.h
    DialogDVSStationaryNoise.h
    DialogDVSStationaryUniformity.h
    DialogDVSFindPeak.h

    # 自定义控件.h
    DoubleSlider.h
    qcustomplot.h
    WidgetChartView.h
    WidgetDVSInit.h
    WidgetImageView.h
    WidgetTableView.h
    WidgetWelcome.h

    # other.h
    AlpMPAlgoInterface.h
    resource.h
)

# UI Files
set(PROJECT_UIS
    AlgorithmTest.ui

    # APS UI Files
    DialogAPSBadPixel.ui
    DialogAPSBLC.ui
    DialogAPSDarkCurrent.ui
    DialogAPSDataMean.ui
    DialogAPSDSNU.ui
    DialogAPSFunction.ui
    DialogAPSHotPixel.ui
    DialogAPSImportData.ui
    DialogAPSInit.ui
    DialogAPSLinearity.ui
    DialogAPSLinearityO.ui
    DialogAPSOETC.ui
    DialogAPSOverallSystemGain.ui
    DialogAPSPedestal.ui
    DialogAPSPTC.ui
    DialogAPSReadNoise.ui
    DialogAPSSaturation.ui
    DialogAPSShading.ui
    DialogAPSShow.ui
    DialogAPSSNoise.ui
    DialogAPSTNoise.ui

    # DVS UI Files
    DialogDVSAccompaniedPeakAndDelayedPeak.ui
    DialogDVSBadPixel.ui
    DialogDVSCountEvents.ui
    DialogDVSFunction.ui
    DialogDVSHotPixel.ui
    DialogDVSImageContrastSensitivity.ui
    DialogDVSImportData.ui
    DialogDVSInit.ui
    DialogDVSShow.ui
    DialogDVSSpatialResponseUniformity.ui
    DialogDVSStationaryNoise.ui
    DialogDVSStationaryUniformity.ui
    DialogDVSFindPeak.ui

    # Widget UI Files
    WidgetChartView.ui
    WidgetImageView.ui
    WidgetTableView.ui
    WidgetWelcome.ui
)

# Resource
set(PROJECT_RESOURCES
    AlgorithmTest.qrc
    AlgorithmTest.rc
)

# exe
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_UIS}
    ${PROJECT_RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
)

# 添加包含目录（AlgorithmLibrary）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接外部库 AlgorithmLibrary
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:AlgorithmLibrary>
)

# 添加库搜索路径
target_link_directories(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/../x64/Release>
)

# Windows特定设置
if(WIN32)
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
endif()

# 编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    QT_DEPRECATED_WARNINGS
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Qt部署, 用于自动复制Qt DLL
if(WIN32)
    # 获取windeployqt路径
    get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt6_bin_dir "${_qt6_qmake_location}" DIRECTORY)

    # 添加部署命令
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${_qt6_bin_dir}/windeployqt.exe"
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Running windeployqt..."
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION "${FILE_VERSION}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release${PROJECT_NAME}/${CURRENT_PLATFORM}/bin"
)
add_version_info(${PROJECT_NAME})
