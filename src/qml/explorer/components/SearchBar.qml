import QtQuick 2.15
import QtQuick.Controls 2.15
import "../../components"
import QtQuick.Layouts 1.15
Item {
    id:searchBar
    property alias textField: searchTextField
    function search(){
        app.showExplorer.search(searchTextField.text,1,4)
        window.lastSearch = searchTextField.text
        parent.forceActiveFocus()
    }

    RowLayout{
        anchors.fill: parent
        CustomTextField{
            checkedColor: "#727CF5"
            id:searchTextField
            color: "white"
            Layout.fillHeight: true
            Layout.preferredWidth: searchBar.width/2
            placeholderText: qsTr("Enter query!")
            text: window.lastSearch
            onAccepted: search()
        }

        CustomButton{
            id: searchButton
            HoverCursorArea{}
            text: "Search"
            focusPolicy: Qt.NoFocus
            onClicked: search()
        }

        CustomButton{
            id: latestButton
            HoverCursorArea{}

            text: "Latest"
            focusPolicy: Qt.NoFocus
            onClicked: {
                app.showExplorer.latest(1,4)
            }
        }

        CustomButton{
            id: popularButton
            HoverCursorArea{}
            text: "Popular"
            focusPolicy: Qt.NoFocus
            onClicked: {
                app.showExplorer.popular(1,4)
            }
        }

        CustomComboBox{
            id:providersComboBox
            Layout.fillWidth: true
            Layout.fillHeight: true
            displayText: showManager.currentSearchProviderName
            model: showManager
            onClickedFun:function(index,model){
                showManager.changeSearchProvider(model.providerEnum)
                providersComboBox.popup.close()
            }
        }
    }


}
