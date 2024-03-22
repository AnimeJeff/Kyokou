import QtQuick
import "./../components"
import QtQuick.Controls 2.15

Rectangle{
    id:playlistBar
    color: '#d0303030'

    Connections {
        target: showManager.playList
        function onCurrentIndexChanged() {
            treeView.scrollToIndex(showManager.playList.currentIndex)
        }

    }

    TreeView {
        id: treeView
        model: showManager.playList
        clip:true
        boundsBehavior:Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds
        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
            bottom: bottomBar.top
        }
        keyNavigationEnabled:false
        smooth: false
        onContentHeightChanged: {
            scrollToIndex(showManager.playList.currentIndex)
        }

        function scrollToIndex(index){
            if (index.valid) {
                // console.log("Expanding to " + index)
                expandToIndex(index);
                // forceLayout()
                // positionViewAtRow(rowAtIndex(index), Qt.AlignVCenter)
                positionViewAtIndex(index, TableView.AlignVCenter)
                sel.setCurrentIndex(index, ItemSelectionModel.SelectCurrent)
            }
        }

        selectionModel: ItemSelectionModel {
            id:sel
            model: showManager.playList
            onCurrentChanged:(current, previous)=>
                             {
                                 // if a playlist is selected, select the previous
                                 if (!current.parent.valid)
                                 {
                                     // previous should be valid
                                     if (previous.parent.valid) {
                                         setCurrentIndex(previous, ItemSelectionModel.SelectCurrent)
                                     } else {
                                         clear()
                                     }
                                     return
                                 }
                                 if (current === showManager.playList.currentIndex) return;
                                 showManager.playList.load(current)
                             }
        }

        selectionBehavior:TableView.SelectRows
        delegate: TreeViewDelegate {
            id:treeDelegate

            implicitWidth :treeView.width
            onYChanged: {
                if(current)
                    treeDelegate.treeView.contentY = treeDelegate.y;
            }


            TapHandler {
                acceptedModifiers: Qt.NoModifier
                onTapped: {
                    if (treeDelegate.hasChildren)
                    {
                        if (treeView.isExpanded(row))
                            treeView.collapse(row)
                        else
                            treeView.expand(row)
                    }
                }
            }

            background: Rectangle {
                anchors.fill: parent
                color: "#d0303030"
            }
            indicator: Text {
                id: indicator
                visible: isTreeNode && hasChildren
                x: padding + (treeDelegate.depth * treeDelegate.indent)
                anchors.verticalCenter: treeDelegate.verticalCenter
                text: "▸"
                rotation: treeDelegate.expanded ? 90 : 0
                color: "red"
                font.bold: true
                font.pixelSize: 16 * root.aspectRatio
                height: font.pixelSize
            }
            contentItem: Text {
                id: label
                x: padding + (treeDelegate.isTreeNode ? (treeDelegate.depth + 1) * treeDelegate.indent : 0)
                width: treeDelegate.width - treeDelegate.padding - x
                font.pixelSize: treeDelegate.hasChildren ? 20 * root.aspectRatio : 16 * root.aspectRatio

                height: treeDelegate.hasChildren ? font.pixelSize : font.pixelSize * 2
                clip: true
                text: model.numberTitle
                color: current || selected ? "red" : "white"

                    //current ? "red" : treeDelegate.hasChildren && row === showManager.playList.playlistIndex ? "green" : "white"


            }
        }

    }

    Rectangle {
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
