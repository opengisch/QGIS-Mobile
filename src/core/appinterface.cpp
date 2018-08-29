/***************************************************************************
                            appinterface.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appinterface.h"
#include "qgismobileapp.h"

AppInterface::AppInterface( QgisMobileapp* app )
  : mApp( app )
{
}

void AppInterface::loadProject( const QString& path )
{
  return mApp->loadProjectFile( QUrl( path ).path() );
}

void AppInterface::print( int layoutIndex )
{
  return mApp->print( layoutIndex );
}

void AppInterface::openFeatureForm()
{
  emit openFeatureFormRequested();
}
