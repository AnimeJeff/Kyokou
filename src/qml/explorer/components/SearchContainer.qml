import QtQuick 2.15
import QtQuick.Controls 2.15
GridView {
    id: list
    property real aspectRatio:319/225
    property real itemPerRow: 6
    property int lastIndex


    populate: Transition {
        OpacityAnimator {from: 0; to: 1; duration: 500}
    }
    add: Transition {
        OpacityAnimator {from: 0; to: 1; duration: 500}
    }
    remove: Transition {
        OpacityAnimator {from: 1; to: 0; duration: 250}
    }

    ScrollBar.vertical: ScrollBar {
        //            visible: explorer.advancedSearch.width == 300 || explorer.advancedSearch.width == 0
        active: hovered || pressed
        anchors.right: parent.right
        width: 20
        hoverEnabled: true
    }

        footer: busyFooter
        Component{
            id:busyFooter
            Rectangle{
                BusyIndicator {
                    width: footerHeight
                    visible: list.count > 0
                    running: true//explorer.grid.count > 0 && !explorer.endOfResults
                    height: footerHeight
                    anchors.centerIn: parent
                }
                color: "transparent"
                width: parent.width
                height: footerHeight
                visible: list.atYEnd
            }
        }
        property real footerHeight:100

    boundsBehavior: Flickable.DragOverBounds


    model: app.showExplorer
    cellHeight: cellWidth * aspectRatio + 35
    cellWidth: width/itemPerRow
    delegate:Rectangle {
        id: item
        color: "transparent"
        Image {
            id:coverImage
            source: model.cover
            onStatusChanged: if (coverImage.status === Image.Error){
                                 source = "qrc:/kyokou/images/error_image.png"
                             }

            width: list.cellWidth
            height: coverImage.width * list.aspectRatio
            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: coverImage.status == Image.Loading
                visible: coverImage.status == Image.Loading
                z:-1 //100
                height: 70
                width: 70
            }

        }

        Text {
            text: model.title
            font.bold: ListView.isCurrentItem
            anchors{
                top: coverImage.bottom
                bottom: parent.bottom
                left: coverImage.left
                right: coverImage.right
            }

            wrapMode: Text.Wrap
            font.pixelSize: 12
            color: "white"
            MouseArea{
                anchors.fill: parent
                onClicked: (mouse)=>{
                               if (mouse.button === Qt.RightButton){
                               }
                           }

            }
        }

        MouseArea{
            anchors.fill: coverImage
            z:parent.z+1
            onClicked: (mouse)=>{

                           //                               app.showExplorer.loadDetails(index)
                           if(list.contentY-list.originY>0)list.lastY = list.contentY-list.originY
                           console.log(list.lastY)
                           app.showExplorer.loadMore()
                       }
            cursorShape: Qt.PointingHandCursor
        }
    }

    highlight: highlight
    highlightFollowsCurrentItem: false
    focus: true
    clip: true

    property real lastY: 0
    property int realContentHeight: Math.ceil(list.count/list.itemPerRow)*list.cellHeight
    property int prevContentY
    property bool canFetch: true
    boundsMovement: GridView.StopAtBounds
    onAtYEndChanged: {
        if (atYEnd && canFetch){
            canFetch = false
            lastIndex = list.count-1
            lastY = contentY - originY
            app.showExplorer.loadMore()
        }
    }


    onContentYChanged: {
        if(!moving && (lastIndex !== (list.count-1))){
            list.positionViewAtIndex(lastIndex,GridView.End)
        }else if(moving){
            canFetch = true
        }
    }

}
