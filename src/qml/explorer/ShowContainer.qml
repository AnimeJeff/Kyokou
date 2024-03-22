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
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: (mouse)=>{
                               showManager.loadShow(index, false)
                           }
            }
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
        }


    }

    onContentYChanged: {
        searchResultsViewlastScrollY = contentY
    }


    onAtYEndChanged: {
        if (atYEnd && count > 0 && showManager.explorer.canLoadMore())
        {
            showManager.explorer.loadMore();
        }
    }
}


