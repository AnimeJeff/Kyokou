import QtQuick 2.15
import QtQuick.Controls
GridView {
    id: gridView

    property real itemPerRow: Math.floor(root.width/200)
    property real spacing: 2
    property real fontSize: 18 * root.fontSizeMultiplier
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    cellWidth: width / itemPerRow
    cellHeight: cellWidth * imageAspectRatio + fontSize * 2
    property real imageAspectRatio: 319/225
    onContentYChanged: watchListViewLastScrollY = contentY
    property int dragFromIndex: -1
    property int dragToIndex: -1
    property bool isDragging: false

    property int heldZ: z + 1000
    //https://doc.qt.io/qt-6/qtquick-tutorials-dynamicview-dynamicview3-example.html
    model: DelegateModel {
        id: visualModel
        model:app.watchList
        delegate: Item {
            required property string title
            required property string cover
            required property int index
            required property int unwatchedEpisodes
            property var view: gridView
            id: content
            width: gridView.cellWidth
            height: gridView.cellHeight
            Drag.active: dragArea.held
            Drag.source: dragArea
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            Image {
                id:coverImage
                source: cover
                onStatusChanged: if (status === Image.Error) source = "qrc:/resources/images/error_image.png"
                height: width * gridView.imageAspectRatio
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    leftMargin: gridView.spacing
                    rightMargin: gridView.spacing
                    topMargin: gridView.spacing
                }
            }

            Text {
                text: title
                anchors {
                    left: parent.left
                    right: parent.right
                    top: coverImage.bottom
                    bottom: parent.bottom
                    leftMargin: gridView.spacing
                    rightMargin: gridView.spacing
                    topMargin: gridView.spacing
                }
                horizontalAlignment:Text.AlignHCenter
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                font.pixelSize: gridView.fontSize
                elide: Text.ElideRight
                color: "white"
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse) => {
                               if (mouse.button === Qt.LeftButton) {
                                   app.loadShow(dragArea.DelegateModel.itemsIndex, true)
                               } else {
                                   contextMenu.showIndex = dragArea.DelegateModel.itemsIndex
                                   contextMenu.popup()
                               }
                           }

                drag.target: held ? content : undefined
                drag.axis: Drag.XAndYAxis
                // property int lastZ
                property bool held: false
                cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.PointingHandCursor

                onPressed: (mouse) => {
                               if (mouse.button === Qt.LeftButton) {
                                   held=true
                               }
                           }
                onReleased: (mouse) => {
                                if (mouse.button === Qt.LeftButton) {
                                    held=false
                                }
                            }

                drag.onActiveChanged: {
                    if (drag.active) {
                        // held ?= true
                        content.z = gridView.heldZ
                    }
                    else
                    {
                        //held = false
                        content.z = gridView.z
                        // console.log("moved",content.view.dragFromIndex, content.view.dragToIndex, content.view.isDragging)
                        app.watchList.move(content.view.dragFromIndex, content.view.dragToIndex)
                        visualModel.items.move(content.view.dragToIndex, content.view.dragToIndex)
                        content.view.dragFromIndex = -1
                    }
                }
                hoverEnabled: true
                DropArea {
                    id:dropArea
                    anchors.fill:parent
                    anchors.margins: 10
                    onEntered: (drag) => {
                                   let oldIndex = drag.source.DelegateModel.itemsIndex
                                   let newIndex = dragArea.DelegateModel.itemsIndex
                                   if (content.view.dragFromIndex === -1){
                                       content.view.dragFromIndex = oldIndex
                                       // console.log("from", content.view.dragFromIndex)
                                   }
                                   // console.log("to", newIndex)
                                   content.view.dragToIndex = newIndex
                                   visualModel.items.move(oldIndex, newIndex)
                               }
                }



            }

        }

    }

    Menu {
        id: contextMenu
        property int showIndex
        MenuItem {
            text: "Remove from library"
            onTriggered:  {
                app.watchList.removeAt(contextMenu.showIndex, -1)
            }
        }

        Menu {
            id: changeListTypeMenu
            title: "Change list type"
            Connections{
                target: app.watchList
                function onLayoutChanged(){
                    instantiator.model = app.watchList.getChangeableListTypes()
                }
            }

            Instantiator {
                id:instantiator
                model: app.watchList.changeableListTypes
                delegate: MenuItem {
                    text: app.watchList.displayableListType(modelData)
                    onTriggered: app.watchList.changeListTypeAt(contextMenu.showIndex, modelData, -1)
                }
                onObjectAdded: (index, object) => changeListTypeMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => changeListTypeMenu.removeItem(object)


            }
        }
    }

}



// AnimatedImage {
//     anchors {
//         left:parent.left
//         right:parent.right
//         bottom:parent.bottom
//     }
//     source: "qrc:/resources/gifs/image-loading.gif"
//     width: parent.width
//     height: width * 0.84
//     visible: parent.status == Image.Loading
//     playing: parent.status == Image.Loading
// }

// Rectangle {
//     visible: unwatchedEpisodes !== 0
//     width: height * 1.5
//     height: parent.height / 10
//     color: "red"
//     radius: 1
//     anchors {
//         right: parent.right
//         top: parent.top
//     }

//     Text {
//         color: "white"
//         text: unwatchedEpisodes
//         font.pixelSize: 16
//     }
// }


