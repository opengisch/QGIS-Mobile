/***************************************************************************
                            expressioncontextutils.h
                              -------------------
              begin                : 5.12.2017
              copyright            : (C) 2017 by Matthias Kuhn
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


#ifndef EXPRESSIONCONTEXTUTILS_H
#define EXPRESSIONCONTEXTUTILS_H

#include <qgsexpressioncontext.h>

class QGeoPositionInfoSource;

class ExpressionContextUtils
{
  public:
    static QgsExpressionContextScope* positionScope( QGeoPositionInfoSource* source );

  private:
    ExpressionContextUtils() = default;
};

#endif // EXPRESSIONCONTEXTUTILS_H
