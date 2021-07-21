#############################################################
# CMake settings
cmake_minimum_required(VERSION 3.5)


set(CMAKE_COLOR_MAKEFILE ON)
set (APP_NAME "QField" CACHE STRING "Application Name")
set (APK_VERSION_CODE "1" CACHE STRING "Apk Version Code (Example: 1)")
set (APP_VERSION "" CACHE STRING "Application Version (Example: 1.0.0)")
set (APP_VERSION_STR "local - dev" CACHE STRING "Application Version Name (Example: 1.0.0 - Homerun)")
set (APP_PACKAGE_NAME "qfield" CACHE STRING "Package name suffix. E.g. qfield --> ch.opengis.qfield")

string(REGEX REPLACE "v" "" CLEAN_APP_VERSION "${APP_VERSION}")

project(${APP_NAME}
  LANGUAGES CXX
  VERSION ${CLEAN_APP_VERSION}
)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)
GET_GIT_HEAD_REVISION(GIT_REFSPEC GIT_REV)

add_definitions(-DAPP_NAME="${APP_NAME}")
add_definitions(-DAPP_VERSION="${APP_VERSION}")
add_definitions(-DAPP_VERSION_STR="${APP_VERSION_STR}")
add_definitions(-DGIT_REV="${GIT_REV}")

cmake_policy(SET CMP0071 NEW)

set (DEFAULT_BIN_SUBDIR     bin)
set (DEFAULT_CGIBIN_SUBDIR  bin)
set (DEFAULT_LIB_SUBDIR     lib${LIB_SUFFIX})
set (DEFAULT_INCLUDE_SUBDIR include/qfield)

set (QFIELD_BIN_SUBDIR     ${DEFAULT_BIN_SUBDIR}     CACHE STRING "Subdirectory where executables will be installed")
set (QFIELD_LIB_SUBDIR     ${DEFAULT_LIB_SUBDIR}     CACHE STRING "Subdirectory where libraries will be installed")
set (QFIELD_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING "Subdirectory where header files will be installed")

mark_as_advanced (QFIELD_INCLUDE_SUBDIR QFIELD_BIN_SUBDIR QFIELD_LIB_SUBDIR)
set (QFIELD_BIN_DIR ${QFIELD_BIN_SUBDIR})
set (QFIELD_LIB_DIR ${QFIELD_LIB_SUBDIR})
set (QFIELD_INCLUDE_DIR ${QFIELD_INCLUDE_SUBDIR})
set (QT_ANDROID_APPLICATION_BINARY "qfield")

# set the default locations where the targets (executables, libraries) will land when compiled
# this is to allow running QField directly from the build directory.
if (ANDROID)
  set (QFIELD_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/android-build)
else()
  set (QFIELD_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QFIELD_OUTPUT_DIRECTORY}/${QFIELD_BIN_SUBDIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QFIELD_OUTPUT_DIRECTORY}/${QFIELD_LIB_SUBDIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/qfield)

set (WITH_SPIX FALSE CACHE STRING "Compile with Spix for testing")

if (WITH_SPIX)
  find_package(Spix)
endif ()

