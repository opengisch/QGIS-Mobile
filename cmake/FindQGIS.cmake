## Once run this will define:
##
## QGIS_FOUND            = system has QGIS lib
##
## QGIS_CORE_LIBRARY     = full path to the CORE library
## QGIS_ANALYSIS_LIBRARY = full path to the ANALYSIS library
## QGIS_INCLUDE_DIR      = where to find headers
##
## QGIS_VERSION          = version as defined in qgsconfig.h, as major.minor.patch
##
## Definitions or ENV variables affecting search locations
##
## OSGEO4W_ROOT          = [A-Z]:/path/to/OSGeo4W/install/root
##                               (^ use forward slashes!)
## OSGEO4W_QGIS_SUBDIR   = qgis[-rel|-ltr][-dev], in OSGEO4W_ROOT/apps/
## QGIS_MAC_PATH         = /path/to/any/QGIS.app/Contents
## QGIS_BUILD_PATH       = [A-Z:]/path/to/QGIS/build/directory
##
## Tim Sutton
## Larry Shaffer (2017-01-31)

#MESSAGE("Searching for QGIS")
IF(WIN32)
  # OSGEO4W_QGIS_SUBDIR relative install: qgis[-rel|-ltr][-dev], etc.
  IF (NOT OSGEO4W_QGIS_SUBDIR OR "${OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
    IF (NOT "$ENV{OSGEO4W_QGIS_SUBDIR}" STREQUAL "")
      SET (OSGEO4W_QGIS_SUBDIR $ENV{OSGEO4W_QGIS_SUBDIR})
    ELSE ()
      SET (OSGEO4W_QGIS_SUBDIR qgis)
    ENDIF ()
  ENDIF ()

  #MESSAGE("Searching for QGIS in $ENV{PROGRAMFILES}/QGIS")
  IF (MINGW)
    FIND_PATH(QGIS_INCLUDE_DIR
      NAMES qgsapplication.h
      PATHS
        "$ENV{PROGRAMFILES}/QGIS/include"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS
        "$ENV{PROGRAMFILES}/QGIS/"
    )
  ENDIF (MINGW)

  IF (MSVC)
    FIND_PATH(QGIS_PLUGIN_DIR
      NAMES wmsprovider.dll
      PATHS
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/plugins"
        "$ENV{PROGRAMFILES}/QGIS/plugins"
        "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qgis/plugins"
    )
    FIND_PATH(QGIS_INCLUDE_DIR
      NAMES qgsapplication.h
      PATHS
        "$ENV{INCLUDE}"
        "$ENV{LIB_DIR}/include/qgis"
        "$ENV{OSGEO4W_ROOT}/include"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/include"
        "$ENV{PROGRAMFILES}/QGIS/include"
    )
    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS
        "$ENV{LIB_DIR}/lib/"
        "$ENV{LIB}"
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
    )
    FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
      NAMES qgis_analysis
      PATHS
        "$ENV{LIB_DIR}"
        "$ENV{LIB}"
        "$ENV{OSGEO4W_ROOT}/lib"
        "$ENV{OSGEO4W_ROOT}/apps/${OSGEO4W_QGIS_SUBDIR}/lib"
        "$ENV{PROGRAMFILES}/QGIS/lib"
    )
  ENDIF (MSVC)
