import QtQuick 2.15
import QtQuick.Controls 2.15
import "../../components"
import QtQuick.Layouts 1.15
Item {
    id:searchBar

    RowLayout{
        anchors.fill: parent
        CustomTextField{
            checkedColor: "#727CF5"
            id:searchTextField
            color: "white"
            Layout.fillHeight: true
            Layout.preferredWidth: searchBar.width/2
            placeholderText: qsTr("Enter query!")
            onAccepted: {
                app.searchResultsModel.search(searchTextField.text,1,1)
            }
        }

        CustomButton{
            id: searchButton
            HoverCursorArea{}
            text: "Search"
            focusPolicy: Qt.NoFocus
            onClicked: {
                app.searchResultsModel.search(searchTextField.text,1,1)
            }
        }

        CustomButton{
            id: latestButton
            HoverCursorArea{}

            text: "Latest"
            focusPolicy: Qt.NoFocus
            onClicked: {
                app.searchResultsModel.latest(1,3)
            }
        }

        CustomButton{
            id: popularButton
            HoverCursorArea{}
            text: "Popular"
            focusPolicy: Qt.NoFocus
            onClicked: {
                app.searchResultsModel.popular(1,3)
            }
        }

        CustomComboBox{
            id:providersComboBox
            Layout.fillWidth: true
            Layout.fillHeight: true
            displayText: global.currentSearchProvider.name
            model: ListModel{}
            Component.onCompleted: {
                global.providers.forEach(function(provider) {
                    providersComboBox.model.append({text:provider.name,providerEnum:provider.providerEnum})
                })
            }
            onClickedFun:function(index,model){
                global.changeSearchProvider(model.providerEnum)
                providersComboBox.popup.close()
            }
        }
    }


}
