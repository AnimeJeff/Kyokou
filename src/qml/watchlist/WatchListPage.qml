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
        loading: app.loading
    }
    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Tab) {
                            event.accepted = true
                            app.watchList.cycleDisplayingListType() ;
                            listTypeComboBox.currentIndex = app.watchList.listType
                        }
                    }

    CustomComboBox {
        id:listTypeComboBox
        anchors {
            left: parent.left
            top: parent.top
        }

        width: parent.width * 0.2
        height: 35
        fontSize: 20
        model: ListModel{
            ListElement { text: "Watching" }
            ListElement { text: "Planned" }
            ListElement { text: "On Hold" }
            ListElement { text: "Dropped" }
            ListElement { text: "Completed" }
        }
        text: "text"
        currentIndex: app.watchList.listType
        onActivated: (index) => {app.watchList.listType = index}

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
