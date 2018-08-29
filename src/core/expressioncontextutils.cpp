/***************************************************************************
                            expressioncontextutils.cpp
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


#include "expressioncontextutils.h"
#include "qgsgeometry.h"
#include <QtPositioning/QGeoPositionInfoSource>

QgsExpressionContextScope* ExpressionContextUtils::positionScope( QGeoPositionInfoSource *source )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Position" ) );
  QGeoPositionInfo positionInfo = source->lastKnownPosition();

  const QGeoCoordinate geoCoord = positionInfo.coordinate();
  const QgsGeometry point = QgsGeometry( new QgsPoint( geoCoord.longitude(), geoCoord.latitude(), geoCoord.altitude() ) );
  const QDateTime timestamp = positionInfo.timestamp();
  const qreal direction = positionInfo.attribute( QGeoPositionInfo::Attribute::Direction );
  const qreal groundSpeed = positionInfo.attribute( QGeoPositionInfo::Attribute::GroundSpeed );
  const qreal magneticVariation = positionInfo.attribute( QGeoPositionInfo::Attribute::MagneticVariation );
  const qreal horizontalAccuracy = positionInfo.attribute( QGeoPositionInfo::Attribute::HorizontalAccuracy );
  const qreal verticalAccuracy = positionInfo.attribute( QGeoPositionInfo::Attribute::VerticalAccuracy );
  const qreal verticalSpeed = positionInfo.attribute( QGeoPositionInfo::Attribute::VerticalSpeed );

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_coordinate" ), QVariant::fromValue<QgsGeometry>( point ), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_timestamp" ), timestamp, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_direction" ), direction, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_ground_speed" ), groundSpeed, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_magnetic_variation" ), magneticVariation, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_horizontal_accuracy" ), horizontalAccuracy, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_vertical_accuracy" ), verticalAccuracy, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_vertical_speed" ), verticalSpeed, true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "position_source_name" ), source->sourceName(), true, true ) );

  return scope;
}
