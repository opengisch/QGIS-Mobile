/***************************************************************************
                            NavigationBar.qml
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


import QtQuick 2.0
import org.qgis 1.0
import "js/style.js" as Style

Rectangle {
  id: toolBar

  property string currentName: ''
  property bool showEditButtons
  property MultiFeatureListModel model
  property FeatureListModelSelection selection
  property FeaturelistExtentController extentController

  signal statusIndicatorClicked
  signal editAttributesButtonClicked
  signal editGeometryButtonClicked
  signal save
  signal cancel

  anchors.top:parent.top
  anchors.left: parent.left
  anchors.right: parent.right
  height: 48*dp

  clip: true

  states: [
    State {
      name: "Navigation"
    },
    State {
      name: "Indication"
    },
    State {
      name: "Edit"
    }
  ]

  state: "Indication"

  Rectangle {
    id: navigationStatusIndicator
    anchors.fill: parent
    height: 48*dp

    color: featureFormList.model.constraintsValid || parent.state !== "Edit" ? "#80CC28" : "orange"

    clip: true

    focus: true

    Text {
      anchors.centerIn: parent

      text: {
        if ( model ) {
          ( selection.selection + 1 ) + '/' + model.count + ': ' + currentName
        }
        else {
          ''
        }
      }
    }

    MouseArea {
      anchors.fill: parent

      onClicked: {
        toolBar.statusIndicatorClicked()
      }
    }
  }

  Button {
    id: nextButton

    anchors.right: parent.right

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: Style.getThemeIcon( "ic_chevron_right_white_24dp" )

    enabled: ( toolBar.model && ( selection.selection + 1 ) < toolBar.model.count )

    onClicked: {
      selection.selection = selection.selection + 1
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: saveButton
    anchors.left: parent.left
    width: ( parent.state == "Edit" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: Style.getThemeIcon( "ic_check_white_48dp" )
    onClicked: {
     if( featureFormList.model.constraintsValid ) {
       toolBar.save()
     } else {
       displayToast( "Constraints not valid" )
     }
    }
    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: cancelButton

    anchors.right: parent.right

    width: ( parent.state == "Edit" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: Style.getThemeIcon( "ic_clear_white_24dp" )

    onClicked: {
      selection.selectionChanged()
      toolBar.cancel()
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  Button {
    id: editGeomButton

    property bool readOnly: false

    visible: stateMachine.state === "digitize"

    anchors.right: editButton.left

    iconSource: Style.getThemeIcon( "ic_create_white_24dp" )

    width: ( parent.state == "Navigation" && !readOnly ? 48*dp : 0 )
    height: 48*dp
    clip: true

    onClicked: {
      extentController.zoomToSelected(true)
      toolBar.editGeometryButtonClicked()
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }

    Connections {
      target: selection

      onSelectionChanged:
      {
        editGeomButton.readOnly = selection.selectedLayer.readOnly
      }
    }
  }

  Button {
    id: editButton

    property bool readOnly: false

    anchors.right: nextButton.left

    width: ( parent.state == "Navigation" && !readOnly ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: Style.getThemeIcon( "ic_create_white_24dp" )

    onClicked: {
      toolBar.editAttributesButtonClicked()
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }

    Connections {
      target: selection

      onSelectionChanged:
      {
        editButton.readOnly = selection.selectedLayer.readOnly
      }
    }
  }

  Button {
    id: followCurrentButton

    anchors.left: previousButton.right

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true
    checkable: true
    checked: extentController.autoZoom

    iconSource: Style.getThemeIcon( "ic_fullscreen_white_24dp" )

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }

    MouseArea {
      anchors.fill: parent

      onClicked: {
        extentController.zoomToSelected()
      }

      onPressAndHold: {
        extentController.autoZoom = !extentController.autoZoom
      }
    }
  }

  Button {
    id: previousButton

    anchors.left: parent.left

    width: ( parent.state == "Navigation" ? 48*dp : 0 )
    height: 48*dp
    clip: true

    iconSource: Style.getThemeIcon( "ic_chevron_left_white_24dp" )

    enabled: ( selection.selection > 0 )

    onClicked: {
      selection.selection = selection.selection - 1
    }

    Behavior on width {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }
}
