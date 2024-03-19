import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
Rectangle{
    property var swipeView
    color: "black"

    LoadingScreen {
        id:loadingScreen
        anchors.centerIn: parent
        z: parent.z + 1
        loading: showManager.loading
    }
    //https://doc.qt.io/qt-6/qtquick-tutorials-dynamicview-dynamicview3-example.html

    ComboBox {
        id:listTypeComboBox
        anchors{
            left: parent.left
            top: parent.top
        }
        width: 150
        height: 30
        model: ListModel{
            ListElement { text: "Watching" }
            ListElement { text: "Planned" }
            ListElement { text: "On Hold" }
            ListElement { text: "Dropped" }
            ListElement { text: "Completed" }
        }
        hoverEnabled: true
        currentIndex: app.watchList.listType
        delegate: ItemDelegate {
            text: model.text
            width: parent.width
            height: 30
            highlighted: hovered
            background: Rectangle {
                color: highlighted ? "lightblue" : "transparent"
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    listTypeComboBox.displayText = model.text
                    app.watchList.listType = model.index
                    listTypeComboBox.popup.close()
                }
            }
        }
    }

    WatchListContainer {
        anchors{
            left: parent.left
            top: listTypeComboBox.bottom
            bottom: parent.bottom
            right: parent.right
        }
        Component.onCompleted: {
            contentY = root.watchListViewLastScrollY
            forceActiveFocus()
        }
    }


}
