import QtQuick 2.15
import QtQuick.Controls 2.15
GridView {
    id: gridView
    property real aspectRatio: 319/225
    property real itemPerRow: Math.floor(root.width/200)
    property real spacing: 10
    boundsBehavior:Flickable.StopAtBounds
    boundsMovement: Flickable.StopAtBounds
    anchors.topMargin: spacing
    clip: true
    cellHeight: cellWidth * aspectRatio + 35
    cellWidth: width/itemPerRow
    add: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 400 }
        NumberAnimation { property: "scale"; from: 0; to: 1.0; duration: 400 }
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }

        // ensure opacity and scale values return to 1.0
        NumberAnimation { property: "opacity"; to: 1.0 }
        NumberAnimation { property: "scale"; to: 1.0 }
    }

    delegate: Column {
        id: item

        Image {
            id:coverImage
            source:  model.cover
            onStatusChanged: {
                if (coverImage.status != Image.Loading) loadingAnimation.destroy()
                if (coverImage.status === Image.Error) source = "qrc:/resources/images/error_image.png"
            }

            width: parent.width - gridView.spacing
            height: width * gridView.aspectRatio
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: (mouse)=>{
                               app.loadShow(index, false)
                           }
            }

            AnimatedImage {
                id: loadingAnimation
                anchors {
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
            width: gridView.cellWidth
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
        if (atYEnd && count > 0 && app.explorer.canLoadMore())
        {
            app.explorer.loadMore();
        }
    }
}


