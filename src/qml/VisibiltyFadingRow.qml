import QtQuick 2.6

Row {
  property bool stateVisible: true
  visible: opacity > 0

  states: [
      State { when: geometryEditingToolbar.stateVisible;
              PropertyChanges { target: geometryEditingToolbar; opacity: 1.0 }},
      State { when: !geometryEditingToolbar.stateVisible;
              PropertyChanges { target: geometryEditingToolbar; opacity: 0.0 }}
  ]

  transitions: Transition {
    SequentialAnimation {
      NumberAnimation {
        target: geometryEditingToolbar
        property: "opacity"
        duration: 400
      }
    }
  }
}
