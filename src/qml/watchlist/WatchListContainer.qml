import QtQuick 2.15

GridView {

    id:container
    property real aspectRatio: 319/225
    property real itemPerRow: Math.floor(root.width/200)
    property real spacing: 5
    property real lastY:0
    property real fontSize: 18 * root.aspectRatio
    model: visualModel
    clip: true
    cellWidth: width / itemPerRow
    cellHeight: cellWidth * aspectRatio + fontSize * 2 * root.aspectRatio
    anchors.topMargin: spacing
    anchors.leftMargin: spacing

    DelegateModel {
        id:visualModel
        model:app.watchList
        delegate: Column {
            required property string title
            required property string cover
            required property int index
            required property int unwatchedEpisodes
            property GridView list: container
            id: content
            Drag.active: dragArea.held
            Drag.source: dragArea
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Image {
                source: cover
                onStatusChanged: if (status === Image.Error) source = "qrc:/resources/images/error_image.png"
                width: parent.width - container.spacing
                height: width * aspectRatio
                anchors.horizontalCenter: parent.horizontalCenter
                AnimatedImage {
                    anchors
                    {
                        left:parent.left
                        right:parent.right
                        bottom:parent.bottom
                    }
                    source: "qrc:/resources/gifs/image-loading.gif"
                    width: parent.width
                    height: width * 0.84
                    visible: parent.status == Image.Loading
                    playing: parent.status == Image.Loading
                }

                Rectangle {
                    visible: unwatchedEpisodes !== 0
                    width: height * 1.5
                    height: parent.height / 10
                    color: "red"
                    radius: 1
                    anchors{
                        right: parent.right
                        top: parent.top
                    }

                    Text {

                        color: "white"
                        text: unwatchedEpisodes
                        font.pixelSize: 16
                    }
                }
                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    onClicked: app.watchList.loadShow(dragArea.DelegateModel.itemsIndex)
                    property int lastZ
                    property bool held: true
                    drag.target: held ? content : undefined
                    drag.axis: Drag.XAndYAxis
                    drag.onActiveChanged: {
                        if(drag.active)
                        {
                            //todo change grab
                            app.cursor.shape = Qt.ClosedHandCursor
                            lastZ = content.z
                            content.z = 10000000
                            content.list.lastY = content.list.contentY
                        }
                        else
                        {
                            app.cursor.shape = Qt.PointingHandCursor
                            content.z = lastZ
                            app.watchList.moveEnded()
                            content.list.contentY = content.list.lastY
                        }
                    }

                    DropArea {
                        anchors.fill:parent
                        anchors.margins: 10
                        onEntered: (drag) => {
                                       let oldIndex = drag.source.DelegateModel.itemsIndex
                                       let newIndex = dragArea.DelegateModel.itemsIndex
                                       //                                       if(container.lastLoadedIndex === oldIndex){
                                       //                                           container.lastLoadedIndex = newIndex
                                       //                                       }
                                       if(Math.abs(newIndex-oldIndex) < container.itemPerRow * 2){
                                           app.watchList.move(oldIndex,newIndex)
                                           visualModel.items.move(oldIndex,newIndex)
                                       }

                                   }
                    }
                    hoverEnabled: true
                    onContainsMouseChanged: {
                        if(containsMouse && !drag.active)
                        {
                            app.cursor.shape = Qt.PointingHandCursor
                        }
                        else
                        {
                            app.cursor.shape = Qt.ArrowCursor
                        }
                    }


                }
            }

            Text {
                text: title
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment:Text.AlignHCenter
                width: container.cellWidth
                height: fontSize * 3 * root.aspectRatio
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                font.pixelSize: container.fontSize
                elide: Text.ElideRight
                color: "white"
            }


        }

    }




}