ELSE(WIN32)
  IF(UNIX)
    #MESSAGE("Searching for QGIS in /usr/bin; /usr/local/bin")
    FIND_PATH(QGIS_INCLUDE_DIR
      NAMES qgis.h
      PATHS
        ${CMAKE_PREFIX_PATH}
        ${QGIS_BUILD_PATH}/output/lib/qgis_core.framework/Headers
        ${QGIS_MAC_PATH}/Frameworks/qgis_core.framework/Headers
        ${QGIS_PREFIX_PATH}/include/qgis
        /usr/include/qgis
        /usr/local/include/qgis
        /Library/Frameworks/qgis_core.framework/Headers
        "$ENV{LIB_DIR}/include/qgis"
      PATH_SUFFIXES QGIS.app/Contents/Frameworks/qgis_core.framework/Headers
    )

    # also get other frameworks' headers folders on OS X
    IF (APPLE AND USE_MAC_BUNDLING)
      FIND_PATH(QGIS_ANALYSIS_INCLUDE_DIR
        NAMES qgsanalysis.h
        PATHS
          ${CMAKE_PREFIX_PATH}
          ${QGIS_BUILD_PATH}/output/lib
          ${QGIS_MAC_PATH}/Frameworks
          /Library/Frameworks
        PATH_SUFFIXES
          qgis_analysis.framework/Headers
          QGIS.app/Contents/Frameworks/qgis_analysis.framework/Headers
      )
      SET(QGIS_INCLUDE_DIR
        ${QGIS_INCLUDE_DIR}
        ${QGIS_ANALYSIS_INCLUDE_DIR}
      )

      # qgis_core is an archive without extensions for ios
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} "")
    ENDIF (APPLE AND USE_MAC_BUNDLING)


    FIND_LIBRARY(QGIS_CORE_LIBRARY
      NAMES qgis_core
      PATHS
        ${CMAKE_PREFIX_PATH}
        ${QGIS_BUILD_PATH}/output/lib
        ${QGIS_MAC_PATH}/Frameworks
        ${QGIS_MAC_PATH}/lib
        ${QGIS_PREFIX_PATH}/lib/
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /Library/Frameworks
        "$ENV{LIB_DIR}/lib/"
      PATH_SUFFIXES
        QGIS.app/Contents/Frameworks/qgis_core.framework
    )
    FIND_LIBRARY(QGIS_ANALYSIS_LIBRARY
      NAMES qgis_analysis
      PATHS
        ${CMAKE_PREFIX_PATH}
        ${QGIS_BUILD_PATH}/output/lib
        ${QGIS_MAC_PATH}/Frameworks
        ${QGIS_MAC_PATH}/lib
        ${QGIS_PREFIX_PATH}/lib/
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /Library/Frameworks
        "$ENV{LIB_DIR}/lib/"
      PATH_SUFFIXES
        QGIS.app/Contents/Frameworks/qgis_analysis.framework
    )

    IF (APPLE AND USE_MAC_BUNDLING)
      FIND_PATH(QGIS_APPLE_RESOURCES_DIR
        NAMES FindQGIS.cmake
        PATHS
          ${CMAKE_PREFIX_PATH}
          ${QGIS_BUILD_PATH}/output/bin
          ${QGIS_MAC_PATH}/Frameworks
          /Library/Frameworks
        PATH_SUFFIXES
          QGIS.app/Contents/Resources
        REQUIRED
      )
    ENDIF()

    # Not sure how this affect other build, so let's keep that only for iOs for now
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")

      function(find_and_link_library TARGET LIBRARY FORCE_SDK)
        if(FORCE_SDK)
          set(ADDITIONAL_ARGS "NO_CMAKE_FIND_ROOT_PATH")
        endif()
        find_library(${LIBRARY}-LIBRARY ${LIBRARY} ${ADDITIONAL_ARGS})
        if(${LIBRARY}-LIBRARY)
          message(STATUS "Link ${${LIBRARY}-LIBRARY} interface to ${TARGET}")
          target_link_libraries(${TARGET} INTERFACE ${${LIBRARY}-LIBRARY})
        else()
          message(FATAL_ERROR "Fail to find library ${LIBRARY}. Make sure it is present in CMAKE_PREFIX_PATH")
        endif()
      endfunction()

      # Create an interface library for qgis_core & qgis_analysis
      # In the best world, this should have been properly handled inside qgis
      # qgis should use cpack, and generate its correct find cmake scripts.
      # If this is implemented in QGIS in the future, this whole won't have a reason to exist.

      add_library(qgis_core INTERFACE)
      target_link_libraries(qgis_core INTERFACE ${QGIS_CORE_LIBRARY})

      function(qgis_core_link_library LIBRARY FORCE_SDK)
        find_and_link_library(qgis_core ${LIBRARY} ${FORCE_SDK})
      endfunction()

      # qgis_core is built using bz2, xml2 from sysroot
      qgis_core_link_library(bz2 FALSE)
      qgis_core_link_library(z FALSE)
      qgis_core_link_library(xml2 FALSE)
      qgis_core_link_library(iconv FALSE)
      qgis_core_link_library(charset TRUE)
      qgis_core_link_library(expat TRUE)
      qgis_core_link_library(freexl TRUE)
      qgis_core_link_library(gdal TRUE)
      qgis_core_link_library(geos TRUE)
      qgis_core_link_library(geos_c TRUE)
      qgis_core_link_library(pq TRUE)
      qgis_core_link_library(proj TRUE)
      # Be careful, protobuf/protobuf-lite are also in sysroot. But we want to use the one from our sdk
      qgis_core_link_library(protobuf-lite TRUE)
      qgis_core_link_library(protobuf TRUE)
      qgis_core_link_library(protoc TRUE)
      qgis_core_link_library(qt5keychain TRUE)
      qgis_core_link_library(qca-qt5 TRUE)
      qgis_core_link_library(spatialindex TRUE)
      qgis_core_link_library(spatialite TRUE)
      qgis_core_link_library(tasn1 TRUE)
      qgis_core_link_library(zip TRUE)

      find_library(QGIS_WSMPROVIDER_PLUGIN_LIBRARY wmsprovider_a
        PATH_SUFFIXES
         QGIS.app/Contents/PlugIns/qgis/
      )
      if(QGIS_WSMPROVIDER_PLUGIN_LIBRARY)
        message(STATUS "Link ${QGIS_WSMPROVIDER_PLUGIN_LIBRARY} interface to qgis_core")
        target_link_libraries(qgis_core INTERFACE ${QGIS_WSMPROVIDER_PLUGIN_LIBRARY})
      else()
        message(FATAL_ERROR "Fail to find library wmsprovider_a. Make sure it is present in CMAKE_PREFIX_PATH/QGIS.app/Contents/PlugIns/qgis/")
      endif()

      find_library(QGIS_POSTGRESPROVIDER_PLUGIN_LIBRARY postgresprovider_a
        PATH_SUFFIXES
         QGIS.app/Contents/PlugIns/qgis/
      )
      if(QGIS_POSTGRESPROVIDER_PLUGIN_LIBRARY)
        message(STATUS "Link ${QGIS_POSTGRESPROVIDER_PLUGIN_LIBRARY} interface to qgis_core")
        target_link_libraries(qgis_core INTERFACE ${QGIS_POSTGRESPROVIDER_PLUGIN_LIBRARY})
      else()
        message(FATAL_ERROR "Fail to find library postgresprovider_a. Make sure it is present in CMAKE_PREFIX_PATH/QGIS.app/Contents/PlugIns/qgis/")
      endif()

      find_library(QGIS_POSTGRESRASTERPROVIDER_PLUGIN_LIBRARY postgresrasterprovider_a
        PATH_SUFFIXES
         QGIS.app/Contents/PlugIns/qgis/
      )
      if(QGIS_POSTGRESRASTERPROVIDER_PLUGIN_LIBRARY)
        message(STATUS "Link ${QGIS_POSTGRESRASTERPROVIDER_PLUGIN_LIBRARY} interface to qgis_core")
        target_link_libraries(qgis_core INTERFACE ${QGIS_POSTGRESRASTERPROVIDER_PLUGIN_LIBRARY})
      else()
        message(FATAL_ERROR "Fail to find library postgresrasterprovider_a. Make sure it is present in CMAKE_PREFIX_PATH/QGIS.app/Contents/PlugIns/qgis/")
      endif()

      target_include_directories(qgis_core INTERFACE ${QGIS_INCLUDE_DIR})

      # Versionless tagging of Qt, only supported from Qt5.15
      if(NOT TARGET Qt::PrintSupport)
        target_compile_definitions(qgis_core INTERFACE "-DQT_NO_PRINTER")
      endif()

      function(qgis_analysis_link_library LIBRARY FORCE_SDK)
        find_and_link_library(qgis_analysis ${LIBRARY} ${FORCE_SDK})
      endfunction()

      add_library(qgis_analysis INTERFACE)
      target_link_libraries(qgis_analysis INTERFACE ${QGIS_ANALYSIS_LIBRARY} qgis_core)

      qgis_analysis_link_library(exiv2 TRUE)
      qgis_analysis_link_library(exiv2-xmp TRUE)

      target_include_directories(qgis_analysis INTERFACE ${QGIS_ANALYSIS_INCLUDE_DIR})

      # Overwrite the variable to point to the interface library
      set(QGIS_CORE_LIBRARY qgis_core)
      set(QGIS_ANALYSIS_LIBRARY qgis_analysis)

      find_file(PROJ_DB proj.db PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES share/proj REQUIRED)
      get_filename_component(QFIELD_PROJ_DIR ${PROJ_DB} DIRECTORY)
      message("Found Proj DB directory: " ${QFIELD_PROJ_DIR})

      FIND_PATH(PROJ_INCLUDE_DIR proj_api.h NO_CMAKE_FIND_ROOT_PATH)
      MESSAGE("Found Proj include dir: " ${PROJ_INCLUDE_DIR})

      FIND_PATH(GDAL_INCLUDE_DIR gdal.h  NO_CMAKE_FIND_ROOT_PATH)
      MESSAGE("Found GDAL include dir: " ${GDAL_INCLUDE_DIR})

    ENDIF() # iOs

  ENDIF(UNIX)
