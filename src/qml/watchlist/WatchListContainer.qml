import QtQuick 2.15

GridView {
    id: gridView
    property real aspectRatio: 319/225
    property real itemPerRow: Math.floor(root.width/200)
    property real spacing: 5
    property real lastY:0
    property real fontSize: 18 * root.aspectRatio
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    cellWidth: width / itemPerRow
    cellHeight: cellWidth * aspectRatio + fontSize * 2 * root.aspectRatio
    anchors.topMargin: spacing
    anchors.leftMargin: spacing
    onContentYChanged: watchListViewLastScrollY = contentY
    property real dragFromIndex: -1
    property real dragToIndex: -1
    property bool isDragging: false


    model: DelegateModel {
        id: visualModel
        model:app.watchList


        delegate: Column {
            required property string title
            required property string cover
            required property int index
            required property int unwatchedEpisodes
            property var view: gridView
            id: content
            Drag.active: dragArea.held
            Drag.source: dragArea
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            Image {
                source: cover
                onStatusChanged: if (status === Image.Error) source = "qrc:/resources/images/error_image.png"
                width: parent.width - gridView.spacing
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
                    anchors {
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
                    onClicked: app.loadShow(dragArea.DelegateModel.itemsIndex, true)
                    drag.target: held ? content : undefined
                    drag.axis: Drag.XAndYAxis
                    property int lastZ
                    property bool held: true
                    cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.PointingHandCursor

                    drag.onActiveChanged: {
                        if (drag.active)
                        {
                            lastZ = content.z
                            content.z = 1000
                            // content.list.lastY = content.list.contentY
                        }
                        else
                        {
                            content.z = lastZ
                            // console.log("moved",content.view.dragFromIndex, content.view.dragToIndex, content.view.isDragging)
                            app.watchList.move(content.view.dragFromIndex, content.view.dragToIndex)
                            visualModel.items.move(content.view.dragToIndex, content.view.dragToIndex)
                            content.view.dragFromIndex = -1
                            // content.list.contentY = content.list.lastY
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
                                       }
                                       content.view.dragToIndex = newIndex
                                       visualModel.items.move(oldIndex, newIndex)
                                   }
                    }
                }
            }

            Text {
                text: title
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment:Text.AlignHCenter
                width: gridView.cellWidth
                height: fontSize * 3 * root.aspectRatio
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                font.pixelSize: gridView.fontSize
                elide: Text.ElideRight
                color: "white"
            }


        }



    }




}

