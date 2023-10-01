import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
Item {
    id: searchPage

    SearchBar {
        id:searchBar
        anchors{
            left: parent.left
            right: parent.right
            top:parent.top
            topMargin: 5
        }
        height: 40
    }

    LoadingScreen {
        id:loadingScreen
        anchors.fill: parent
        z:parent.z+1
        loading: app.showExplorer.loading || showManager.loading
        onCancelled: {

            app.showExplorer.cancel()
            showManager.cancel()
        }
        onTimedOut: {
            root.notifier.headerText = "Error"
            root.notifier.text = "Operation took too long"
            root.notifier.open()
        }
    }


    ShowContainer {
        id:showContainer
        model: app.showExplorer
        anchors {
            top: searchBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Component.onCompleted: {
            contentY = root.searchResultsViewlastScrollY
            forceActiveFocus()
        }
    }

    Keys.enabled: true
    Keys.onPressed: event => handleKeyPress(event)
    function handleKeyPress(event){
        if(event.modifiers & Qt.ControlModifier){
            if(event.key === Qt.Key_R) {app.showExplorer.reload()}
        }else{
            switch (event.key) {
            case Qt.Key_Escape:
            case Qt.Key_Alt:
                if(searchBar.textField.activeFocus)
                    searchPage.forceActiveFocus()
                break;
            case Qt.Key_Enter:
                searchBar.search()
                break;
            case Qt.Key_Slash:
                searchBar.textField.forceActiveFocus()
                break;
            case Qt.Key_P:
                app.showExplorer.popular(1,4)
                break;
            case Qt.Key_L:
                app.showExplorer.latest(1,4)
                break;
            case Qt.Key_Up:
                showContainer.flick(0,500)
                break;

            }
        }

    }
}