ENDIF(WIN32)

IF (QGIS_INCLUDE_DIR)
  SET(QGIS_VERSION QGIS_VERSION-NOTFOUND)
  FIND_FILE(_qgsconfig_h qgsconfig.h PATHS ${QGIS_INCLUDE_DIR})
  IF (_qgsconfig_h)
    FILE(READ ${_qgsconfig_h} _qgsconfig)
    IF (_qgsconfig)
      # version defined like #define VERSION "2.14.8-Essen"
      FILE(STRINGS "${_qgsconfig_h}" _qgsversion_str REGEX "^#define VERSION .*$")
      STRING(REGEX REPLACE "^#define VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1" _qgsversion "${_qgsversion_str}")
      IF (_qgsversion)
        SET(QGIS_VERSION ${_qgsversion})
      ELSE ()
        MESSAGE(WARNING "No QGIS version determined: failed to parse qgsconfig.h")
      ENDIF ()
    ELSE()
      MESSAGE(WARNING "No QGIS version determined: failed to read qgsconfig.h")
    ENDIF ()
  ELSE ()
    MESSAGE(WARNING "No QGIS version determined: failed to find qgsconfig.h")
  ENDIF ()
ENDIF ()

IF (QGIS_INCLUDE_DIR AND QGIS_CORE_LIBRARY AND QGIS_ANALYSIS_LIBRARY)
   SET(QGIS_FOUND TRUE)
