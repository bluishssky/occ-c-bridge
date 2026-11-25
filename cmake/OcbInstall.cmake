# Copyright (c) 2026 Yonghua Liao
# Author: Yonghua Liao
# SPDX-License-Identifier: Apache-2.0
# 安装/分发规则（由 CMakeLists.txt 末尾 include）
# 用法：cmake --install build --config Release --prefix <目标目录>
# 产出：include/*.h、lib/occ_c_bridge.lib、bin/occ_c_bridge.dll、
#       lib/cmake/occ_c_bridge/{occ_c_bridgeConfig.cmake, ...ConfigVersion.cmake, ...Targets.cmake}
# 说明：桥本身依赖 OCCT 运行库（TK*.dll）与 TBB/FreeType，由消费方部署其 OCCT 运行环境。
#       安装树同时包含第三方声明与许可证全文（share/doc/occ_c_bridge/）。

include(GNUInstallDirs)

install(TARGETS occ_c_bridge
    EXPORT occ_c_bridgeTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    include/occ_c_bridge.h
    include/occ_c_types.h
    include/occ_c_guard.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES docs/consumer-integration.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    OPTIONAL
)

install(FILES
    LICENSE
    NOTICE
    THIRD_PARTY_NOTICES.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

install(DIRECTORY third_party/licenses/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/licenses
)

install(DIRECTORY LICENSES/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/licenses
)

install(EXPORT occ_c_bridgeTargets
    NAMESPACE occ_c_bridge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/occ_c_bridge
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/occ_c_bridgeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/occ_c_bridgeConfig.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/occ_c_bridgeTargets.cmake\")\n")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/occ_c_bridgeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/occ_c_bridgeConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/occ_c_bridge
)
