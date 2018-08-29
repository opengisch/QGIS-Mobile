import QtQuick 2.0
import QtQml 2.2
import org.qgis 1.0
import org.qfield 1.0

Repeater {
    id: vertexRubberband
    property MapSettings mapSettings

    delegate: Rectangle {
      MapToScreen {
          id: mapToScreen
          mapSettings: vertexRubberband.mapSettings
          mapPoint: Point
      }

      x: mapToScreen.screenPoint.x - width/2
      y: mapToScreen.screenPoint.y - width/2

      width: 20*dp
      height: 20*dp
      radius: SegmentVertex ? width / 2 : 0

      border.color: if (CurrentVertex) {'red'} else {'blue'}
      border.width: 2*dp
    }
}