ELSE ()
  # Useful logs addressed to sdk mainteners
  IF(NOT QGIS_INCLUDE_DIR)
    MESSAGE(WARNING "Fail to find QGIS core include folder")
  ENDIF ()
  IF(NOT QGIS_CORE_LIBRARY)
    MESSAGE(WARNING "Fail to find QGIS core include folder")
  ENDIF ()
  IF(NOT QGIS_ANALYSIS_INCLUDE_DIR)
    MESSAGE(WARNING "Fail to find QGIS analysis include folder")
  ENDIF ()
  IF(NOT QGIS_ANALYSIS_LIBRARY)
    MESSAGE(WARNING "Fail to find QGIS analysis include folder")
  ENDIF ()
ENDIF ()

IF (QGIS_FOUND)
   IF (NOT QGIS_FIND_QUIETLY)
     MESSAGE(STATUS "Found QGIS: ${QGIS_VERSION}")
     MESSAGE(STATUS "Found QGIS core: ${QGIS_CORE_LIBRARY}")
     MESSAGE(STATUS "Found QGIS analysis: ${QGIS_ANALYSIS_LIBRARY}")
   ENDIF (NOT QGIS_FIND_QUIETLY)
ELSE (QGIS_FOUND)
   IF (QGIS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find QGIS. Make sure QGIS sdk is located in CMAKE_PREFIX_PATH")
   ENDIF (QGIS_FIND_REQUIRED)
ENDIF (QGIS_FOUND)
