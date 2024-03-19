import QtQuick 2.15
import QtQuick.Controls 2.15
GridView {
    id: container
    property real aspectRatio: 319/225
    property real itemPerRow: Math.floor(root.width/200)
    property real spacing: 10
    boundsBehavior:Flickable.StopAtBounds
    boundsMovement: Flickable.StopAtBounds
    anchors.topMargin: spacing
    clip: true
    cellHeight: cellWidth * aspectRatio + 35
    cellWidth: width/itemPerRow


    delegate: Column {
        id: item
        Image {
            id:coverImage
            source:  model.cover// : "qrc:/kyokou/images/error_image.png"
            onStatusChanged: if (coverImage.status === Image.Error) source = "qrc:/resources/images/error_image.png"
            width: parent.width - container.spacing
            height: width * container.aspectRatio
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

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: {
                    if (containsMouse)
                    {
                        app.cursor.shape = Qt.PointingHandCursor
                    }
                    else
                    {
                        app.cursor.shape = Qt.ArrowCursor
                    }
                }
                onClicked: (mouse)=>{
                               app.showExplorer.loadShow(index)
                           }
            }
        }

        Text {
            text: model.title
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment:Text.AlignHCenter
            width: container.cellWidth
            height: contentHeight
            wrapMode: Text.Wrap
            font.pixelSize: 20
            elide: Text.ElideRight
            color: "white"
            MouseArea {
                anchors.fill: parent
                onClicked: (mouse)=>{
                               if (mouse.button === Qt.RightButton)
                               {
                                   app.showExplorer.loadShow(index)
                               }
                           }
            }
        }
    }

    onContentYChanged: {
        searchResultsViewlastScrollY = contentY
    }

    //        footer: Rectangle{
    //            color: "transparent"
    //            width: parent.width
    //            height: 100
    //            z:container.z+1
    //            BusyIndicator{
    //                running: true
    //                visible: true
    //                width: width
    //                height: parent.height
    //                anchors.centerIn: parent

    //            }
    //        }

    //        interactive: true
    //        property int realContentHeight: Math.ceil(container.count/6)*cellHeight

    onAtYEndChanged: {
        if (atYEnd && count > 0 && app.showExplorer.canLoadMore())
        {
            app.showExplorer.loadMore();
        }
    }
}


