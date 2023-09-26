import QtQuick 2.0
import QtQuick.Controls 2.0


Button {
    id: button
    property color backgroundDefaultColor: "#4E5BF2"
    property color backgroundPressedColor: Qt.darker(backgroundDefaultColor, 1.2)
    property color contentItemTextColor: "white"

    text: "Button"
    contentItem: Text {
        text: button.text
        color: button.contentItemTextColor
        font.pixelSize: 15
        font.family: "Arial"
        font.weight: Font.Thin
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 83
        implicitHeight: 37
        color: button.down ? button.backgroundPressedColor : button.backgroundDefaultColor
        radius: 3
//        layer.enabled: true
//        layer.effect: DropShadow {
//            transparentBorder: true
//            color: button.down ? button.backgroundPressedColor : button.backgroundDefaultColor
//            samples: 20
//        }
    }
}
