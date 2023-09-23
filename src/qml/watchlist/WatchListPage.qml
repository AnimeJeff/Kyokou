import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle{
    property var swipeView
    color: "black"

    Component {
        id: dragDelegate
        //https://doc.qt.io/qt-6/qtquick-tutorials-dynamicview-dynamicview3-example.html

        Rectangle {
            required property string title
            required property string cover
            required property int index
            required property int unwatchedEpisodes
            id: content
            color: "black"
            Image {
                id:coverImage
                source: cover
                width: watchListView.cellWidth
                cache: true // Cache the image
                height: coverImage.width * aspectRatio
                anchors{
                    left: parent.left
                    top: parent.top
                }
                Rectangle {
                    visible: unwatchedEpisodes !== 0
                    width: height*1.5
                    height: parent.height/10
                    color: "red"
                    radius: 1
                    anchors{
                        right: parent.right
                        top: parent.top
                    }

                    Text {
//                        anchor.fill: parent
                        color: "white"
                        text: unwatchedEpisodes
                        font.pixelSize: 16
                    }
                }
            }

            Text {
                text: title
                font.bold: ListView.isCurrentItem
                anchors.top: coverImage.bottom
                //                anchors.bottom: parent.bottom
                width: watchListView.cellWidth
                wrapMode: Text.Wrap
                font.pixelSize: 12
                height: contentHeight
                color: "white"
            }
            property var list: watchListView
            width: watchListView.cellWidth
            height: watchListView.cellHeight

            Drag.active: dragArea.held
            Drag.source: dragArea
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            MouseArea {
                id: dragArea
                anchors {
                    fill: parent
                }
                onClicked: {
                    app.watchList.loadDetails(dragArea.DelegateModel.itemsIndex)
                }

                property int lastZ
                property bool held: true

                drag.target: held ? content : undefined
                drag.axis: Drag.XAndYAxis

                drag.onActiveChanged: {
                    if(drag.active)
                    {
                        lastZ = content.z
                        content.z = 10000000
                        content.list.lastY = content.list.contentY
                    }
                    else
                    {
                        content.z = lastZ
                        app.watchList.moveEnded()
                        content.list.contentY = content.list.lastY
                    }
                }

                DropArea {
                    anchors {
                        fill: parent
                        margins: 10
                    }

                    onEntered: (drag) => {
                                   let oldIndex = drag.source.DelegateModel.itemsIndex
                                   let newIndex = dragArea.DelegateModel.itemsIndex
                                   if(watchListView.lastLoadedIndex === oldIndex){
                                       watchListView.lastLoadedIndex = newIndex
                                   }
                                   if(Math.abs(newIndex-oldIndex) < itemPerRow * 2){
                                       app.watchList.move(oldIndex,newIndex)
                                       visualModel.items.move(oldIndex,newIndex)
                                   }

                               }
                }
            }
        }
    }
    DelegateModel {
        id: visualModel

        model: app.watchList
        delegate: dragDelegate
    }
    property real aspectRatio:319/225
    property real itemPerRow: 6



    ComboBox{
        id:listTypeComboBox
        anchors{
            left: parent.left
            top: parent.top
        }
        width: 150
        height: 30
        model: ListModel{
            ListElement { text: "Watching" }
            ListElement { text: "Planned" }
            ListElement { text: "On Hold" }
            ListElement { text: "Dropped" }
            ListElement { text: "Completed" }
        }
        hoverEnabled: true
        currentIndex: app.watchList.listType
        delegate: ItemDelegate {
            text: model.text
            width: parent.width
            height: 30
            highlighted: hovered
            background: Rectangle {
                color: highlighted ? "lightblue" : "transparent"
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    listTypeComboBox.displayText = model.text
                    app.watchList.listType = model.index
                    listTypeComboBox.popup.close()
                }
            }
        }
    }
    GridView{
        ScrollBar.vertical: ScrollBar {}
        id:watchListView
        property int lastY:0
        clip: true
        anchors{
            left: parent.left
            top: listTypeComboBox.bottom
            bottom: parent.bottom
            right: parent.right
        }
        model: visualModel
        cellHeight: cellWidth * aspectRatio + 35
        cellWidth: width/itemPerRow
    }



}
