# Copyright (c) 2026 Yonghua Liao
# Author: Yonghua Liao
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20)
project(occ_c_bridge VERSION 1.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OCB_BUILD_TESTS "Build tests" ON)
# 静态链接 OCCT：产物 dll 不再依赖 TK*.dll（用于开源交付/独立分发）。
# 需要一份静态编译的 OCCT（BUILD_LIBRARY_TYPE=Static），默认从
# OCB_OCCT_STATIC_ROOT 指向的构建树的 win64/vc14/lib 取静态库。
option(OCB_OCC_STATIC "Link OCCT statically (no TK*.dll dependency)" OFF)
set(OCB_OCCT_STATIC_ROOT "" CACHE PATH "OCCT static build root (contains win64/vc14/lib)")
# Static OCCT built with USE_FREETYPE=ON needs the Freetype library here
# (runtime still requires freetype.dll unless a static Freetype build is given).
set(OCB_FREETYPE_LIB "" CACHE FILEPATH "Freetype library for USE_FREETYPE=ON static OCCT")
set(OCB_OCCT_ROOT "C:/OpenCASCADE-7.7.0-vc14-64/opencascade-7.7.0"
    CACHE PATH "OpenCASCADE root directory")
set(OCB_3RDPARTY_ROOT "C:/OpenCASCADE-7.7.0-vc14-64"
    CACHE PATH "OpenCASCADE third-party root (tbb, freetype, ...)")

# 静态模式下优先使用静态构建树自带的 inc，确保头文件与静态库同源。
if(OCB_OCC_STATIC AND OCB_OCCT_STATIC_ROOT)
    set(OCB_OCCT_INC_DIR "${OCB_OCCT_STATIC_ROOT}/inc")
else()
    set(OCB_OCCT_INC_DIR "${OCB_OCCT_ROOT}/inc")
endif()
if(NOT EXISTS "${OCB_OCCT_INC_DIR}/Standard.hxx")
    message(FATAL_ERROR "OpenCASCADE headers not found at ${OCB_OCCT_INC_DIR}")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(occ_c_bridge SHARED
    src/occ_c_bridge.cpp
    src/occ_c_ocaf.cpp
    src/occ_c_shape.cpp
    src/occ_c_io.cpp
    src/occ_c_viewer.cpp
    src/handle_table.h
)
target_include_directories(occ_c_bridge
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    PRIVATE "${OCB_OCCT_INC_DIR}"
)
target_compile_definitions(occ_c_bridge PRIVATE OCB_EXPORTS)
target_precompile_headers(occ_c_bridge PRIVATE
    <AIS_InteractiveContext.hxx>
    <AIS_Shape.hxx>
    <BRepAlgoAPI_Fuse.hxx>
    <BRepBuilderAPI_Transform.hxx>
    <BRepMesh_IncrementalMesh.hxx>
    <BRepPrimAPI_MakeBox.hxx>
    <BRep_Tool.hxx>
    <BinXCAFDrivers.hxx>
    <TDF_Label.hxx>
    <TDocStd_Application.hxx>
    <TDocStd_Document.hxx>
    <TopoDS_Shape.hxx>
    <XCAFDoc_DocumentTool.hxx>
    <XCAFDoc_ShapeTool.hxx>
    <gp_Pnt.hxx>
    <gp_Trsf.hxx>
    <gp_Vec.hxx>
    <string>
    <vector>
    <unordered_map>
    <memory>)
if(WIN32)
    target_sources(occ_c_bridge PRIVATE src/occ_c_bridge.def)
endif()

if(MSVC)
    target_compile_options(occ_c_bridge PRIVATE /W4)
endif()

set(OCB_OCCT_LIBS
    TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo
    TKBRep TKTopAlgo TKPrim TKBO TKBool TKShHealing TKMesh TKHLR TKFeat TKFillet TKOffset
    TKCDF TKLCAF TKCAF TKVCAF TKXCAF
    TKBin TKBinL TKBinXCAF TKStd TKStdL TKTObj
    TKSTEPBase TKSTEPAttr TKSTEP209 TKSTEP TKXSBase TKXDESTEP
    TKService TKV3d TKOpenGl
)

