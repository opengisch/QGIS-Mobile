if (ANDROID)
  add_library(qfield_service SHARED
    main.cpp
  )
endif()

#target_compile_definitions(qfield_service
#  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

if (ANDROID)
    target_link_libraries(qfield_service PRIVATE
      android
      log
    )
endif()

target_link_libraries(qfield_service PRIVATE
  qfield_core
  ${QGIS_CORE_LIBRARY}
)

set_target_properties(qfield PROPERTIES AUTORCC TRUE)
install(TARGETS qfield_service
  BUNDLE DESTINATION ${QFIELD_BIN_DIR}
  RUNTIME DESTINATION ${QFIELD_BIN_DIR}
  LIBRARY DESTINATION ${QFIELD_LIB_DIR}
  ARCHIVE DESTINATION ${QFIELD_LIB_DIR}
  FRAMEWORK DESTINATION ${QFIELD_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QFIELD_INCLUDE_DIR}
)
