# Copyright (c) 2026 Yonghua Liao
# Author: Yonghua Liao
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.16)
project(ocb_minimal_consumer CXX)

# 只依赖安装包：不 include 任何 OpenCASCADE 头文件
find_package(occ_c_bridge 1.1 CONFIG REQUIRED)

add_executable(ocb_minimal_consumer main.cpp)
target_link_libraries(ocb_minimal_consumer PRIVATE occ_c_bridge::occ_c_bridge)
target_compile_features(ocb_minimal_consumer PRIVATE cxx_std_17)

# DLL 与本程序同目录即可运行（发行包把依赖 DLL 放在 bin/）
