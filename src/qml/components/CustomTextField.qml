import QtQuick.Controls 2.0
import QtQuick 2.15
import QtQuick 2.4

TextField {
    id: textField
    property color checkedColor: "#D5DBDB"
    property color textColor: "white"

    signal doubleClicked(var/*MouseEvent*/ event)

    font.family: "QTxiaotu"

    font.pixelSize: 16
    font.weight: Font.Thin

    antialiasing: true

    background: Rectangle {
        radius: parent.height/2
        color: textField.enabled ? "transparent" : "#F4F6F6"
        border.color: textField.enabled ? textField.checkedColor : "#D5DBDB"
        border.width: 2
        opacity: textField.enabled ? 1 : 0.7
        layer.enabled: textField.hovered
    }



    onDoubleClicked: selectAll()

    HoverHandler{
        onHoveredChanged: {
            if (hovered)
            {
                app.cursor.shape = Qt.IBeamCursor
            }
            else
            {
                app.cursor.shape = Qt.ArrowCursor
            }
        }
    }
}



