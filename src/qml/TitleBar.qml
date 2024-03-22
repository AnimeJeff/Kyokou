import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15
Rectangle {
    color: "#E6404040"
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: 35


    AnimatedImage {
        id:finn
        anchors {
            top:parent.top
            bottom: parent.bottom
            left:parent.right
        }
        visible:true
        z:parent.z+1
        source: "qrc:/resources/gifs/finn.gif"
        width: height
        playing: false
        HoverHandler{
            onHoveredChanged: parent.playing = hovered
        }

    }


    MouseArea {
        property var clickPos
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPressed: (mouse)=>{
                       clickPos  = Qt.point(mouse.x,mouse.y)
                   }
        onPositionChanged: (mouse)=> {
                               if (!root.maximised && clickPos !== null){
                                   root.x = app.cursor.pos().x - clickPos.x
                                   root.y = app.cursor.pos().y - clickPos.y
                               }
                           }
        onDoubleClicked: {
            root.maximised = !root.maximised
            clickPos = null
        }

    }

    Button  {
        id: closeButton
        width: 14
        height: 14
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 8
        background: Rectangle { color:  "#fa564d"; radius: 7; anchors.fill: parent }
        onClicked: root.close()
        focusPolicy: Qt.NoFocus
    }

    Button {
        id: maxButton
        width: 14
        height: 14
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: closeButton.left
        anchors.rightMargin: 6
        background: Rectangle { color: "#ffbf39"; radius: 7; anchors.fill: parent }
        onClicked: root.maximised = !root.maximised
        focusPolicy: Qt.NoFocus

    }

    Button {
        id: minButton
        width: 14
        height: 14
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: maxButton.left
        anchors.rightMargin: 6
        background: Rectangle { color: "#53cb43"; radius: 7; anchors.fill: parent }
        onClicked: {
            root.showMinimized()
        }
        focusPolicy: Qt.NoFocus
    }




}
