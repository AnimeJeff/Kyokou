import QtQuick 2.15
import QtQuick.Controls 2.15


Item {
    id:imageButton
    property alias source: image.source
    property string hoverSource: "" // Source for the image on hover
    property bool selected
    signal clicked()

    width: image.width
    height: image.height

    Image {
        id: image
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: imageButton.clicked()
        onEntered: {
            app.cursor.shape = Qt.PointingHandCursor
            if (hoverSource !== "") {
                image.source = hoverSource;
            }
        }
        onExited: {
            app.cursor.shape = Qt.ArrowCursor
            if (hoverSource !== "") {
                image.source = source; // Assuming the source hasn't dynamically changed while hovering
            }
        }
    }


}









// Button {
//     id: button
//     property string image: ""
//     property string hoverImage: ""
//     property bool selected: false
//     HoverHandler {
//         onHoveredChanged: {
//             if (hovered)
//             {
//                 app.cursor.shape = Qt.PointingHandCursor
//             }
//             else
//             {
//                 app.cursor.shape = Qt.ArrowCursor
//             }
//         }
//     }
//     background: Image {
//         mipmap: true
//         source: button.hovered ? hoverImage : image
//         sourceSize.width: parent.width
//         sourceSize.height: parent.height
//     }
//     focusPolicy: Qt.NoFocus

// }
