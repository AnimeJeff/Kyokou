import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button
    property string image: ""
    property string hoverImage: ""
    property bool selected: false
    HoverHandler {
        onHoveredChanged: {
            if(hovered)
            {
                app.cursor.shape = Qt.PointingHandCursor
            }
            else
            {
                app.cursor.shape = Qt.ArrowCursor
            }
        }
    }
    background: Image {
        mipmap: true
        source: button.hovered ? hoverImage : image
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }
    focusPolicy: Qt.NoFocus

    // topInset and bottomInset is available after Qt5.12

}
