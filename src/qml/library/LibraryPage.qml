import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
Rectangle{
    id: libraryPage
    property var swipeView
    color: "black"

    LoadingScreen {
        id:loadingScreen
        anchors.centerIn: parent
        z: parent.z + 1
        loading: app.currentShow.isLoading && libraryPage.visible
    }
    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Tab) {
                            event.accepted = true
                            listTypeComboBox.popup.close()
                            app.library.cycleDisplayingListType() ;
                            listTypeComboBox.currentIndex = app.library.listType
                        }
                    }

    CustomComboBox {
        id:listTypeComboBox
        anchors {
            left: parent.left
            top: parent.top
        }


        width: parent.width * 0.2
        height: parent.height * 0.07
        fontSize: 20

        model: ListModel{
            ListElement { text: "Watching" }
            ListElement { text: "Planned" }
            ListElement { text: "On Hold" }
            ListElement { text: "Dropped" }
            ListElement { text: "Completed" }
        }
        text: "text"
        currentIndex: app.library.listType
        onActivated: (index) => {app.library.listType = index}

    }

    LibraryGridView {
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
