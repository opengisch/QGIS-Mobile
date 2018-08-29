/***************************************************************************
                            FeatureForm.qml
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

import QtQuick 2.3
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import org.qgis 1.0
import org.qfield 1.0
import "js/style.js" as Style

Rectangle {
  id: featureForm

  property FeatureListModelSelection selection
  property MapSettings mapSettings
  property color selectionColor
  property alias model: globalFeaturesList.model
  property bool allowEdit
  property int formViewWidthDivisor

  signal showMessage(string message)
  signal editGeometry

  states: [
    State {
      name: "Hidden"
      StateChangeScript {
        script: {
          hide()
          if( featureFormList.state === "Edit" ){
            //e.g. tip on the canvas during an edit
            featureFormList.save()
          }
        }
      }
    },
    /* Shows a list of features */
    State {
      name: "FeatureList"
      PropertyChanges {
        target: globalFeaturesList
        shown: true

      }
      PropertyChanges {
        target: featureListToolBar
        state: "Indication"
      }
      StateChangeScript {
        script: {
          show()
          if( featureFormList.state === "Edit" ){
            ///e.g. tip on the canvas during an edit
            featureFormList.save()
          }
        }
      }
    },
    /* Shows the form for the currently selected feature */
    State {
      name: "FeatureForm"
      PropertyChanges {
        target: globalFeaturesList
        shown: false
      }
      PropertyChanges {
        target: featureListToolBar
        state: "Navigation"
      }
      PropertyChanges {
        target: featureFormList
        state: "ReadOnly"

      }
    },
    /* Shows an editable form for the currently selected feature */
    State {
      name: "FeatureFormEdit"
      PropertyChanges {
        target: featureListToolBar
        state: "Edit"
      }
      PropertyChanges {
        target: featureFormList
        state: "Edit"
      }
    }

  ]
  state: "Hidden"

  clip: true

  width: props.isVisible ? state == "FeatureList" ? parent.width / 3 : parent.width / formViewWidthDivisor : 0

  QtObject {
    id: props

    property bool isVisible: false
  }

  ListView {
    id: globalFeaturesList

    anchors.top: featureListToolBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: parent.height - featureListToolBar.height

    property bool shown: false

    clip: true

    section.property: "layerName"
    section.labelPositioning: ViewSection.CurrentLabelAtStart | ViewSection.InlineLabels
    section.delegate: Component {
      /* section header: layer name */
      Rectangle {
        width: parent.width
        height: 30*dp
        color: "lightGray"

        Text {
          anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
          font.bold: true
          text: section
        }
      }
    }

    delegate: Item {
      anchors { left: parent.left; right: parent.right }

      focus: true

      height: Math.max( 48*dp, featureText.height )

      Text {
        id: featureText
        anchors { leftMargin: 10; left: parent.left; right: deleteButton.left; verticalCenter: parent.verticalCenter }
        font.bold: true
        text: display
      }

      Rectangle {
        anchors.left: parent.left
        height: parent.height
        width: 6
        color: featureForm.selectionColor
        opacity: ( index == featureForm.selection.selection )
        Behavior on opacity {
          PropertyAnimation {
            easing.type: Easing.InQuart
          }
        }
      }

      MouseArea {
        anchors.fill: parent

        onClicked: {
          featureForm.selection.selection = index
          featureForm.state = "FeatureForm"
        }

        onPressAndHold:
        {
          featureForm.selection.selection = index
        }
      }

      Row
      {
        id: editRow
        anchors { top: parent.top; right: parent.right }

        Button {
          id: deleteButton

          width: 48*dp
          height: 48*dp

          visible: deleteFeatureCapability && allowEdit

          iconSource: Style.getThemeIcon( "ic_delete_forever_white_24dp" )

          onClicked: {
            deleteDialog.currentLayer = currentLayer
            deleteDialog.featureId = featureId
            deleteDialog.visible = true
          }
        }
      }

      /* bottom border */
      Rectangle {
        anchors.bottom: parent.bottom
        height: 1
        color: "lightGray"
        width: parent.width
      }
    }

    /* bottom border */
    Rectangle {
      anchors.bottom: parent.bottom
      height: 1
      color: "lightGray"
      width: parent.width
    }

    onShownChanged: {
      if ( shown )
      {
        height = parent.height - featureListToolBar.height
      }
      else
      {
        height = 0
      }
    }

    Behavior on height {
      PropertyAnimation {
        easing.type: Easing.InQuart
      }
    }
  }

  FeatureForm {
    id: featureFormList

    anchors.top: featureListToolBar.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: parent.height - globalFeaturesList.height

    model: AttributeFormModel {
      featureModel: FeatureModel {
        currentLayer: featureForm.selection.selectedLayer
        feature: featureForm.selection.selectedFeature
      }
    }

    focus: true

    visible: !globalFeaturesList.shown

  }

  NavigationBar {
    id: featureListToolBar
    model: globalFeaturesList.model
    selection: featureForm.selection
    extentController: FeaturelistExtentController {
      model: globalFeaturesList.model
      selection: featureForm.selection
      mapSettings: featureForm.mapSettings
    }

    onStatusIndicatorClicked: {
      featureForm.state = "FeatureList"
    }

    onEditAttributesButtonClicked: {
      featureForm.state = "FeatureFormEdit"
    }

    onEditGeometryButtonClicked: {
      editGeometry()
    }

    onSave: {
      featureFormList.save()
      featureForm.state = "FeatureForm"
      displayToast( qsTr( "Changes saved" ) )
    }

    onCancel: {
      featureFormList.model.featureModel.reset()
      featureForm.state = "FeatureForm"
      displayToast( qsTr( "Changes discarded" ) )
    }
  }

  Keys.onReleased: {
    if ( event.key === Qt.Key_Back ||
        event.key === Qt.Key_Escape ) {
      if( state != "FeatureList" ) {
        if( featureListToolBar.state === "Edit"){
          if( featureFormList.model.constraintsValid ) {
            featureListToolBar.save()
          } else {
            displayToast( "Constraints not valid" )
          }
        }else{
          state = "FeatureList"
        }
      }else{
        state = "Hidden"
      }
      event.accepted = true
    }
  }

  Behavior on width {
    PropertyAnimation {
      easing.type: Easing.InQuart

      onRunningChanged: {
        if ( running )
          mapCanvasMap.freeze('formresize')
        else
          mapCanvasMap.unfreeze('formresize')
      }
    }
  }

  Connections {
    target: globalFeaturesList.model

    onRowsInserted: {
      if ( model.rowCount() > 0 ) {
        state = "FeatureList"
      } else {
        showMessage( qsTr('No feature at this position') )
        state = "Hidden"
      }
    }
  }

  function show()
  {
    props.isVisible = true
    focus = true
  }

  function hide()
  {
    props.isVisible = false
    focus = false
    model.clear()
  }

  MessageDialog {
    id: deleteDialog

    property int featureId
    property VectorLayer currentLayer

    visible: false

    title: qsTr( "Delete feature" )
    text: qsTr( "Should this feature really be deleted?" )
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    onAccepted: {
      featureForm.model.deleteFeature( currentLayer, featureId )
      visible = false
    }
    onRejected: {
      visible = false
    }
  }
}
