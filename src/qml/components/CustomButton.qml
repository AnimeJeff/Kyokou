import QtQuick.Controls
import QtQuick

Button {
    id: button
    property color backgroundDefaultColor: "#4E5BF2"
    property color backgroundPressedColor: Qt.darker(backgroundDefaultColor, 1.2)
    property color contentItemTextColor: "white"
    property int fontSize: 18
    property alias radius: backRect.radius
    text: "Button"
    contentItem: Text {
        text: button.text
        color: button.contentItemTextColor
        font.weight: Font.Thin
        font.pixelSize: fontSize
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        wrapMode: Text.Wrap
    }

    HoverHandler{
        onHoveredChanged: {
            if (hovered)
            {
                app.cursor.shape = Qt.PointingHandCursor
            }
            else
            {
                app.cursor.shape = Qt.ArrowCursor
            }
        }
    }

    background: Rectangle {
        id:backRect
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
