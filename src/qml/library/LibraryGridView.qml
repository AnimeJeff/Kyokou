import QtQuick 2.15
import QtQuick.Controls
import "../components"
MediaGridView {
    id: gridView

    property int dragFromIndex: -1
    property int dragToIndex: -1
    property bool isDragging: false
    property int heldZ: z + 1000
    onContentYChanged: watchListViewLastScrollY = contentY

    //https://doc.qt.io/qt-6/qtquick-tutorials-dynamicview-dynamicview3-example.html
    model: DelegateModel {
        id: visualModel
        model: app.library
        delegate: ShowItem {
            id: content
            width: gridView.cellWidth
            height: gridView.cellHeight
            Drag.active: dragArea.held
            Drag.source: dragArea
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            title: model.title
            cover: model.cover
            // required property int index
            // required property int unwatchedEpisodes


            MouseArea {
                id: dragArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                hoverEnabled: true
                drag.target: held ? content : undefined
                drag.axis: Drag.XAndYAxis
                cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.PointingHandCursor

                property bool held: false


                onClicked: (mouse) => {
                               if (mouse.button === Qt.LeftButton) {
                                   app.loadShow(index, true)
                               } else {
                                   contextMenu.showIndex = index
                                   contextMenu.popup()
                               }
                           }

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
                        content.z = gridView.heldZ
                    }
                    else
                    {
                        content.z = gridView.z
                        // console.log("moved",content.view.dragFromIndex, content.view.dragToIndex, content.view.isDragging)
                        app.library.move(gridView.dragFromIndex, gridView.dragToIndex)
                        visualModel.items.move(gridView.dragToIndex, gridView.dragToIndex)
                        gridView.dragFromIndex = -1
                    }
                }

                DropArea {
                    id:dropArea
                    anchors.fill:parent
                    anchors.margins: 10
                    onEntered: (drag) => {
                                   //console.log(drag.source.DelegateModel, drag.source.DelegateModel.itemsIndex)
                                   let oldIndex = drag.source.DelegateModel.itemsIndex
                                   let newIndex = dragArea.DelegateModel.itemsIndex
                                   if (gridView.dragFromIndex === -1){
                                       gridView.dragFromIndex = oldIndex
                                   }
                                   gridView.dragToIndex = newIndex
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
                app.library.removeAt(contextMenu.showIndex, -1)
            }
        }

        Menu {
            id: changeListTypeMenu
            title: "Change list type"
            Connections{
                target: app.library
                function onLayoutChanged(){
                    instantiator.model = app.library.getChangeableListTypes()
                }
            }

            Instantiator {
                id:instantiator
                model: app.library.changeableListTypes
                delegate: MenuItem {
                    text: app.library.displayableListType(modelData)
                    onTriggered: app.library.changeListTypeAt(contextMenu.showIndex, modelData, -1)
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


