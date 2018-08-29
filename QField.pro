TEMPLATE = subdirs
CONFIG += c++11
CONFIG += qt

TRANSLATIONS = \
  i18n/qfield_ca.ts \
  i18n/qfield_de.ts \
  i18n/qfield_en.ts \
  i18n/qfield_es.ts \
  i18n/qfield_et.ts \
  i18n/qfield_fi.ts \
  i18n/qfield_fr.ts \
  i18n/qfield_gl.ts \
  i18n/qfield_hu.ts \
  i18n/qfield_hi.ts \
  i18n/qfield_it.ts \
  i18n/qfield_lt.ts \
  i18n/qfield_mn.ts \
  i18n/qfield_nl.ts \
  i18n/qfield_pl.ts \
  i18n/qfield_pt.ts \
  i18n/qfield_ro.ts \
  i18n/qfield_rm.ts \
  i18n/qfield_uk.ts

SUBDIRS += 3rdparty qgsquick core app

qgsquick.subdir = src/qgsquick
core.subdir = src/core
app.subdir = src/app

core.depends = qgsquick

app.depends = 3rdparty
app.depends = qgsquick
app.depends = core

CONFIG += ordered

# Include the tests and plugins subprojects only on debug mode
CONFIG(debug, debug|release): {
  SUBDIRS += test
  test.depends = qgsquick
  test.depends = core
}

