import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import Qt.labs.calendar 1.0

import Theme 1.0

import "."

/*
  Config:
  * field_format
  * display_format
  * calendar_popup
  * allow_null

  If the calendar_popup is enabled, no direct editing is possible in the TextField.
  If not, it will try to match the display_format with the best possible InputMask.
  A Date/Time object (Date in QML) is used even with text field as source (not DateTime)
  to allow a full flexibility of field and display formats.

 */

EditorWidgetBase {
    id: main

    anchors {
        right: parent.right
        left: parent.left
    }

    height: childrenRect.height + 10
    enabled: isEnabled

    property bool isDateTimeType: field.isDateOrTime
    property bool fieldIsDate: LayerUtils.fieldType( field ) === 'QDate'
    property var currentValue: isDateTimeType ? value : Qt.formatDateTime(value, config['field_format'])


    Rectangle {
        visible: !enabled
        y: label.height - height - label.bottomPadding / 2
        implicitWidth: 120
        width: label.width
        height: 1
        color: "#C8E6C9"
    }

    TextField {
        id: label

        anchors.left: parent.left
        anchors.right: parent.right
        verticalAlignment: Text.AlignVCenter
        font: Theme.defaultFont
        leftPadding: enabled ? 5 : 0

        inputMethodHints: Qt.ImhDigitsOnly

        // TODO[DR] generate input mask using regex
        inputMask:      if (config['display_format'] === "yyyy-MM-dd" ) { "9999-99-99;_" }
                        else if (config['display_format'] === "yyyy.MM.dd" ) { "9999.99.99;_" }
                        else if (config['display_format'] === "yyyy-MM-dd HH:mm:ss" ) { "9999-99-99 99:99:99;_" }
                        else if (config['display_format'] === "HH:mm:ss" ) { "99:99:99;_" }
                        else if (config['display_format'] === "HH:mm" ) { "99:99;_" }
                        else { "" }

        text: if ( value === undefined || value == '' )
              {
                  qsTr('(no date)')
              }
              else
              {
                  var displayFormat = config['display_format'] == null
                          ? 'yyyy-MM-dd'
                          : config['display_format']

                  if ( main.isDateTimeType )
                  {
                      // if the field is a QDate, the automatic conversion to JS date [1]
                      // leads to the creation of date time object with the time zone.
                      // For instance shapefiles has support for dates but not date/time or time.
                      // So a date coming from a shapefile as 2001-01-01 will become 2000-12-31 19:00:00 -05 in QML/JS (in the carribeans).
                      // And when formatting this with the display format, this is shown as 2000-12-31.
                      // So we detect if the field is a date only and revert the time zone offset.
                      // [1] http://doc.qt.io/qt-5/qtqml-cppintegration-data.html#basic-qt-data-types
                      if (main.fieldIsDate) {
                          const date = new Date(value)
                          Qt.formatDateTime(new Date(date.getTime() + date.getTimezoneOffset() * 60000), displayFormat)
                      } else {
                          Qt.formatDateTime(value, displayFormat)
                      }
                  }
                  else
                  {
                      const date = Date.fromLocaleString(Qt.locale(), value, config['field_format'])
                      Qt.formatDateTime(date, displayFormat)
                  }
              }

        color: value === undefined || !enabled ? 'gray' : 'black'

        background: Item {
            implicitWidth: 120
            height: label.height

            Rectangle {
                id: backgroundRect
                anchors.fill: parent
                anchors.topMargin: label.topPadding / 2
                anchors.bottomMargin: label.bottomPadding / 2
                border.color: label.activeFocus ? "#4CAF50" : "#C8E6C9"
                border.width: label.activeFocus ? 2 : 1
                color: enabled ? Theme.lightGray : "transparent"
                radius: 2
                visible: enabled
            }
        }

        MouseArea {
            enabled: config['calendar_popup']
            anchors.fill: parent
            onClicked: {
                var usedDate = new Date();
                if (value !== undefined && value != '') {
                    usedDate = value;
                }

                calendar.selectedDate = usedDate
                calendar.year = usedDate.getFullYear()
                calendar.month = usedDate.getMonth()

                hoursSpinBox.value = usedDate.getHours()
                minutesSpinBox.value = usedDate.getMinutes()
                secondsSpinBox.value = usedDate.getSeconds()

                popup.open()
            }
        }

        onTextEdited: {
            var newDate = Date.fromLocaleString(Qt.locale(), label.text, config['display_format'])
            if ( newDate.toLocaleString() !== "" )
            {
                if ( !main.isDateTimeType )
                {
                    newDate = Qt.formatDateTime(newDate, config['field_format'])
                }
                valueChanged(newDate, newDate === undefined)
            }
            else
            {
                valueChanged(undefined, true)
            }
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                // getting focus => placing cursor at proper position
                // TODO: make it work on empty value
                var mytext = label.text
                var cur = label.cursorPosition
                while ( cur > 0 )
                {
                    if (!mytext.charAt(cur-1).match("[0-9]") )
                        break
                    cur--
                }
                label.cursorPosition = cur
            } else {
                // leaving field => if invalid, clear
                var newDate = Date.fromLocaleString(Qt.locale(), label.text, config['display_format'])
                if ( newDate.toLocaleString() === "" )
                {
                    label.text = qsTr('(no date)')
                }
            }
        }

        Image {
            id: todayButton
            width: 20
            height: 20
            source: Theme.getThemeIcon("ic_calendar_today_black_18dp")
            sourceSize.width: 20 * screen.devicePixelRatio
            sourceSize.height: 20 * screen.devicePixelRatio
            fillMode: Image.PreserveAspectFit
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -2
            visible: enabled

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if ( main.isDateTimeType )
                    {
                        var currentDateTime = new Date()
                        valueChanged(currentDateTime, false)
                    }
                    else
                    {
                        var currentDate = new Date()
                        var textDate = Qt.formatDateTime(currentDate, config['field_format'])
                        valueChanged(textDate, false)
                    }
                    displayToast(qsTr( 'Date value set to today.'))
                }
            }
        }

        Image {
            id: clearButton
            width: 20
            height: 20
            source: Theme.getThemeIcon("ic_clear_black_18dp")
            sourceSize.width: 20 * screen.devicePixelRatio
            sourceSize.height: 20 * screen.devicePixelRatio
            fillMode: Image.PreserveAspectFit
            anchors.right: todayButton.left
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -2
            visible: ( value !== undefined ) && config['allow_null'] && enabled

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    valueChanged(undefined, true)
                }
            }
        }
    }

    FontMetrics {
        id: fontMetrics
        font: label.font
    }

    Popup {
        id: popup
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        parent: ApplicationWindow.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        z: 10000 // 1000s are embedded feature forms, use a higher value to insure feature form popups always show above embedded feature formes

        ColumnLayout {
            Rectangle {
                id: calendarOverlay
                width: 350
                height: 240
                color: "transparent"

                MouseArea {
                    anchors.fill: parent
                    onClicked: mouse.accepted = true
                    onWheel: wheel.accepted = true
                }

                GridLayout {
                    id: calendarGrid
                    anchors.left: parent.left
                    anchors.right: parent.right
                    columns: 3

                    Row {
                        Layout.column: 0
                        Layout.row: 0

                        QfToolButton {
                            enabled: true
                            iconSource: Theme.getThemeIcon( 'ic_doublearrow_left_black_24dp' )
                            bgcolor: "transparent"
                            roundborder: true

                            onClicked: {
                                calendar.year -= 1;
                            }
                        }

                        QfToolButton {
                            enabled: true
                            iconSource: Theme.getThemeIcon( 'ic_arrow_left_black_24dp' )
                            bgcolor: "transparent"
                            roundborder: true

                            onClicked: {
                                if (calendar.month !== Calendar.January) {
                                    calendar.month -= 1;
                                } else {
                                    calendar.year -= 1;
                                    calendar.month = Calendar.December;
                                }
                            }
                        }
                    }

                    Text {
                        text: calendar.title
                        horizontalAlignment: Text.AlignHCenter
                        Layout.column: 1
                        Layout.row: 0
                        Layout.fillWidth: true
                        font: Theme.tipFont
                    }

                    Row {
                        Layout.column: 2
                        Layout.row: 0

                        QfToolButton {
                            enabled: true
                            iconSource: Theme.getThemeIcon( 'ic_arrow_right_black_24dp' )
                            bgcolor: "transparent"
                            roundborder: true

                            onClicked: {
                                if (calendar.month != Calendar.December) {
                                    calendar.month += 1;
                                } else {
                                    calendar.month = Calendar.January
                                    calendar.year += 1;
                                }
                            }
                        }
                        QfToolButton {
                            enabled: true
                            iconSource: Theme.getThemeIcon( 'ic_doublearrow_right_black_24dp' )
                            bgcolor: "transparent"
                            roundborder: true

                            onClicked: {
                                calendar.year += 1;
                            }
                        }
                    }

                    DayOfWeekRow {
                        locale: calendar.locale

                        Layout.column: 0
                        Layout.columnSpan: 3
                        Layout.row: 1
                        Layout.fillWidth: true
                    }

                    MonthGrid {
                        id: calendar
                        month: Calendar.January
                        year: 2020
                        Layout.row: 2
                        Layout.column: 0
                        Layout.columnSpan: 3
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        property date selectedDate: new Date()

                        delegate: Rectangle {
                            property bool isDate: calendar.selectedDate.getYear() === model.date.getYear() && calendar.selectedDate.getMonth() === model.date.getMonth() && calendar.selectedDate.getDate() === model.date.getDate()
                            color:  isDate ? Theme.mainColor : "transparent"
                            width: 18
                            height: 18

                            Text {
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                opacity: model.month !== calendar.month ? 0.5 : 1
                                text: model.day
                                font: parent.isDate ? Theme.strongTipFont : Theme.tipFont
                                color: parent.isDate ? "white" : "black"
                            }
                        }

                        onClicked: {
                            selectedDate = date;
                        }

                        function resetDate() {
                            selectedDate = main.currentValue ? main.isDateTimeType ? main.currentValue : Date.fromLocaleString(Qt.locale(), main.currentValue, config['field_format']) : new Date()
                        }
                    }
                }
            }

            RowLayout {
                GridLayout {
                    id: timeGrid
                    visible: !main.fieldIsDate
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: 20
                    rows: 3
                    columns: 2

                    Label {
                        Layout.alignment: Qt.AlignRight
                        Layout.row: 0
                        Layout.column: 0
                        text: qsTr( "Hours" )
                        font: Theme.tipFont
                    }
                    SpinBox {
                        id: hoursSpinBox
                        Layout.row: 0
                        Layout.column: 1
                        editable: true
                        from: 0
                        to: 23
                        value: 12
                        inputMethodHints: Qt.ImhTime
                        font: Theme.tipFont
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        Layout.row: 1
                        Layout.column: 0
                        text: qsTr( "Minutes" )
                        font: Theme.tipFont
                    }
                    SpinBox {
                        id: minutesSpinBox
                        Layout.row: 1
                        Layout.column: 1
                        editable: true
                        from: 0
                        to: 59
                        value: 30
                        inputMethodHints: Qt.ImhTime
                        font: Theme.tipFont
                    }
                    Label {
                        Layout.alignment: Qt.AlignRight
                        Layout.row: 2
                        Layout.column: 0
                        text: qsTr( "Seconds" )
                        font: Theme.tipFont
                    }
                    SpinBox {
                        id: secondsSpinBox
                        Layout.row: 2
                        Layout.column: 1
                        editable: true
                        from: 0
                        to: 59
                        value: 30
                        inputMethodHints: Qt.ImhTime
                        font: Theme.tipFont
                    }
                }
            }

            RowLayout {
                QfButton {
                    text: qsTr( "OK" )
                    font: Theme.tipFont
                    Layout.fillWidth: true

                    onClicked: {
                        var newDate = calendar.selectedDate
                        newDate.setHours(hoursSpinBox.value);
                        newDate.setMinutes(minutesSpinBox.value);
                        newDate.setSeconds(secondsSpinBox.value);
                        if ( main.isDateTimeType )
                        {
                            valueChanged(newDate, newDate === undefined)
                        }
                        else
                        {
                            var textDate = Qt.formatDateTime(newDate, config['field_format'])
                            valueChanged(textDate, textDate === undefined)
                        }

                        popup.close()
                    }
                }
            }
        }
    }

    onIsDateTimeTypeChanged: {
        calendar.resetDate()
    }

    onCurrentValueChanged: {
        calendar.resetDate()
    }
}
