import QtQuick
import "./../components"
import QtQuick.Controls 2.15

Rectangle{
    id:playlistBar
    visible: false
    color: '#d0303030'

    Connections {
        id:currentIndexChangeConnection
        target:app.playlist
        function onCurrentIndexChanged()
        {
            if (!app.playlist.currentIndex.valid) return
            sel.setCurrentIndex(app.playlist.currentIndex, ItemSelectionModel.SelectCurrent)
            treeView.expandToIndex(app.playlist.currentIndex)
            treeView.forceLayout()
//            let halfHeight = treeView.height / 2
//            let contentY = 45 * (app.playlist.itemIndex-12) + 26.25 * (app.playlist.playlistIndex + 1)// - halfHeight
//            if (contentY < 0) contentY = 0
//            treeView.contentY = contentY

        }

    }
    TreeView {
        id: treeView
        model: app.playlist
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

        selectionModel: ItemSelectionModel {
            id:sel
            model: app.playlist
            onCurrentChanged:(current, previous)=>
                             {
                                 if (!current.parent.valid)
                                 {
                                     if (previous.parent.valid)
                                     {
                                         setCurrentIndex(previous, ItemSelectionModel.SelectCurrent)
                                     }
                                     else
                                     {
                                         clear()
                                     }
                                     return
                                 }
                                 if (current === app.playlist.currentIndex) return;
                                 app.playlist.load(current)
                             }
        }
        Component.onCompleted: {
            currentIndexChangeConnection.onCurrentIndexChanged()
        }

        selectionBehavior:TableView.SelectRows
        delegate: TreeViewDelegate {
            id:treeDelegate
            implicitWidth :treeView.width
            TapHandler {
                acceptedModifiers: Qt.NoModifier
                onTapped:
                {
                    if (treeDelegate.hasChildren)
                    {
                        if (treeView.isExpanded(row))
                        {
                            treeView.collapse(row)
                        }
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
                visible: treeDelegate.isTreeNode && treeDelegate.hasChildren
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
                color: current ? "red" : treeDelegate.hasChildren && row === app.playlist.playlistIndex ? "green" : "white"
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