if (ANDROID)
  add_definitions(-DQGIS_INSTALL_DIR="") # TODO: do we need osgeo4a/[lib]/files here? see qgis.pri
  set (ANDROID_TARGET_PLATFORM CACHE INT "The target android platform sdk version")
  set (ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/android)
  set (OSGEO4A_STAGE_DIR NONE CACHE DIRECTORY "OSGEO4A stage path (base path without architecture)")
  set (OSGEO4A_LIB_DIR ${OSGEO4A_STAGE_DIR}/${ANDROID_ABI}/lib)
  list(APPEND ANDROID_EXTRA_LIBS
    ${OSGEO4A_LIB_DIR}/libssl_1_1.so
    ${OSGEO4A_LIB_DIR}/libcrypto_1_1.so
    ${OSGEO4A_LIB_DIR}/libexpat.so
    ${OSGEO4A_LIB_DIR}/libgeos.so
    ${OSGEO4A_LIB_DIR}/libgeos_c.so
    ${OSGEO4A_LIB_DIR}/libgslcblas.so
    ${OSGEO4A_LIB_DIR}/libsqlite3.so
    ${OSGEO4A_LIB_DIR}/libcharset.so
    ${OSGEO4A_LIB_DIR}/libexiv2.so
    ${OSGEO4A_LIB_DIR}/libiconv.so
    ${OSGEO4A_LIB_DIR}/libopenjp2.so
    ${OSGEO4A_LIB_DIR}/libfreexl.so
    ${OSGEO4A_LIB_DIR}/libtiff.so
    ${OSGEO4A_LIB_DIR}/libpng16.so
    ${OSGEO4A_LIB_DIR}/libgdal.so
    ${OSGEO4A_LIB_DIR}/libproj.so
    ${OSGEO4A_LIB_DIR}/libspatialindex.so
    ${OSGEO4A_LIB_DIR}/libpq.so
    ${OSGEO4A_LIB_DIR}/libspatialite.so
    ${OSGEO4A_LIB_DIR}/libgsl.so
    ${OSGEO4A_LIB_DIR}/libgslcblas.so
    ${OSGEO4A_LIB_DIR}/libqca-qt5_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libqgis_core_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libqgis_analysis_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libqgis_native_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libqt5keychain_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libzip.so
    ${OSGEO4A_LIB_DIR}/libzstd.so
    ${OSGEO4A_LIB_DIR}/libpoppler_${ANDROID_ABI}.so
    ${OSGEO4A_LIB_DIR}/libfreetype.so
  )
  find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)

  configure_file(cmake/generated.xml.in ${CMAKE_SOURCE_DIR}/android/res/values/generated.xml @ONLY)
  configure_file(cmake/AndroidManifest.xml.in ${CMAKE_SOURCE_DIR}/android/AndroidManifest.xml @ONLY)

  include(CreateZip)

  file(GLOB PLUGINS_QGIS_PROVIDERS
    ${OSGEO4A_LIB_DIR}/*provider*.so
  )
  file(GLOB PLUGINS_QGIS_AUTHMETHODS
    ${OSGEO4A_LIB_DIR}/*authmethod*.so
  )

  set(SHARE_PATH "${CMAKE_CURRENT_BINARY_DIR}/android-build/assets/share")
  file(COPY ${PLUGINS_QGIS_PROVIDERS} DESTINATION "${SHARE_PATH}/plugins/")
  file(COPY ${PLUGINS_QGIS_AUTHMETHODS} DESTINATION "${SHARE_PATH}/plugins/")
  file(COPY "${OSGEO4A_LIB_DIR}/qca-qt5/crypto/" DESTINATION "${SHARE_PATH}/plugins/crypto/")
  file(COPY "${OSGEO4A_STAGE_DIR}/${ANDROID_ABI}/share/proj/" DESTINATION "${SHARE_PATH}/proj/")
  file(COPY "${OSGEO4A_STAGE_DIR}/${ANDROID_ABI}/files/share/svg/" DESTINATION "${SHARE_PATH}/qgis/svg/")
  file(COPY "${OSGEO4A_STAGE_DIR}/${ANDROID_ABI}/files/share/resources/" DESTINATION "${SHARE_PATH}/qgis/resources/")

  # Strip libs to save space
  file(GLOB_RECURSE PLUGINS_LIBS
    ${SHARE_PATH}/plugins/*.so
  )
  foreach(PLUGINS_LIB IN LISTS PLUGINS_LIBS)
    if(ANDROID_ABI STREQUAL "x86")
      EXECUTE_PROCESS(COMMAND "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android-strip" "${PLUGINS_LIB}")
    elseif(ANDROID_ABI STREQUAL "x86_64")
      EXECUTE_PROCESS(COMMAND "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android-strip" "${PLUGINS_LIB}")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
      EXECUTE_PROCESS(COMMAND "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-android-strip" "${PLUGINS_LIB}")
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
      EXECUTE_PROCESS(COMMAND "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip" "${PLUGINS_LIB}")
    endif()
  endforeach()

  # Remove world map to keep apk size a bit smaller
  file(REMOVE "${SHARE_PATH}/qgis/resources/data/world_map.gpkg")
  file(COPY resources/ DESTINATION "${SHARE_PATH}/qfield/")
endif()

set(QT_MIN_VERSION 5.14.0)
find_package(Qt5 COMPONENTS Concurrent Core Qml Gui Xml Positioning Widgets Network Quick Svg OpenGL Sql Sensors WebView Bluetooth REQUIRED)

# PrintSupport isn't required, because it doesn't exist for ios
# qgis will deal with it an define a public 'QT_NO_PRINTER'
find_package(Qt5 COMPONENTS PrintSupport QUIET)

find_package(QGIS REQUIRED)
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
  find_package(Proj)
  find_package(GDAL)
endif()
find_package(QCA)
find_package(QtKeychain)

if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
  add_definitions(-DQT_NO_PRINTER)

  set(SHARE_PATH "${CMAKE_CURRENT_BINARY_DIR}/share/")

  # keep trailing slash for QField to rename the directory (instead of copying in subdir)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION ${SHARE_PATH}/qfield)
  file(COPY ${QGIS_APPLE_RESOURCES_DIR}/resources DESTINATION ${SHARE_PATH}/qgis)
  file(COPY ${QGIS_APPLE_RESOURCES_DIR}/svg DESTINATION ${SHARE_PATH}/qgis)
  file(COPY ${QFIELD_PROJ_DIR} DESTINATION ${SHARE_PATH})
  # Remove world map to keep apk size a bit smaller
  file(REMOVE "${SHARE_PATH}/qgis/resources/data/world_map.gpkg")

  set(QFIELD_RESOURCE_FILES ${SHARE_PATH})
endif()

set (ENABLE_TESTS TRUE CACHE BOOL "Build unit tests?")

if(MSVC)
  find_package(Qt5 COMPONENTS Charts REQUIRED) # vcpkg doesn't include QtCharts.dll as dep of the qml module otherwise
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
endif()

add_subdirectory(3rdparty/tessellate)
add_subdirectory(src/core)
add_subdirectory(src/app)

if (ENABLE_TESTS)
  find_package(Qt5 COMPONENTS Test QuickTest)
  enable_testing()
  add_subdirectory(test)
endif()

if(MSVC)
  file(GLOB PROVIDER_LIBS
    "${QGIS_PLUGIN_DIR}/*provider.dll"
  )
  add_custom_target(deploy)
  add_custom_command(TARGET deploy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/output/bin/qgis/plugins"
  )
  foreach(LIB ${PROVIDER_LIBS})
    add_custom_command(TARGET deploy
                      POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LIB}" "${CMAKE_BINARY_DIR}/output/bin/qgis/plugins"
    )
  endforeach()
  add_custom_command(TARGET deploy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${QGIS_PLUGIN_DIR}/../resources" "${CMAKE_BINARY_DIR}/output/bin/qgis/resources"
  )
  add_custom_command(TARGET deploy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${QGIS_PLUGIN_DIR}/../../../share/proj4" "${CMAKE_BINARY_DIR}/output/bin/proj"
  )
  add_custom_command(TARGET deploy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${QGIS_PLUGIN_DIR}/../../../share/gdal" "${CMAKE_BINARY_DIR}/output/bin/gdal"
  )

  add_custom_command(TARGET deploy
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${QGIS_PLUGIN_DIR}/../../../plugins/mediaservice" "${CMAKE_BINARY_DIR}/output/bin/plugins/mediaservice"
  )

  add_dependencies(qfield deploy)
endif()