if(WIN32)
    if(OCB_OCC_STATIC)
        if(NOT OCB_OCCT_STATIC_ROOT)
            message(FATAL_ERROR "OCB_OCC_STATIC=ON requires -DOCB_OCCT_STATIC_ROOT=<static build root>")
        endif()
        if(WIN32)
            set(OCB_OCCT_LIB_DIR "${OCB_OCCT_STATIC_ROOT}/win64/vc14/lib")
        else()
            set(OCB_OCCT_LIB_DIR "${OCB_OCCT_STATIC_ROOT}/lib")
        endif()
        message(STATUS "occ_c_bridge: static OCCT from ${OCB_OCCT_LIB_DIR}")
    else()
        set(OCB_OCCT_LIB_DIR "${OCB_OCCT_ROOT}/win64/vc14/lib")
        set(OCB_OCCT_BIN_DIR "${OCB_OCCT_ROOT}/win64/vc14/bin")
    endif()
else()
    set(OCB_OCCT_LIB_DIR "${OCB_OCCT_ROOT}/lib")
    set(OCB_OCCT_BIN_DIR "${OCB_OCCT_ROOT}/bin")
endif()

target_link_directories(occ_c_bridge PRIVATE "${OCB_OCCT_LIB_DIR}")
target_link_libraries(occ_c_bridge PRIVATE ${OCB_OCCT_LIBS})
if(OCB_OCC_STATIC AND WIN32)
    # 静态 OCCT 需要系统库（OSD_File 的 recv/send、OpenGl 的 wgl、GDI 等）
    target_link_libraries(occ_c_bridge PRIVATE ws2_32 user32 gdi32 opengl32 comdlg32 winmm advapi32 shell32 ole32 oleaut32 uuid)
    if(OCB_FREETYPE_LIB)
        target_link_libraries(occ_c_bridge PRIVATE "${OCB_FREETYPE_LIB}")
    endif()
    target_compile_definitions(occ_c_bridge PRIVATE OCB_OCC_STATIC_BUILD)
endif()

if(OCB_BUILD_TESTS)
    enable_testing()
    add_executable(occ_c_bridge_tests
        tests/test_main.cpp
        tests/test_geometry.cpp
        tests/test_shape.cpp
        tests/test_handle.cpp
        tests/test_ocaf.cpp
        tests/test_geom2.cpp
        tests/test_primitives.cpp
        tests/test_topology.cpp
        tests/test_mesh_props.cpp
        tests/test_io.cpp
        tests/test_ais.cpp
        tests/test_ocaf2.cpp
        tests/test_contract.cpp
        tests/test_integration.cpp
        tests/test_query.cpp
        tests/test_features.cpp
        tests/test_math.cpp
        tests/test_gp_parity.cpp
        tests/test_lights.cpp
    tests/test_view_host.cpp
    tests/test_api_basic.cpp
        tests/test_wrap.cpp
        tests/test_guard.cpp
        tests/test_v12_p0.cpp
        tests/test_v13.cpp
    )
    target_link_libraries(occ_c_bridge_tests PRIVATE occ_c_bridge ${OCB_OCCT_LIBS})
    target_include_directories(occ_c_bridge_tests PRIVATE include tests tests/third_party "${OCB_OCCT_INC_DIR}")
    
    target_link_directories(occ_c_bridge_tests PRIVATE "${OCB_OCCT_LIB_DIR}")
    if(OCB_OCC_STATIC AND WIN32)
        target_link_libraries(occ_c_bridge_tests PRIVATE ws2_32 user32 gdi32 opengl32 comdlg32 winmm advapi32 shell32 ole32 oleaut32 uuid)
    endif()

    if(MSVC)
        target_compile_options(occ_c_bridge_tests PRIVATE /W4)
    endif()

    if(WIN32)
        add_custom_command(TARGET occ_c_bridge_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${OCB_OCCT_BIN_DIR}"
                "$<TARGET_FILE_DIR:occ_c_bridge_tests>"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${OCB_3RDPARTY_ROOT}/tbb_2021.5-vc14-64/bin"
                "$<TARGET_FILE_DIR:occ_c_bridge_tests>"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${OCB_3RDPARTY_ROOT}/freetype-2.5.5-vc14-64/bin"
                "$<TARGET_FILE_DIR:occ_c_bridge_tests>"
            COMMENT "Copy OpenCASCADE runtime DLLs for tests"
        )
    endif()

    add_test(NAME occ_c_bridge_tests COMMAND occ_c_bridge_tests)
endif()

# 安装/分发规则（include/lib/dll/CMake package config）
include(cmake/OcbInstall.cmake)









