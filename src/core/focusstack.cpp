/***************************************************************************
  focusstack.cpp

 ---------------------
 begin                : 11.4.2018
 copyright            : (C) 2018 by david
 email                : david at opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "focusstack.h"

void FocusStack::addFocusTaker( QQuickItem *item )
{
  connect( item, &QQuickItem::activeFocusChanged, this, &FocusStack::itemFocusChanged );
}

void FocusStack::itemFocusChanged( bool itemActiveFocus )
{
  if( itemActiveFocus )
  {
    setFocused( qobject_cast<QQuickItem*>( sender() ) );
  }
  else
  {
    setUnfocused( qobject_cast<QQuickItem*>( sender() ) );
  }
}

void FocusStack::setFocused( QQuickItem* item  )
{
  mStackList.removeAll( item );
  mStackList.append( item );
}

void FocusStack::setUnfocused( QQuickItem* item  )
{
  if( !item->isVisible() )
  {
    mStackList.removeAll( item );
    if( !mStackList.isEmpty() )
    {
      mStackList.last()->forceActiveFocus();
    }
  }
}
