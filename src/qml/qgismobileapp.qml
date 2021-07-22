/***************************************************************************
                            qgismobileapp.qml
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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.0 as LabSettings
import QtQml 2.12
import QtPositioning 5.12
import QtSensors 5.12

import org.qgis 1.0
import org.qfield 1.0
import Theme 1.0

import '.'
import 'geometry_editors'

ApplicationWindow {
  id: mainWindow
  objectName: 'mainWindow'
  visible: true

  LabSettings.Settings {
      property alias x: mainWindow.x
      property alias y: mainWindow.y
      property alias width: mainWindow.width
      property alias height: mainWindow.height
  }

  FocusStack{
      id: focusstack
  }

  QuestionDialog{
    id: questionDialog
    parent: ApplicationWindow.overlay
  }

  //this keyHandler is because otherwise the back-key is not handled in the mainWindow. Probably this could be solved cuter.
  Item {
    id: keyHandler
    objectName: "keyHandler"

    visible: true
    focus: true

    Keys.onReleased: {
      if ( event.key === Qt.Key_Back || event.key === Qt.Key_Escape ) {
        if ( featureForm.visible ) {
            featureForm.hide();
        } else if ( stateMachine.state === 'measure' ) {
          mainWindow.closeMeasureTool()
        } else {
          mainWindow.close();
        }
        event.accepted = true
      }
    }

    Component.onCompleted: focusstack.addFocusTaker( this )
  }

  //currentRubberband provides the rubberband depending on the current state (digitize or measure)
  property Rubberband currentRubberband
  property LayerObserver layerObserverAlias: layerObserver
  property QgsGpkgFlusher gpkgFlusherAlias: gpkgFlusher

  signal closeMeasureTool()
  signal changeMode( string mode )

  Item {
    id: stateMachine

    property string lastState

    states: [
      State {
        name: "browse"
        PropertyChanges { target: identifyTool; deactivated: false }
      },

      State {
        name: "digitize"
        PropertyChanges { target: identifyTool; deactivated: false }
        PropertyChanges { target: mainWindow; currentRubberband: digitizingRubberband }
      },

      State {
        name: 'measure'
        PropertyChanges { target: identifyTool; deactivated: true }
        PropertyChanges { target: mainWindow; currentRubberband: measuringRubberband }
        PropertyChanges { target: featureForm; state: "Hidden" }
      }
    ]
    state: "browse"
  }

  onChangeMode: {
    if ( stateMachine.state == mode )
      return

    stateMachine.lastState = stateMachine.state
    stateMachine.state = mode
    switch ( stateMachine.state )
    {
      case 'browse':
        displayToast( qsTr( 'You are now in browse mode' ) );
        break;
      case 'digitize':
        dashBoard.ensureEditableLayerSelected();
        if (dashBoard.currentLayer)
        {
          displayToast( qsTr( 'You are now in digitize mode on layer %1' ).arg( dashBoard.currentLayer.name ) );
        }
        else
        {
          displayToast( qsTr( 'You are now in digitize mode' ) );
        }
        break;
      case 'measure':
        displayToast( qsTr( 'You are now in measure mode' ) );
        break;
    }
  }

  onCloseMeasureTool: {
    overlayFeatureFormDrawer.close()
    changeMode( stateMachine.lastState)
  }

  /**
   * The position source to access the GPS
   */
  TransformedPositionSource {
    id: positionSource
    destinationCrs: mapCanvas.mapSettings.destinationCrs
    deltaZ: positioningSettings.antennaHeightActivated ? positioningSettings.antennaHeight * -1 : 0
    skipAltitudeTransformation: positioningSettings.skipAltitudeCorrection
    device: positioningSettings.positioningDevice
  }

  Item {
    /*
     * This is the map canvas
     * On top of it are the base map and other items like GPS icon...
     */
    id: mapCanvas
    clip: true
    property bool isBeingTouched: false

    DragHandler {
        id: freehandHandler
        property bool isDigitizing: false
        enabled: freehandButton.visible && freehandButton.freehandDigitizing && !digitizingToolbar.rubberbandModel.frozen && !featureForm.visible
        acceptedDevices: !qfieldSettings.mouseAsTouchScreen ? PointerDevice.Stylus | PointerDevice.Mouse : PointerDevice.Stylus
        grabPermissions: PointerHandler.CanTakeOverFromHandlersOfSameType | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything

        onActiveChanged: {
            if (!active) {
                var screenLocation = centroid.position;
                var screenFraction = settings.value( "/QField/Digitizing/FreehandRecenterScreenFraction", 5 );
                var threshold = Math.min( mainWindow.width, mainWindow.height ) / screenFraction;
                if ( screenLocation.x < threshold || screenLocation.x > mainWindow.width - threshold ||
                        screenLocation.y < threshold || screenLocation.y > mainWindow.height - threshold )
                {
                    mapCanvas.mapSettings.setCenter(mapCanvas.mapSettings.screenToCoordinate(screenLocation));
                }
            }
        }

        onCentroidChanged: {
            if (active) {
                if (geometryEditorsToolbar.canvasClicked(centroid.position)) {
                    // needed to handle freehand digitizing of rings
                } else {
                    digitizingToolbar.addVertex();
                }
            }
        }
    }

    HoverHandler {
        id: hoverHandler
        enabled: !qfieldSettings.mouseAsTouchScreen
                 && !gpsLinkButton.linkActive
                 && !parent.isBeingTouched
                 && (!digitizingToolbar.rubberbandModel || !digitizingToolbar.rubberbandModel.frozen)
        acceptedDevices: PointerDevice.Stylus | PointerDevice.Mouse
        grabPermissions: PointerHandler.TakeOverForbidden

        onPointChanged: {
            function pointInItem(point, item) {
                var itemCoordinates = item.mapToItem(mainWindow.contentItem, 0, 0);
                return point.position.x >= itemCoordinates.x && point.position.x <= itemCoordinates.x + item.width &&
                       point.position.y >= itemCoordinates.y && point.position.y <= itemCoordinates.y + item.height;
            }
            // when hovering digitizing toolbars, reset coordinate locator position for nicer UX
            if ( !freehandHandler.active && pointInItem( point, digitizingToolbar ) ) {
                coordinateLocator.sourceLocation = mapCanvas.mapSettings.coordinateToScreen( digitizingToolbar.rubberbandModel.lastCoordinate );
            } else if ( !freehandHandler.active && pointInItem( point, geometryEditorsToolbar ) ) {
                coordinateLocator.sourceLocation = mapCanvas.mapSettings.coordinateToScreen( geometryEditorsToolbar.editorRubberbandModel.lastCoordinate );
            } else {
                // after a click, it seems that the position is sent once at 0,0 => weird
                if (point.position !== Qt.point(0, 0))
                    coordinateLocator.sourceLocation = point.position
            }
        }

        onActiveChanged: {
            if ( !active )
                coordinateLocator.sourceLocation = undefined

        }

        onHoveredChanged: {
            if ( !hovered )
                coordinateLocator.sourceLocation = undefined
        }
    }
    Timer {
        id: resetIsBeingTouchedTimer
        interval: 750
        repeat: false

        onTriggered: {
            parent.isBeingTouched = false
        }
    }
    /* The second hover handler is a workaround what appears to be an issue with
     * Qt whereas synthesized mouse event would trigger the first HoverHandler even though
     * PointerDevice.TouchScreen was explicitly taken out of the accepted devices.
     */
    HoverHandler {
        id: dummyHoverHandler
        enabled: !qfieldSettings.mouseAsTouchScreen
        acceptedDevices: PointerDevice.TouchScreen
        grabPermissions: PointerHandler.TakeOverForbidden

        onHoveredChanged: {
            if ( hovered ) {
                parent.isBeingTouched = true
                resetIsBeingTouchedTimer.stop()
            }
            else {
                resetIsBeingTouchedTimer.restart()
            }
        }
    }

    /* Initialize a MapSettings object. This will contain information about
     * the current canvas extent. It is shared between the base map and all
     * map canvas items and is used to transform map coordinates to pixel
     * coordinates.
     * It may change any time and items that hold a reference to this property
     * are responsible to handle this properly.
     */
    property MapSettings mapSettings: mapCanvasMap.mapSettings

    /* Placement and size. Share right anchor with featureForm */
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: positionInformationView.visible ? positionInformationView.top : parent.bottom

    Rectangle {
      id: mapCanvasBackground
      anchors.fill: parent
      color: mapCanvas.mapSettings.backgroundColor
    }

    /* The base map */
    MapCanvas {
      id: mapCanvasMap
      incrementalRendering: true
      freehandDigitizing: freehandButton.freehandDigitizing && freehandHandler.active

      anchors.fill: parent

      onClicked:  {
          if (locatorItem.state == "on")
          {
              locatorItem.state = "off"
          }
          else if ( type === "stylus" )
          {
              // Check if geometry editor is taking over
              if ( !gpsLinkButton.linkActive && geometryEditorsToolbar.canvasClicked(point) )
                  return;

              if ( !gpsLinkButton.linkActive && !featureForm.visible &&
                   ( ( stateMachine.state === "digitize" && digitizingFeature.currentLayer ) || stateMachine.state === 'measure' ) )
              {
                  if ( Number( currentRubberband.model.geometryType ) === QgsWkbTypes.PointGeometry ||
                          Number( currentRubberband.model.geometryType ) === QgsWkbTypes.NullGeometry )
                  {
                      digitizingToolbar.confirm()
                  }
                  else
                  {
                      digitizingToolbar.addVertex()
                  }
              }
              else
              {
                  if( !overlayFeatureFormDrawer.visible )
                  {
                      identifyTool.identify(point)
                  }
              }
          }
      }

      onConfirmedClicked: {
          if( !digitizingToolbar.geometryRequested && !overlayFeatureFormDrawer.visible )
          {
              identifyTool.identify(point)
          }
      }

      onLongPressed: {
        if ( type === "stylus" ){
          if (geometryEditorsToolbar.canvasLongPressed(point)) {
            // for instance, the vertex editor will select a vertex if possible
            return
          }
          if ( stateMachine.state === "digitize" && dashBoard.currentLayer ) { // the sourceLocation test checks if a (stylus) hover is active
            if ( ( Number( currentRubberband.model.geometryType ) === QgsWkbTypes.LineGeometry && currentRubberband.model.vertexCount >= 2 )
               || ( Number( currentRubberband.model.geometryType ) === QgsWkbTypes.PolygonGeometry && currentRubberband.model.vertexCount >= 2 ) ) {
                digitizingToolbar.addVertex();

                // When it's released, it will normally cause a release event to close the attribute form.
                // We get around this by temporarily switching the closePolicy.
                overlayFeatureFormDrawer.closePolicy = Popup.CloseOnEscape

                digitizingToolbar.confirm()
                return
            }
          }
          // do not use else, as if it was catch it has return before
          if( !overlayFeatureFormDrawer.visible ) {
            identifyTool.identify(point)
          }
        }
      }

      onLongPressReleased: {
        if ( type === "stylus" ){
          // The user has released the long press. We can re-enable the default close behavior for the feature form.
          // The next press will be intentional to close the form.
          overlayFeatureFormDrawer.closePolicy = Popup.CloseOnEscape | Popup.CloseOnPressOutside
        }
      }

      Component.onCompleted: platformUtilities.showRateThisApp()
    }


  /**************************************************
   * Position markers
   * - Coordinate Locator
   * - GPS Marker
   * - Identify Highlight
   * - Digitizing Rubberband
   **************************************************/

    /** The identify tool **/
    IdentifyTool {
      id: identifyTool

      mapSettings: mapCanvas.mapSettings
      model: featureForm.model
      searchRadiusMm: 3
    }

    /** A rubberband for measuring **/
    Rubberband {
      id: measuringRubberband
      width: 2
      color: '#80000000'

      mapSettings: mapCanvas.mapSettings

      model: RubberbandModel {
        frozen: false
        currentCoordinate: coordinateLocator.currentCoordinate
        geometryType: QgsWkbTypes.PolygonGeometry
        crs: mapCanvas.mapSettings.destinationCrs
      }

      anchors.fill: parent

      visible: stateMachine.state === 'measure'
    }

    /** Tracking sessions **/
    Repeater {
        id: trackings
        model: trackingModel
        Tracking {
        }
    }

    /** A rubberband for ditizing **/
    Rubberband {
      id: digitizingRubberband
      width: 2

      mapSettings: mapCanvas.mapSettings

      model: RubberbandModel {
        frozen: false
        currentCoordinate: coordinateLocator.currentCoordinate
        vectorLayer: digitizingToolbar.geometryRequested ? digitizingToolbar.geometryRequestedLayer : dashBoard.currentLayer
        crs: mapCanvas.mapSettings.destinationCrs
      }

      anchors.fill: parent

      visible: stateMachine.state === "digitize"
    }

    /** A rubberband for the different geometry editors **/
    Rubberband {
      id: geometryEditorsRubberband
      width: 2
      color: '#80000000'

      mapSettings: mapCanvas.mapSettings

      model: RubberbandModel {
        frozen: false
        currentCoordinate: coordinateLocator.currentCoordinate
        crs: mapCanvas.mapSettings.destinationCrs
        geometryType: QgsWkbTypes.LineGeometry
      }

      anchors.fill: parent
    }

    /** A coordinate locator for digitizing **/
    CoordinateLocator {
      id: coordinateLocator
      anchors.fill: parent
      visible: stateMachine.state === "digitize" || stateMachine.state === 'measure'
      highlightColor: digitizingToolbar.isDigitizing ? currentRubberband.color : "#CFD8DC"
      mapSettings: mapCanvas.mapSettings
      currentLayer: dashBoard.currentLayer
      positionInformation: positionSource.positionInfo
      overrideLocation: gpsLinkButton.linkActive ? positionSource.projectedPosition : undefined
    }

    /* GPS marker  */
    LocationMarker {
      id: locationMarker
      mapSettings: mapCanvas.mapSettings
      anchors.fill: parent
      visible: positionSource.active && positionSource.positionInfo && positionSource.positionInfo.latitudeValid
      location: positionSource.projectedPosition
      accuracy: positionSource.projectedHorizontalAccuracy
      direction: positionSource.positionInfo && positionSource.positionInfo.directionValid ? positionSource.positionInfo.direction : -1

      onLocationChanged: {
        if ( gpsButton.followActive ) {
          var screenLocation = mapSettings.coordinateToScreen( location );
          var screenFraction = settings.value( "/QField/Positioning/FollowScreenFraction", 5 );
          var threshold = Math.min( mainWindow.width, mainWindow.height ) / screenFraction;
          if ( screenLocation.x < threshold || screenLocation.x > mainWindow.width - threshold ||
               screenLocation.y < threshold || screenLocation.y > mainWindow.height - threshold )
          {
              gpsButton.followActiveSkipExtentChanged = true;
              mapCanvas.mapSettings.setCenter(positionSource.projectedPosition);
          }
        }
      }
    }

    /* Rubberband for vertices  */
    Item {
      // highlighting vertices
      VertexRubberband {
        id: vertexRubberband
        model: geometryEditingFeature.vertexModel
        mapSettings: mapCanvas.mapSettings
      }

      // highlighting geometry (point, line, surface)
      Rubberband {
        id: editingRubberband
        vertexModel: vertexModel
        mapSettings: mapCanvas.mapSettings
        width: 4
      }
    }

    /* Locator Highlight */
    GeometryHighlighter {
      id: locatorHighlightItem
      //width: 10
      //color: "yellow"
    }

    /* Highlight the currently selected item on the feature list */
    FeatureListSelectionHighlight {
      selectionModel: featureForm.selection
      mapSettings: mapCanvas.mapSettings

      //model: featureForm.model
      //selection: featureForm.selection

      color: "yellow"
      focusedColor: "#ff7777"
      selectedColor: Theme.mainColor
      width: 5
    }
  }

  Item {
    id: positionInformationView
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    visible: positioningSettings.showPositionInformation

    height: childrenRect.height
    width: parent.width

    PositionInformationView {
      id: p
      positionSource: positionSource
      antennaHeight: positioningSettings.antennaHeightActivated ? positioningSettings.antennaHeight : NaN
    }
  }

  DropShadow {
    anchors.fill: positionInformationView
    visible: positionInformationView.visible
    verticalOffset: -2
    radius: 6.0
    samples: 17
    color: "#30000000"
    source: positionInformationView
  }

  /**************************************************
   * Map Canvas Decorations like
   * - Position Information View
   * - Scale Bar
   **************************************************/

  Text {
    DistanceArea {
      id: digitizingGeometryMeasure

      property VectorLayer currentLayer: dashBoard.currentLayer

      rubberbandModel: measuringRubberband.model
      project: qgisProject
      crs: qgisProject.crs
    }

    // The position is dynamically calculated to follow the coordinate locator
    x: {
        var newX = coordinateLocator.displayPosition.x + 20;
        if (newX + width > mapCanvas.x + mapCanvas.width)
            newX -= width + 40;
        return newX;
    }
    y: {
        var newY = coordinateLocator.displayPosition.y + 10
        if (newY + height > mapCanvas.y + mapCanvas.height)
            newY -= height - 20;
        return newY;
    }

    text: ( qfieldSettings.numericalDigitizingInformation && stateMachine.state === "digitize" ) || stateMachine.state === 'measure' ?
              '%1%2%3%4'
                .arg(stateMachine.state === 'digitize' || !digitizingToolbar.isDigitizing ? '<p>%1: %2<br>%3: %4</p>'
                  .arg(coordinateLocator.mapSettings.destinationCrs.isGeographic ? qsTr( 'Lon' ) : 'X')
                  .arg(coordinateLocator.currentCoordinate.x.toFixed( coordinateLocator.mapSettings.destinationCrs.isGeographic ? 5 : 2 ))
                  .arg(coordinateLocator.mapSettings.destinationCrs.isGeographic ? qsTr( 'Lat' ) : 'Y')
                  .arg(coordinateLocator.currentCoordinate.y.toFixed( coordinateLocator.mapSettings.destinationCrs.isGeographic ? 5 : 2 ))
                  : '' )

                .arg(digitizingGeometryMeasure.lengthValid ? '<p>%1: %2%3</p>'
                  .arg( digitizingGeometryMeasure.segmentLength != digitizingGeometryMeasure.length ? qsTr( 'Segment') : qsTr( 'Length') )
                  .arg(UnitTypes.formatDistance( digitizingGeometryMeasure.segmentLength, 3, digitizingGeometryMeasure.lengthUnits ) )
                  .arg(digitizingGeometryMeasure.length !== -1 && digitizingGeometryMeasure.segmentLength != digitizingGeometryMeasure.length ? '<br>%1: %2'.arg( qsTr( 'Length') ).arg(UnitTypes.formatDistance( digitizingGeometryMeasure.length, 3, digitizingGeometryMeasure.lengthUnits ) ) : '' )
                  : '' )

                .arg(digitizingGeometryMeasure.areaValid ? '<p>%1: %2</p>'
                  .arg( qsTr( 'Area') )
                  .arg(UnitTypes.formatArea( digitizingGeometryMeasure.area, 3, digitizingGeometryMeasure.areaUnits ) )
                  : '' )

                .arg(stateMachine.state === 'measure' && digitizingToolbar.isDigitizing? '<p>%1: %2<br>%3: %4</p>'
                  .arg(coordinateLocator.mapSettings.destinationCrs.isGeographic ? qsTr( 'Lon' ) : 'X')
                  .arg(coordinateLocator.currentCoordinate.x.toFixed( coordinateLocator.mapSettings.destinationCrs.isGeographic ? 5 : 2 ))
                  .arg(coordinateLocator.mapSettings.destinationCrs.isGeographic ? qsTr( 'Lat' ) : 'Y')
                  .arg(coordinateLocator.currentCoordinate.y.toFixed( coordinateLocator.mapSettings.destinationCrs.isGeographic ? 5 : 2 ))
                  : '' )
              : ''

    font: Theme.strongTipFont
    style: Text.Outline
    styleColor: Theme.light
  }

  ScaleBar {
    visible: qfieldSettings.showScaleBar
    mapSettings: mapCanvas.mapSettings

    anchors.left: mapCanvas.left
    anchors.bottom: mapCanvas.bottom
    anchors.margins: 10
  }

  DropShadow {
    anchors.fill: featureForm
    horizontalOffset: mainWindow.width >= mainWindow.height ? -2: 0
    verticalOffset: mainWindow.width < mainWindow.height ? -2: 0
    radius: 6.0
    samples: 17
    color: "#80000000"
    source: featureForm
  }

  QfToolButton {
    id: alertIcon
    iconSource: Theme.getThemeIcon( "ic_alert_black_24dp" )
    round: true
    bgcolor: "transparent"

    visible: messageLog.unreadMessages

    anchors.right: locatorItem.right
    anchors.top: locatorItem.top
    anchors.topMargin: 52

    onClicked: messageLog.visible = true
  }

  Column {
    id: zoomToolbar
    anchors.right: mapCanvas.right
    anchors.rightMargin: 4
    anchors.bottom: mapCanvas.bottom
    anchors.bottomMargin: ( mapCanvas.height - zoomToolbar.height / 2 ) / 2
    spacing: 4

    QfToolButton {
      id: zoomInButton
      round: true
      anchors.right: parent.right

      bgcolor: Theme.darkGray
      iconSource: Theme.getThemeIcon( "ic_add_white_24dp" )

      transform: Scale {
          origin.x: zoomInButton.width / 1.5
          origin.y: zoomInButton.height / 1.25
          xScale: 0.75
          yScale: 0.75
      }

      onClicked: {
          if ( gpsButton.followActive ) gpsButton.followActiveSkipExtentChanged = true;
          mapCanvasMap.zoomIn(Qt.point(mapCanvas.x + mapCanvas.width / 2,mapCanvas.y + mapCanvas.height / 2));
      }
    }
    QfToolButton {
      id: zoomOutButton
      round: true
      anchors.right: parent.right

      bgcolor: Theme.darkGray
      iconSource: Theme.getThemeIcon( "ic_remove_white_24dp" )

      transform: Scale {
          origin.x: zoomOutButton.width / 1.5
          origin.y: zoomOutButton.height / 1.75
          xScale: 0.75
          yScale: 0.75
      }

      onClicked: {
          if ( gpsButton.followActive ) gpsButton.followActiveSkipExtentChanged = true;
          mapCanvasMap.zoomOut(Qt.point(mapCanvas.x + mapCanvas.width / 2,mapCanvas.y + mapCanvas.height / 2));
      }
    }
  }

  LocatorItem {
    id: locatorItem

    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: 4

    visible: stateMachine.state !== 'measure'

    Keys.onReleased: {
      if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
        event.accepted = true
        state = "off"
      }
    }

    onStateChanged: {
      if ( state == "off" ) {
        focus = false
        if ( featureForm.visible ) {
          featureForm.focus = true
        } else {
          keyHandler.focus = true
        }
      }
    }
  }

  LocatorSettings {
      id: locatorSettings
      locatorModelSuperBridge: locatorItem.locatorModelSuperBridge

      modal: true
      closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
      parent: ApplicationWindow.overlay
  }

  DropShadow {
    anchors.fill: locatorItem
    visible: locatorItem.searchFieldVisible
    verticalOffset: 2
    radius: 10
    samples: 17
    color: "#66212121"
    source: locatorItem
  }

  Image {
    Magnetometer {
      id: mag
      active: true
      returnGeoValues: false
    }
    source: Theme.getThemeVectorIcon( 'north_arrow' )
    x: (root.width - width)/2
    y: (root.height - height)/2
    rotation: ((Math.atan2(mag.x, mag.y) / Math.PI) * 180)
  }

  DashBoard {
    id: dashBoard
    allowLayerChange: !digitizingToolbar.isDigitizing
    mapSettings: mapCanvas.mapSettings
    interactive: !welcomeScreen.visible

    onOpenedChanged: {
      if ( !opened ) {
        if ( featureForm.visible ) {
          featureForm.focus = true;
        }
      }
    }

    function ensureEditableLayerSelected() {
      var firstEditableLayer = null;
      var currentLayerLocked = false;
      for (var i = 0; layerTree.rowCount(); i++)
      {
        var index = layerTree.index(i,0)
        if (firstEditableLayer === null)
        {
          if (layerTree.data(index,FlatLayerTreeModel.Type) === 'layer' && layerTree.data(index, FlatLayerTreeModel.ReadOnly) === false && layerTree.data(index, FlatLayerTreeModel.GeometryLocked) === false)
          {
             firstEditableLayer = layerTree.data(index, FlatLayerTreeModel.VectorLayerPointer);
          }
        }
        if (currentLayer != null && currentLayer === layerTree.data(index, FlatLayerTreeModel.VectorLayerPointer))
        {
           if (layerTree.data(index, FlatLayerTreeModel.ReadOnly) === true || layerTree.data(index, FlatLayerTreeModel.GeometryLocked) === true)
           {
             currentLayerLocked = true;
           }
           else
           {
             break;
           }
        }
        if (firstEditableLayer !== null && (currentLayer === null || currentLayerLocked === true))
        {
          currentLayer = firstEditableLayer;
          break;
        }
      }
    }
  }

  /* The main menu */
  Row {
    id: mainMenuBar
    width: childrenRect.width + 8
    height: childrenRect.height + 8
    topPadding: 4
    leftPadding: 4
    spacing: 4

    QfToolButton {
      id: menuButton
      round: true
      iconSource: Theme.getThemeIcon( "ic_menu_white_24dp" )
      onClicked: dashBoard.opened ? dashBoard.close() : dashBoard.open()
      bgcolor: dashBoard.opened ? Theme.mainColor : Theme.darkGray
    }

    CloseTool {
      id: closeMeasureTool
      visible: stateMachine.state === 'measure'
      toolText: qsTr( 'Close measure tool' )
      onClosedTool: mainWindow.closeMeasureTool()
    }

    CloseTool {
      id: closeGeometryEditorsTool
      visible: ( stateMachine.state === "digitize" && vertexModel.vertexCount > 0 )
      toolText: qsTr( 'Stop editing' )
      onClosedTool: geometryEditorsToolbar.cancelEditors()
    }

    CloseTool {
      id: abortRequestGeometry
      visible: digitizingToolbar.geometryRequested
      toolText: qsTr( 'Cancel addition' )
      onClosedTool: digitizingToolbar.cancel()
    }
  }

  Column {
    id: mainToolbar
    anchors.left: mainMenuBar.left
    anchors.top: mainMenuBar.bottom
    anchors.leftMargin: 4
    spacing: 4

    QfToolButton {
      id: topologyButton
      round: true
      visible: stateMachine.state === "digitize"
          && dashBoard.currentLayer
          && dashBoard.currentLayer.isValid
          && ( dashBoard.currentLayer.geometryType() === QgsWkbTypes.PolygonGeometry || dashBoard.currentLayer.geometryType() === QgsWkbTypes.LineGeometry )
      state: qgisProject && qgisProject.topologicalEditing ? "On" : "Off"
      iconSource: Theme.getThemeIcon( "ic_topology_white_24dp" )

      bgcolor: Theme.darkGray

      states: [
        State {

          name: "Off"
          PropertyChanges {
            target: topologyButton
            iconSource: Theme.getThemeIcon( "ic_topology_white_24dp" )
            bgcolor: Qt.hsla(Theme.darkGray.hslHue, Theme.darkGray.hslSaturation, Theme.darkGray.hslLightness, 0.3)
          }
        },

        State {
          name: "On"
          PropertyChanges {
            target: topologyButton
            iconSource: Theme.getThemeIcon( "ic_topology_green_24dp" )
            bgcolor: Theme.darkGray
          }
        }
      ]

      onClicked: {
        qgisProject.topologicalEditing = !qgisProject.topologicalEditing;
        displayToast( qgisProject.topologicalEditing ? qsTr( "Topological editing turned on" ) : qsTr( "Topological editing turned off" ) );
      }
    }

    QfToolButton {
      id: freehandButton
      round: true
      visible: hoverHandler.hovered && !gpsLinkButton.linkActive && stateMachine.state === "digitize"
          && dashBoard.currentLayer
          && dashBoard.currentLayer.isValid
          && ( dashBoard.currentLayer.geometryType() === QgsWkbTypes.PolygonGeometry || dashBoard.currentLayer.geometryType() === QgsWkbTypes.LineGeometry )
      iconSource: Theme.getThemeIcon( "ic_freehand_white_24dp" )

      bgcolor: Theme.darkGray

      property bool freehandDigitizing: false
      state: freehandDigitizing ? "On" : "Off"

      states: [
        State {
          name: "Off"
          PropertyChanges {
            target: freehandButton
            iconSource: Theme.getThemeIcon( "ic_freehand_white_24dp" )
            bgcolor: Qt.hsla(Theme.darkGray.hslHue, Theme.darkGray.hslSaturation, Theme.darkGray.hslLightness, 0.3)
          }
        },

        State {
          name: "On"
          PropertyChanges {
            target: freehandButton
            iconSource: Theme.getThemeIcon( "ic_freehand_green_24dp" )
            bgcolor: Theme.darkGray
          }
        }
      ]

      onClicked: {
        freehandDigitizing = !freehandDigitizing
        displayToast( freehandDigitizing ? qsTr( "Freehand digitizing turned on" ) : qsTr( "Freehand digitizing turned off" ) );
        settings.setValue( "/QField/Digitizing/FreehandActive", freehandDigitizing );
      }

      Component.onCompleted: {
          freehandDigitizing = settings.valueBool( "/QField/Digitizing/FreehandActive", false )
      }
    }
  }

  Column {
    id: locationToolbar
    anchors.right: mapCanvas.right
    anchors.rightMargin: 4
    anchors.bottom: mapCanvas.bottom
    anchors.bottomMargin: 4
    spacing: 4

    QfToolButton {
      id: gpsLinkButton
      state: linkActive ? "On" : "Off"
      visible: gpsButton.state === "On" && ( stateMachine.state === "digitize" || stateMachine.state === 'measure' )
      round: true
      checkable: true
      anchors.right: parent.right

      readonly property bool linkActive: gpsButton.state === "On" && checked

      states: [
        State {
          name: "Off"
          PropertyChanges {
            target: gpsLinkButton
            iconSource: Theme.getThemeIcon( "ic_gps_link_white_24dp" )
            bgcolor: Theme.darkGraySemiOpaque
          }
        },

        State {
          name: "On"
          PropertyChanges {
            target: gpsLinkButton
            iconSource: Theme.getThemeIcon( "ic_gps_link_activated_white_24dp" )
            bgcolor: Theme.darkGray
          }
        }
      ]

      onCheckedChanged: {
        if (gpsButton.state === "On") {
            if (checked) {
                if (freehandButton.freehandDigitizing) {
                    // deactivate freehand digitizing when cursor locked is on
                    freehandButton.clicked();
                }
                displayToast( qsTr( "Coordinate cursor now locked to position" ) )
                if (positionSource.positionInfo.latitudeValid) {
                  var screenLocation = mapCanvas.mapSettings.coordinateToScreen(locationMarker.location);
                  if ( screenLocation.x < 0 || screenLocation.x > mainWindow.width ||
                       screenLocation.y < 0 || screenLocation.y > mainWindow.height )
                  {
                    mapCanvas.mapSettings.setCenter(positionSource.projectedPosition);
                  }
                }
            }
            else
            {
                displayToast( qsTr( "Coordinate cursor unlocked" ) )
            }
        }
      }
    }

    QfToolButton {
      id: gpsButton
      state: positionSource.active ? "On" : "Off"
      visible: positionSource.valid
      round: true

      anchors.right: parent.right

      onIconSourceChanged: {
        if( state === "On" ){
          if( positionSource.positionInfo && positionSource.positionInfo.latitudeValid ) {
            displayToast( qsTr( "Received position" ) )
          } else {
            displayToast( qsTr( "Searching for position" ) )
          }
        }
      }

      /*
      / When set to true, the map will follow the device's current position; the map
      / will stop following the position whe the user manually drag the map.
      */
      property bool followActive: false
      /*
      / When set to true, map canvas extent changes will not result in the
      / deactivation of the above followActive mode.
      */
      property bool followActiveSkipExtentChanged: false

      states: [
        State {
          name: "Off"
          PropertyChanges {
            target: gpsButton
            iconSource: Theme.getThemeIcon( "ic_location_disabled_white_24dp" )
            bgcolor: Theme.darkGraySemiOpaque
          }
        },

        State {
          name: "On"
          PropertyChanges {
            target: gpsButton
            iconSource: positionSource.positionInfo && positionSource.positionInfo.latitudeValid ? Theme.getThemeIcon( "ic_my_location_" + ( followActive ? "white" : "blue" ) + "_24dp" ) : Theme.getThemeIcon( "ic_gps_not_fixed_white_24dp" )
            bgcolor: followActive ? "#64B5F6" : Theme.darkGray
          }
        }
      ]

      onClicked: {
        followActive = true
        if ( positionSource.projectedPosition.x )
        {
          if ( !positionSource.active )
          {
            positioningSettings.positioningActivated = true
          }
          else
          {
              gpsButton.followActiveSkipExtentChanged = true;
              mapCanvas.mapSettings.setCenter(positionSource.projectedPosition)
              displayToast( qsTr( "Canvas follows location" ) )
          }
        }
        else
        {
          if ( positionSource.valid )
          {
            if ( positionSource.active )
            {
              displayToast( qsTr( "Waiting for location" ) )
            }
            else
            {
              positioningSettings.positioningActivated = true
            }
          }
        }
      }

      onPressAndHold: {
        gpsMenu.popup(locationToolbar.x + locationToolbar.width - gpsMenu.width, locationToolbar.y + locationToolbar.height - gpsMenu.height)
      }

      function toggleGps() {
        switch ( gpsButton.state )
        {
          case "Off":
            gpsButton.state = "On"
            displayToast( qsTr( "Positioning activated" ) )
            break;

          case "On":
            gpsButton.state = "Off"
            displayToast( qsTr( "Positioning turned off" ) )
            break;
        }
      }

      Rectangle {
          anchors {
              top: parent.top
              right: parent.right
              rightMargin: 2
              topMargin: 2
          }

          width: 12
          height: 12
          radius: width / 2

          border.width: 1.5
          border.color: 'white'

          visible: positioningSettings.accuracyIndicator && gpsButton.state === "On"
          color: !positionSource.positionInfo
                 || !positionSource.positionInfo.haccValid
                 || positionSource.positionInfo.hacc > positioningSettings.accuracyBad
                     ? Theme.errorColor
                     : positionSource.positionInfo.hacc > positioningSettings.accuracyExcellent
                       ? Theme.warningColor
                       : Theme.mainColor
      }
    }

    Connections {
        target: mapCanvas.mapSettings

        function onExtentChanged() {
            if ( gpsButton.followActive ) {
                if ( gpsButton.followActiveSkipExtentChanged ) {
                    gpsButton.followActiveSkipExtentChanged = false;
                } else {
                    gpsButton.followActive = false
                    displayToast( qsTr( "Canvas stopped following location" ) )
                }
            }
        }
    }

    DigitizingToolbar {
      id: digitizingToolbar

      stateVisible: (stateMachine.state === "digitize"
                     && dashBoard.currentLayer
                     && !dashBoard.currentLayer.readOnly
                     // unfortunately there is no way to call QVariant::toBool in QML so the value is a string
                     && dashBoard.currentLayer.customProperty( 'QFieldSync/is_geometry_locked' ) !== 'true'
                     && !geometryEditorsToolbar.stateVisible) || stateMachine.state === 'measure' ||
                    (stateMachine.state === "digitize" && digitizingToolbar.geometryRequested)
      rubberbandModel: currentRubberband ? currentRubberband.model : null
      mapSettings: mapCanvas.mapSettings
      showConfirmButton: stateMachine.state === "digitize"
      screenHovering: hoverHandler.hovered

      digitizingLogger.type: stateMachine.state === 'measure' ? '' : 'add'

      FeatureModel {
        id: digitizingFeature
        project: qgisProject
        currentLayer: digitizingToolbar.geometryRequested ? digitizingToolbar.geometryRequestedLayer : dashBoard.currentLayer
        positionInformation: positionSource.positionInfo
        topSnappingResult: coordinateLocator.topSnappingResult
        positionLocked: gpsLinkButton.checked
        cloudUserInformation: cloudConnection.userInformation
        geometry: Geometry {
          id: digitizingGeometry
          rubberbandModel: digitizingRubberband.model
          vectorLayer: digitizingToolbar.geometryRequested ? digitizingToolbar.geometryRequestedLayer : dashBoard.currentLayer
        }
      }

      property string previousStateMachineState: ''
      onGeometryRequestedChanged: {
          if ( geometryRequested ) {
              digitizingRubberband.model.reset()
              previousStateMachineState = stateMachine.state
              stateMachine.state = "digitize"
          }
          else
          {
              stateMachine.state = previousStateMachineState
          }
      }

      onVertexCountChanged: {
        if( qfieldSettings.autoSave && stateMachine.state === "digitize" ) {
            if( digitizingToolbar.geometryValid )
            {
                if (digitizingRubberband.model.geometryType === QgsWkbTypes.NullGeometry )
                {
                  digitizingRubberband.model.reset()
                }
                else
                {
                  digitizingFeature.geometry.applyRubberband()
                  digitizingFeature.applyGeometry()
                }

                if( !overlayFeatureFormDrawer.featureForm.featureCreated )
                {
                    overlayFeatureFormDrawer.featureModel.geometry = digitizingFeature.geometry
                    overlayFeatureFormDrawer.featureModel.applyGeometry()
                    overlayFeatureFormDrawer.featureForm.resetAttributes()
                    if( overlayFeatureFormDrawer.featureForm.model.constraintsHardValid ) {
                      // when the constrainst are fulfilled
                      // indirect action, no need to check for success and display a toast, the log is enough
                      overlayFeatureFormDrawer.featureForm.featureCreated = overlayFeatureFormDrawer.featureForm.create()
                    }
                } else {
                  // indirect action, no need to check for success and display a toast, the log is enough
                  overlayFeatureFormDrawer.featureModel.geometry = digitizingFeature.geometry
                  overlayFeatureFormDrawer.featureModel.applyGeometry()
                  overlayFeatureFormDrawer.featureForm.save()
                }
            } else {
              if( overlayFeatureFormDrawer.featureForm.featureCreated ) {
                // delete the feature when the geometry gets invalid again
                // indirect action, no need to check for success and display a toast, the log is enough
                overlayFeatureFormDrawer.featureForm.featureCreated = !overlayFeatureFormDrawer.featureForm.deleteFeature()
              }
            }
        }
      }

      onCancel: {
          if ( geometryRequested )
          {
              geometryRequested = false
          }
      }

      onConfirmed: {
        if ( geometryRequested )
        {
            if ( overlayFeatureFormDrawer.isAdding )
                overlayFeatureFormDrawer.open()

            coordinateLocator.flash()
            digitizingFeature.geometry.applyRubberband()
            geometryRequestedItem.requestedGeometry(digitizingFeature.geometry)
            digitizingRubberband.model.reset()
            geometryRequested = false
            return;
        }

        if (digitizingRubberband.model.geometryType === QgsWkbTypes.NullGeometry )
        {
          digitizingRubberband.model.reset()
        }
        else
        {
          coordinateLocator.flash()
          digitizingFeature.geometry.applyRubberband()
          digitizingFeature.applyGeometry()
          digitizingRubberband.model.frozen = true
          digitizingFeature.updateRubberband()
        }

        if ( !digitizingFeature.suppressFeatureForm() )
        {
          overlayFeatureFormDrawer.featureModel.geometry = digitizingFeature.geometry
          overlayFeatureFormDrawer.featureModel.applyGeometry()
          overlayFeatureFormDrawer.featureModel.resetAttributes()
          overlayFeatureFormDrawer.open()
          overlayFeatureFormDrawer.state = "Add"
          overlayFeatureFormDrawer.featureForm.reset()
        }
        else
        {
          if ( !overlayFeatureFormDrawer.featureForm.featureCreated ) {
              overlayFeatureFormDrawer.featureModel.geometry = digitizingFeature.geometry
              overlayFeatureFormDrawer.featureModel.applyGeometry()
              overlayFeatureFormDrawer.featureModel.resetAttributes()
              if ( !overlayFeatureFormDrawer.featureModel.create() ) {
                displayToast( qsTr( "Failed to create feature!" ), 'error' )
              }
          } else {
              if ( !overlayFeatureFormDrawer.featureModel.save() ) {
                displayToast( qsTr( "Failed to save feature!" ), 'error' )
              }
          }
          digitizingRubberband.model.reset()
          digitizingFeature.resetFeature();
        }
      }
    }

    GeometryEditorsToolbar {
      id: geometryEditorsToolbar

      featureModel: geometryEditingFeature
      mapSettings: mapCanvas.mapSettings
      editorRubberbandModel: geometryEditorsRubberband.model
      screenHovering: hoverHandler.hovered

      stateVisible: ( stateMachine.state === "digitize" && vertexModel.vertexCount > 0 )
    }
  }


  Menu {
    id: mainMenu
    title: qsTr( "Main Menu" )

    width: {
        var result = 0;
        var padding = 0;
        for (var i = 0; i < count; ++i) {
            var item = itemAt(i);
            result = Math.max(item.contentItem.implicitWidth, result);
            padding = Math.max(item.padding, padding);
        }
        return result + padding * 2;
    }

    MenuItem {
      text: qsTr( 'Measure Tool' )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      onTriggered: {
        dashBoard.close()
        changeMode( 'measure' )
        highlighted = false
      }
    }

    MenuItem {
      id: printItem
      text: qsTr( "Print to PDF" )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      onTriggered: {
        if (layoutListInstantiator.model.rowCount() > 1)
        {
          printMenu.popup( mainMenu.x, mainMenu.y + printItem.y )
        }
        else
        {
          mainMenu.close();
          displayToast( qsTr( 'Printing to PDF') )
          printMenu.printName =layoutListInstantiator.model.titleAt( 0 );
          printMenu.printTimer.restart();
        }
        highlighted = false
      }
    }

    MenuSeparator { width: parent.width }

    MenuItem {
      id: openProjectMenuItem

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      text: qsTr( "Open Project" )
      onTriggered: {
        dashBoard.close()
        welcomeScreen.visible = true
        welcomeScreen.focus = true
        highlighted = false
      }
    }

    MenuSeparator { width: parent.width }

    MenuItem {
      text: qsTr( "Settings" )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      onTriggered: {
        dashBoard.close()
        qfieldSettings.visible = true
        highlighted = false
      }
    }

    MenuItem {
      text: qsTr( "Message Log" )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      onTriggered: {
        dashBoard.close()
        messageLog.visible = true
        highlighted = false
      }
    }

    MenuItem {
      text: qsTr( "About QField" )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      onTriggered: {
        dashBoard.close()
        aboutDialog.visible = true
        highlighted = false
      }
    }

    Connections {
        target: printMenu

        function onEnablePrintItem(rows) {
          printItem.enabled = rows
        }
    }

    /*
    We removed this MenuItem part, because usually a mobile app has not the functionality to quit.
    But we keep the code in case, the concept changes or we need to close the app completely (remove from background)
    */

    /*
    Controls.MenuSeparator {}

    Controls.MenuItem {
      text: qsTr( "Quit" )
      iconSource: Theme.getThemeIcon( "ic_close_white_24dp" )
      onTriggered: {
        Qt.quit()
      }
    }
    */
  }

  Menu {
    id: printMenu

    property alias printTimer: timer
    property alias printName: timer.printName

    title: qsTr( "Print to PDF" )

    signal enablePrintItem( int rows )

    width: {
        var result = 0;
        var padding = 0;
        for (var i = 0; i < count; ++i) {
            var item = itemAt(i);
            result = Math.max(item.contentItem.implicitWidth, result);
            padding = Math.max(item.padding, padding);
        }
        return Math.min( result + padding * 2,mainWindow.width - 20);
    }

    MenuItem {
      text: qsTr( 'Select template below' )

      font: Theme.defaultFont
      height: 48
      leftPadding: 10

      enabled: false
    }

    Instantiator {

      id: layoutListInstantiator

      model: PrintLayoutListModel {
      }

      MenuItem {
        text: Title

        font: Theme.defaultFont
        leftPadding: 10

        onTriggered: {
            highlighted = false
            displayToast( qsTr( 'Printing to PDF') )
            printMenu.printName = Title
            printMenu.printTimer.restart();
        }
      }
      onObjectAdded: printMenu.insertItem(index+1, object)
      onObjectRemoved: printMenu.removeItem(object)
    }

    Connections {
      target: iface

      function onLoadProjectEnded() {
        layoutListInstantiator.model.project = qgisProject
        layoutListInstantiator.model.reloadModel()
        printMenu.enablePrintItem(layoutListInstantiator.model.rowCount())
        welcomeScreen.visible = false
        welcomeScreen.focus = false
        recentProjectListModel.reloadModel()
        settings.setValue( "/QField/FirstRunFlag", false )
        if (stateMachine.state === "digitize") {
            dashBoard.ensureEditableLayerSelected();
        }
      }
    }

    Timer {
      id: timer

      property string printName: ''

      interval: 500
      repeat: false
      onTriggered: iface.print( printName )
    }
  }

  PositioningSettings {
      id: positioningSettings

      onPositioningActivatedChanged: {
          if ( positioningActivated )
          {
            if ( platformUtilities.checkPositioningPermissions() )
            {
              displayToast( qsTr( "Activating positioning service" ) )
              positionSource.active = true
            }
            else
            {
              displayToast( qsTr( "QField has no permissions to use positioning." ), 'warning' )
              positioningSettings.positioningActivated = false
            }
          }
          else
          {
              positionSource.active = false
          }
      }
  }

  Menu {
    id: gpsMenu
    title: qsTr( "Positioning Options" )
    font: Theme.defaultFont

    width: {
        var result = 0;
        var padding = 0;
        for (var i = 0; i < count; ++i) {
            var item = itemAt(i);
            result = Math.max(item.contentItem.implicitWidth, result);
            padding = Math.max(item.padding, padding);
        }
        return Math.min( result + padding * 2,mainWindow.width - 20);
    }

    MenuItem {
        id: positioningDeviceName
        text: positioningSettings.positioningDeviceName
        height: 48
        font: Theme.defaultFont
        enabled:false
    }

    MenuSeparator { width: parent.width }

    MenuItem {
      id: positioningItem
      text: qsTr( "Enable Positioning" )
      height: 48
      font: Theme.defaultFont

      checkable: true
      checked: positioningSettings.positioningActivated
      indicator.height: 20
      indicator.width: 20
      indicator.implicitHeight: 24
      indicator.implicitWidth: 24
      onCheckedChanged: positioningSettings.positioningActivated = checked
    }

    MenuItem {
      text: qsTr( "Show Position Information" )
      height: 48
      font: Theme.defaultFont

      checkable: true
      checked: positioningSettings.showPositionInformation
      indicator.height: 20
      indicator.width: 20
      indicator.implicitHeight: 24
      indicator.implicitWidth: 24
      onCheckedChanged: positioningSettings.showPositionInformation = checked
    }

    MenuItem {
      text: qsTr( "Positioning Settings" )
      height: 48
      font: Theme.defaultFont

      onTriggered: {
        qfieldSettings.currentPanel = 1
        qfieldSettings.visible = true
      }
    }

    MenuSeparator { width: parent.width }

    MenuItem {
      text: qsTr( "Center to Current Location" )
      height: 48
      font: Theme.defaultFont

      onTriggered: {
        mapCanvas.mapSettings.setCenter(positionSource.projectedPosition)
      }
    }
  }

  /* The feature form */
  FeatureListForm {
    id: featureForm

    objectName: "featureForm"
    mapSettings: mapCanvas.mapSettings
    digitizingToolbar: digitizingToolbar

    visible: state != "Hidden"
    focus: visible

    anchors { right: parent.right; bottom: parent.bottom }
    border { color: "lightGray"; width: 1 }
    allowEdit: stateMachine.state === "digitize"
    allowDelete: stateMachine.state === "digitize"

    model: MultiFeatureListModel {}

    selection: FeatureListModelSelection {
      id: featureListModelSelection
      model: featureForm.model
    }

    selectionColor: "#ff7777"

    onShowMessage: displayToast(message)

    onEditGeometry: {
      // Set overall selected (i.e. current) layer to that of the feature geometry being edited,
      // important for snapping settings to make sense when set to current layer
      if ( dashBoard.currentLayer != featureForm.selection.focusedLayer ) {
        dashBoard.currentLayer = featureForm.selection.focusedLayer
        displayToast( qsTr( "Current layer switched to the one holding the selected geometry." ) );
      }
      geometryEditingFeature.vertexModel.geometry = featureForm.selection.focusedGeometry
      geometryEditingFeature.vertexModel.crs = featureForm.selection.focusedLayer.crs
      geometryEditingFeature.currentLayer = featureForm.selection.focusedLayer
      geometryEditingFeature.feature = featureForm.selection.focusedFeature

      if (!vertexModel.editingAllowed)
      {
        displayToast( qsTr( "Editing of multi geometry layer is not supported yet." ) )
        vertexModel.clear()
      }
      else
      {
        featureForm.state = "Hidden"
      }

      geometryEditorsToolbar.init()
    }

    Component.onCompleted: focusstack.addFocusTaker( this )

    //that the focus is set by selecting the empty space
    MouseArea {
      anchors.fill: parent
      propagateComposedEvents: true
      enabled: !parent.activeFocus

      //onPressed because onClicked shall be handled in underlying MouseArea
      onPressed: {
        parent.focus=true
        mouse.accepted=false
      }
    }
  }

  OverlayFeatureFormDrawer {
    id: overlayFeatureFormDrawer
    digitizingToolbar: digitizingToolbar
    featureModel.currentLayer: dashBoard.currentLayer
  }

  function displayToast( message, type ) {
    //toastMessage.text = message
    if( !welcomeScreen.visible )
      toast.show(message, type)
  }

  Rectangle {
    id: busyMessage
    anchors.fill: parent
    color: Theme.darkGray
    opacity: 0
    visible: false

    state: "hidden"
    states: [
        State {
            name: "hidden"
            PropertyChanges { target: busyMessage; opacity: 0 }
            PropertyChanges { target: busyMessage; visible: false }
        },

        State {
            name: "visible"
            PropertyChanges { target: busyMessage; visible: true }
            PropertyChanges { target: busyMessage; opacity: 0.75 }
        }]
    transitions: [
        Transition {
            from: "hidden"
            to: "visible"
            SequentialAnimation {
                PropertyAnimation { target: busyMessage; property: "visible"; duration: 0 }
                NumberAnimation { target: busyMessage; easing.type: Easing.InOutQuad; properties: "opacity"; duration: 250 }
            }
        },
        Transition {
            from: "visible"
            to: "hidden"
            SequentialAnimation {
                PropertyAnimation { target: busyMessage; easing.type: Easing.InOutQuad; property: "opacity"; duration: 250 }
                PropertyAnimation { target: busyMessage; property: "visible"; duration: 0 }
            }
        }
    ]

    BusyIndicator {
      id: busyMessageIndicator
      anchors.centerIn: parent
      running: true
      width: 100
      height: 100
    }

    Text {
      id: busyMessageText
      anchors.top: busyMessageIndicator.bottom
      anchors.horizontalCenter: parent.horizontalCenter
      horizontalAlignment: Text.AlignHCenter
      font: Theme.tipFont
      color: Theme.mainColor
      text: ''
    }

    Timer {
      id: readProjectTimer

      interval: 250
      repeat: false
      onTriggered: iface.readProject()
    }

    Connections {
      target: iface

      function onLoadProjectTriggered(path,name) {
        welcomeScreen.visible = false

        dashBoard.layerTree.freeze()
        mapCanvasMap.freeze('projectload')

        busyMessageText.text = qsTr( "Loading %1" ).arg( name !== '' ? name : path )
        busyMessage.state = "visible"

        projectInfo.filePath = '';
        readProjectTimer.start()
      }

      function onLoadProjectEnded(path,name) {
        mapCanvasMap.unfreeze('projectload')
        busyMessage.state = "hidden"

        projectInfo.filePath = path;

        mapCanvasBackground.color = mapCanvas.mapSettings.backgroundColor

        var cloudProjectId = QFieldCloudUtils.getProjectId(qgisProject.fileName)
        cloudProjectsModel.currentProjectId = cloudProjectId
        cloudProjectsModel.refreshProjectModification(cloudProjectId)
        if (cloudProjectId != '') {
          var cloudProjectData = cloudProjectsModel.getProjectData(cloudProjectId)
          switch(cloudProjectData.UserRole) {
            case 'reader':
              stateMachine.state = "browse"
              projectInfo.insertRights = false
              projectInfo.editRights = false
              break;
            case 'reporter':
              projectInfo.insertRights = true
              projectInfo.editRights = false
              break;
            case 'editor':
            case 'manager':
            case 'admin':
              projectInfo.insertRights = true
              projectInfo.editRights = true
              break;
            default:
              projectInfo.insertRights = true
              projectInfo.editRights = true
              break;
          }

          if (cloudProjectsModel.layerObserver.deltaFileWrapper.hasError()) {
            cloudPopup.show()
          }
        } else {
          projectInfo.insertRights = true
          projectInfo.editRights = true
        }
      }

      function onSetMapExtent(extent) {
          mapCanvas.mapSettings.extent = extent;
      }
    }
  }

  ProjectInfo {
    id: projectInfo

    mapSettings: mapCanvas.mapSettings
    layerTree: dashBoard.layerTree

    property bool insertRights: true
    property bool editRights: true
  }

  BusyIndicator {
    id: busyIndicator
    anchors.left: mainMenuBar.left
    anchors.top: mainToolbar.bottom
    width: mainMenuBar.height
    height: mainMenuBar.height
    running: mapCanvasMap.isRendering
  }

  MessageLog {
    id: messageLog
    anchors.fill: parent
    focus: visible

    model: MessageLogModel {
      id: messageLogModel
    }

    visible: false

    onFinished: {
      visible = false
    }

    Keys.onReleased: {
      if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
        event.accepted = true
        visible = false
      }
    }

    Component.onCompleted: focusstack.addFocusTaker( this )
  }

  BadLayerItem {
    id: badLayersView

    anchors.fill: parent
    model: BadLayerHandler {
      project: qgisProject

      onBadLayersFound: {
        badLayersView.visible = true
      }
    }

    visible: false

    onFinished: {
      visible = false
    }
  }

  Item {
    id: layerLogin

    Connections {
      target: iface

      function onLoadProjectEnded() {
        dashBoard.layerTree.unfreeze( true );
        if( !qfieldAuthRequestHandler.handleLayerLogins() )
        {
          //project loaded without more layer handling needed
          messageLogModel.unsuppressTags(["WFS","WMS"])
        }
      }
    }
    Connections {
        target: iface

        function onLoadProjectTriggered(path) {
          messageLogModel.suppressTags(["WFS","WMS"])
        }
    }

    Connections {
      target: qfieldAuthRequestHandler

      function onShowLoginDialog(realm) {
          loginDialogPopup.realm = realm || ""
          badLayersView.visible = false
          loginDialogPopup.open()
      }

      function onReloadEverything() {
          iface.reloadProject()
      }

      function onShowLoginBrowser(url) {
          loginBrowserPopup.url = url;
          loginBrowserPopup.open();
      }

      function onHideLoginBrowser() {
          loginBrowserPopup.close();
      }
    }

    Popup {
        id: loginBrowserPopup
        parent: ApplicationWindow.overlay

        property alias url: browserPanel.url

        x: 24
        y: 24
        width: parent.width - 48
        height: parent.height - 48
        padding: 0
        modal: true
        closePolicy: Popup.CloseOnEscape

        BrowserPanel {
            id: browserPanel
            anchors.fill: parent
            visible: true

            onCancel: {
                qfieldAuthRequestHandler.abortAuthBrowser();
                loginBrowserPopup.close();
            }
        }
    }

    Popup {
      id: loginDialogPopup
      parent: ApplicationWindow.overlay

      property var realm: ""

      x: 24
      y: 24
      width: parent.width - 48
      height: parent.height - 48
      padding: 0
      modal: true
      closePolicy: Popup.CloseOnEscape

      LayerLoginDialog {
        id: loginDialog

        anchors.fill: parent

        visible: true

        realm: loginDialogPopup.realm
        inCancelation: false

        onEnter: {
          qfieldAuthRequestHandler.enterCredentials( realm, usr, pw)
          inCancelation = false;
          loginDialogPopup.close()
        }
        onCancel: {
          inCancelation = true;
          loginDialogPopup.close(true)
        }
      }

      onClosed: {
        // handled here with parameter inCancelation because the loginDialog needs to be closed before the signal is fired
        qfieldAuthRequestHandler.loginDialogClosed(loginDialog.realm, loginDialog.inCancelation )
      }
    }

  }

  About {
    id: aboutDialog
    anchors.fill: parent
    focus: visible

    visible: false

    Keys.onReleased: {
      if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
        event.accepted = true
        visible = false
      }
    }

    Component.onCompleted: focusstack.addFocusTaker( this )
  }

  QFieldSettings {
    id: qfieldSettings

    anchors.fill: parent
    visible: false
    focus: visible

    onFinished: {
      visible = false
    }

    Keys.onReleased: {
      if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
        event.accepted = true
        finished()
      }
    }

    onDimBrightnessChanged: iface.setScreenDimmerActive( qfieldSettings.dimBrightness )
    Component.onCompleted: focusstack.addFocusTaker( this )
  }

  QFieldCloudConnection {
    id: cloudConnection
    onLoginFailed: function(reason) { displayToast( reason ) }
  }

  QFieldCloudProjectsModel {
    id: cloudProjectsModel
    cloudConnection: cloudConnection
    layerObserver: layerObserverAlias
    gpkgFlusher: gpkgFlusherAlias

    onProjectDownloaded: function ( projectId, projectName, hasError, errorString ) {
      return hasError
          ? displayToast( qsTr( "Project %1 failed to download" ).arg( projectName ), 'error' )
          : displayToast( qsTr( "Project %1 successfully downloaded, it's now available to open" ).arg( projectName ) );
    }

    onPushFinished: function ( projectId, hasError, errorString ) {
      if ( hasError ) {
        displayToast( qsTr( "Changes failed to reach QFieldCloud: %1" ).arg( errorString ), 'error' )
        return;
      }

      displayToast( qsTr( "Changes successfully pushed to QFieldCloud" ) )
    }

    onWarning: displayToast( message )
  }

  QFieldCloudDeltaHistory {
      id: qfieldCloudDeltaHistory

      modal: true
      closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
      parent: ApplicationWindow.overlay
  }

  QFieldCloudScreen {
    id: qfieldCloudScreen

    anchors.fill: parent
    visible: false
    focus: visible

    onFinished: {
      visible = false
      welcomeScreen.visible = true
    }

    Component.onCompleted: focusstack.addFocusTaker( this )
  }

  QFieldCloudPopup {
    id: cloudPopup
    visible: false
    parent: ApplicationWindow.overlay

    width: parent.width
    height: parent.height
  }

  QFieldCloudExportLayersFeedback {
    id: cloudExportLayersFeedback
    visible: false
    parent: ApplicationWindow.overlay
  }

  WelcomeScreen {
    id: welcomeScreen
    objectName: 'welcomeScreen'
    model: RecentProjectListModel {
      id: recentProjectListModel
    }
    property ProjectSource __projectSource

    anchors.fill: parent
    focus: true

    visible: true

    onShowOpenProjectDialog: {
      __projectSource = platformUtilities.openProject()
    }
    onShowQFieldCloudScreen: {
      welcomeScreen.visible = false
      qfieldCloudScreen.visible = true
    }

    Keys.onReleased: {
      if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
        if ( qgisProject.fileName != '') {
          event.accepted = true
          visible = false
          focus = false
        } else {
          event.accepted = false
          mainWindow.close()
        }
      }
    }

    Component.onCompleted: {
        focusstack.addFocusTaker( this )
    }
  }

  Changelog {
    id: changelogPopup
    parent: ApplicationWindow.overlay

    property var expireDate: new Date(2038,1,19)
    visible: settings && settings.value( "/QField/ChangelogVersion", "" ) !== appVersion && expireDate > new Date()
  }

  // Toast
  Popup {
      id: toast
      opacity: 0
      height: 40;
      width: parent.width
      y: parent.height - 112
      z: 10001
      margins: 0
      closePolicy: Popup.NoAutoClose

      background: Rectangle { color: "transparent" }

      function show(text, type) {
          toastMessage.text = text
          toastContent.type = type || 'info'
          toast.open()
          toastContent.visible = true
          toast.opacity = 1
          toastTimer.restart()
      }

      Behavior on opacity {
        NumberAnimation { duration: 250 }
      }

      Rectangle {
        id: toastContent
        color: "#66212121"

        property var type: 'info'

        height: toastMessage.height
        width: 8 + toastMessage.text.length * toastFontMetrics.averageCharacterWidth > mainWindow.width
               ? mainWindow.width - 16
               : 8 + toastMessage.text.length * toastFontMetrics.averageCharacterWidth

        anchors.centerIn: parent

        radius: 4

        z: 1

        Rectangle {
          id: toastIndicator
          anchors.left: parent.left
          anchors.leftMargin: 8
          anchors.verticalCenter: parent.verticalCenter
          width:  10
          height: 10
          radius: 5
          color: toastContent.type === 'error' ? Theme.errorColor : Theme.warningColor
          visible: toastContent.type != 'info'
        }

        Text {
          id: toastMessage
          anchors.left: parent.left
          anchors.right: parent.right
          wrapMode: Text.Wrap
          leftPadding: 18
          rightPadding: 18
          topPadding: 3
          bottomPadding: 3
          color: Theme.light

          font: Theme.secondaryTitleFont
          horizontalAlignment: Text.AlignHCenter
        }
      }

      FontMetrics {
          id: toastFontMetrics
          font: toastMessage.font
      }

      // Visible only for 3 seconds
      Timer {
          id: toastTimer
          interval: 3000
          onTriggered: {
              toast.opacity = 0
          }
      }

      onOpacityChanged: {

          if ( opacity == 0 ) {
              toastContent.visible = false
              toast.close()
          }
      }

      MouseArea {
        anchors.fill: parent
        onClicked: {
            toast.close()
            toast.opacity = 0
        }
      }
  }

  DropArea {
    id: dropArea
    anchors.fill: parent
    onEntered: {
      if ( drag.urls.length !== 1
         || !validateFileExtension( drag.urls[0] ) ) {
          drag.accepted = false
      }
      else {
        drag.accept (Qt.CopyAction)
        drag.accepted = true
      }
    }
    onDropped: {
      iface.loadFile( drop.urls[0] )
    }

    function validateFileExtension(filePath) {
      return filePath.split('.').pop() === "qgs" || filePath.split('.').pop() === "qgz"
    }
  }

  property bool alreadyCloseRequested: false

  onClosing: {
      if( !alreadyCloseRequested )
      {
        close.accepted = false
        alreadyCloseRequested = true
        displayToast( qsTr( "Press back again to close project and app" ) )
        closingTimer.start()
      }
      else
      {
        close.accepted = true
      }
  }

  Timer {
    id: closingTimer
    interval: 2000
    onTriggered: {
        alreadyCloseRequested = false
    }
  }

  Connections {
    target: welcomeScreen.__projectSource

    function onProjectOpened(path) {
      iface.loadFile(path)
    }
  }

  // ! MODELS !
  FeatureModel {
    id: geometryEditingFeature
    project: qgisProject
    currentLayer: null
    positionInformation: positionSource.positionInfo
    positionLocked: gpsLinkButton.checked
    vertexModel: vertexModel
    cloudUserInformation: cloudConnection.userInformation
  }

  VertexModel {
      id: vertexModel
      currentPoint: coordinateLocator.currentCoordinate
      mapSettings: mapCanvas.mapSettings
      isHovering: hoverHandler.hovered
  }
}
