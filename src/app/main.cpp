/***************************************************************************
                            main.cpp  -  main for qgis mobileapp
                            based on src/browser/main.cpp
                              -------------------
              begin                : Wed Apr 04 10:48:28 CET 2012
              copyright            : (C) 2012 by Marco Bernasocchi
              email                : marco@bernawebdesign.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgismobileapp.h"

#include <QLocale>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include <QMainWindow>
#include <QLabel>
#include <QDialog>
#include <QApplication>
#include "qgsapplication.h"
#include "qgslogger.h"
#include <qmainwindow.h>
#include <QApplication>
#include <QDebug>

int main( int argc, char ** argv )
{
#if 0
  // A dummy app for reading settings that need to be used before constructing the real app
  QCoreApplication* dummyApp = new QCoreApplication( argc, argv );
  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "OPENGIS.ch" );
  QCoreApplication::setOrganizationDomain( "opengis.ch" );
  QCoreApplication::setApplicationName( "QField" );

  if ( settings.value( "/HighDpiScaling", false ).toBool() )
    QGuiApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
  delete dummyApp;
#endif

  QgsApplication app( argc, argv, true );
  QSettings settings;

  app.setThemeName( settings.value( "/Themes", "default" ).toString() );

  // load providers
#ifdef ANDROID
  app.setPrefixPath( "" QGIS_INSTALL_DIR, true );
  app.setPluginPath( QApplication::applicationDirPath() );
  app.setPkgDataPath( AndroidPlatformUtilities().packagePath() );
#else
  app.setPrefixPath( CMAKE_INSTALL_PREFIX, true );
#endif
  app.initQgis();

  //set NativeFormat for settings
  QSettings::setDefaultFormat( QSettings::NativeFormat );

  // Set up the QSettings environment must be done after qapp is created
  QCoreApplication::setOrganizationName( "OPENGIS.ch" );
  QCoreApplication::setOrganizationDomain( "opengis.ch" );
  QCoreApplication::setApplicationName( "QField" );

  QTranslator qfieldTranslator;
  QTranslator qtTranslator;
  qfieldTranslator.load( QLocale(), "qfield", "_", ":/" );
  qtTranslator.load( QLocale(), "qt", "_", ":/" );
  app.installTranslator( &qtTranslator );
  app.installTranslator( &qfieldTranslator );

  QgisMobileapp mApp( &app );
  return app.exec();
}
