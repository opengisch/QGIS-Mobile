import QtQuick 2.14
import QtQuick.Controls 2.12

import Theme 1.0
import org.qfield 1.0

import "."

EditorWidgetBase {
  id: topItem

  property bool isEditable: isEnabled &&
                            LayerUtils.fieldType( field ) !== 'QStringList' &&
                            LayerUtils.fieldType( field ) !== 'QVariantList' &&
                            LayerUtils.fieldType( field ) !== 'QVariantMap'

  height: childrenRect.height

  // Due to QTextEdit::onLinkActivated does not work on Android & iOS, we need a separate `Text` element to support links https://bugreports.qt.io/browse/QTBUG-38487
  Label {
    id: textReadonlyValue
    height: textArea.height == 0 ? textField.height : 0
    topPadding: 10
    bottomPadding: 10
    visible: height !== 0 && !isEditable
    anchors.left: parent.left
    anchors.right: parent.right
    font: Theme.defaultFont
    color: 'gray'
    wrapMode: Text.Wrap

    text: value == null ? '' : stringUtilities.insertLinks(value)

    onLinkActivated: Qt.openUrlExternally(link)
  }

  TextField {
    id: textField
    topPadding: 10
    bottomPadding: 10
    visible: height !== 0 && isEditable
    anchors.left: parent.left
    anchors.right: parent.right
    font: Theme.defaultFont
    color: 'black'
    maximumLength: field.length > 0 ? field.length : -1
    wrapMode: TextInput.Wrap

    text: value == null ? '' : value

    validator: {
      if (field && field.isNumeric)
          if ( LayerUtils.fieldType( field ) === 'double')
          {
            doubleValidator;
          }
          else
          {
            intValidator;
          }
      else {
        null;
      }
    }

    IntValidator {
      id: intValidator
    }

    DoubleValidator {
      id: doubleValidator

      locale: 'C'
    }

    inputMethodHints: field && field.isNumeric ? Qt.ImhFormattedNumbersOnly : Qt.ImhNone

    background: Rectangle {
        width: parent.width
        height: parent.height
        color: "white";
    }

    onTextChanged: {
      valueChanged( text, text == '' )
    }
  }

  TextArea {
    id: textArea
    height: config['IsMultiline'] === true ? undefined : 0
    visible: height !== 0
    enabled: isEditable
    anchors.left: parent.left
    anchors.right: parent.right
    wrapMode: Text.Wrap
    font: Theme.defaultFont

    text: value !== undefined ? value : ''
    textFormat: config['UseHtml'] ? TextEdit.RichText : TextEdit.PlainText

    background: Rectangle {
        width: parent.width
        height: parent.height
        color: "white";
    }

    onEditingFinished: {
      valueChanged( text, text == '' )
    }
  }

  Rectangle {
      anchors.left: parent.left
      anchors.right: parent.right
      y: Math.max( textField.height, textArea.height ) - height - textField.bottomPadding / 2
      implicitWidth: 120
      height: textField.activeFocus || textArea.activeFocus ? 2: 1
      color: textField.activeFocus || textArea.activeFocus ? "#4CAF50" : "#C8E6C9"
  }

  FontMetrics {
    id: fontMetrics
    font: textField.font
  }
}
