import QtQuick
import "./../components"
import QtQuick.Controls 2.15

Rectangle{
    id:playlistBar
    visible: false
    color: '#d0303030'
    CustomComboBox{
        id:playlistComboBox
        anchors{
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 30
        displayText: app.playlist.showName
        //        model: showManager
        onClickedFun:function(index,model){

        }
    }

    ListView
    {
        id: listView
        model: app.playlist
        clip:true
        anchors
        {
            top: playlistComboBox.bottom
            right: parent.right
            left: parent.left
            bottom: bottomBar.top
        }
//        currentIndex: app.playlistModel.currentIndex

        onCurrentIndexChanged:
        {
            positionViewAtIndex(currentIndex, ListView.PositionAtCenter)
        }
        delegate: Rectangle
        {
            width: listView.width
            height: itemText.height + 10
            radius: 4
            clip: true
            color:"black"// index === app.playlistModel.currentIndex ? 'red': "black"
            Text
            {
                id:itemText
                text: model.numberTitle
                font.pixelSize: 14
                wrapMode:Text.Wrap
                color: "white"
                anchors{
                    left: parent.left
                    right: parent.right
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {

                    app.playlistModel.play(index)
                }
            }
        }
    }

    Rectangle
    {
        id:bottomBar
        anchors{
            bottom:parent.bottom
            left: parent.left
            right: parent.right
        }
        color: "#3C4144"
        height: 40
    }
}
