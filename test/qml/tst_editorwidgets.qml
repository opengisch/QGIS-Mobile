import QtQuick 2.3
import QtTest 1.0

import QFieldControls 1.0

TestCase {
    name: "QFieldTests"

    TextEdit {
        id: textEdit
        property var value: 'one'
        property var config: undefined
        property bool isEnabled: true
    }

    Range {
        id: range
        property var value: 1
        property var config: undefined
        property bool isEnabled: true
    }

    function test_textEdit() {
        compare(textEdit.children[1].text, 'one')
        textEdit.value = 'two'
        compare(textEdit.children[1].text, 'two')
    }

    function test_range() {
        compare(range.children[0].children[0].text, '1')
        range.value = 2
        compare(range.children[0].children[0].text, '2')
    }
}
