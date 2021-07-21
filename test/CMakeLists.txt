macro (ADD_QFIELD_TEST TESTNAME TESTSRC)
  add_executable(${TESTNAME} ${TESTSRC})
  set_target_properties(${TESTNAME} PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${TESTNAME} PRIVATE
    qfield_core
    ${QGIS_CORE_LIBRARY}
    ${QGIS_ANALYSIS_LIBRARY}
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
  )
  add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
endmacro (ADD_QFIELD_TEST)

macro (ADD_QFIELD_QML_TEST TESTNAME TESTSRC)
  add_executable(${TESTNAME} ${TESTSRC})
  set_target_properties(${TESTNAME} PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${TESTNAME} PRIVATE
    qfield_core
    ${QGIS_CORE_LIBRARY}
    ${QGIS_ANALYSIS_LIBRARY}
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::QuickTest
  )
  add_test(NAME ${TESTNAME} COMMAND ${TESTNAME} -import ${CMAKE_SOURCE_DIR}/src/qml/imports -input ${CMAKE_SOURCE_DIR}/test/qml)
endmacro (ADD_QFIELD_QML_TEST)

if (WITH_SPIX)
  add_subdirectory(spix)
endif ()

ADD_QFIELD_TEST(vertexmodeltest test_vertexmodel.cpp)
ADD_QFIELD_TEST(referencingfeaturelistmodeltest test_referencingfeaturelistmodel.cpp)
ADD_QFIELD_TEST(orderedrelationmodeltest test_orderedrelationmodel.cpp)
ADD_QFIELD_TEST(deltafilewrappertest test_deltafilewrapper.cpp)
ADD_QFIELD_TEST(layerobservertest test_layerobserver.cpp)
ADD_QFIELD_TEST(featureutilstest test_featureutils.cpp)
ADD_QFIELD_TEST(fileutilstest test_fileutils.cpp)
ADD_QFIELD_TEST(geometryutilstest test_geometryutils.cpp)
ADD_QFIELD_TEST(stringutilstest test_stringutils.cpp)
ADD_QFIELD_TEST(urlutilstest test_urlutils.cpp)
ADD_QFIELD_TEST(digitizingloggertest test_digitizinglogger.cpp)

ADD_QFIELD_QML_TEST(qmltest test_qml_editorwidgets.cpp)

