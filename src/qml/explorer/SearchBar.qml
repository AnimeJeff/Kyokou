import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
import QtQuick.Layouts 1.15


RowLayout {
    id:searchBar
    property alias textField: searchTextField
    function search(){
        app.showExplorer.search(searchTextField.text, 1, typeComboBox.type)
        root.lastSearch = searchTextField.text
        parent.forceActiveFocus()
    }

    CustomTextField {
        checkedColor: "#727CF5"
        id:searchTextField
        color: "white"
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.3
        placeholderText: qsTr("Enter query!")
        text: root.lastSearch
        font.pixelSize: 20 * root.aspectRatio
        onAccepted: search()
    }
    
    CustomButton {
        id: searchButton
        text: "Search"
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width * 0.08
        fontSize:20 * root.aspectRatio
        radius: 20
        focusPolicy: Qt.NoFocus
        onClicked: search()
    }
    
    CustomButton {
        id: latestButton
        text: "Latest"
        Layout.fillHeight: true
        radius: 20
        fontSize:20 * root.aspectRatio
        Layout.preferredWidth: parent.width * 0.08
        focusPolicy: Qt.NoFocus
        onClicked: app.showExplorer.latest(1, typeComboBox.type)
    }
    
    CustomButton {
        id: popularButton
        text: "Popular"
        Layout.fillHeight: true
        fontSize:20 * root.aspectRatio
        radius: 20
        Layout.preferredWidth: parent.width * 0.08
        focusPolicy: Qt.NoFocus
        onClicked: {
            app.showExplorer.popular(1, typeComboBox.type)
        }
    }
    CustomComboBox {
        id:providersComboBox
//        Layout.preferredWidth: parent.width * 0.28
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentRadius: 20
        fontSize:20 * root.aspectRatio
        model: showManager
        currentIndex: showManager.rowCount() - 1
        onCurrentIndexChanged: {
            showManager.changeSearchProvider(currentIndex)
            providersComboBox.currentIndex = showManager.rowCount() - 1

        }
    }
    CustomComboBox {
        id:typeComboBox
//        Layout.preferredWidth: parent.width * 0.15
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentRadius: 20
        fontSize:20 * root.aspectRatio
        model: ListModel{
            id:showTypeModel
        }
        property var typeName: ["Movie", "Tv Series", "Variety", "Anime", "Documentary", "None"];
        property int type
        Connections {
            target:showManager
            function onSearchProviderChanged(){
                showTypeModel.clear()
                for (let showType of showManager.availableShowTypes)
                {
                    showTypeModel.append({text : typeComboBox.typeName[showType - 1], type: showType})
                }
                if (showManager.availableShowTypes.includes(typeComboBox.type)){
                    for (var i = 0; i < showTypeModel.count; ++i)
                    {
                        if (showTypeModel.get(i).type === typeComboBox.type)
                        {
                            typeComboBox.currentIndex = i
                        }
                    }
                }
                else
                {
                    typeComboBox.currentIndex = 0
                    typeComboBox.type = showTypeModel.get(currentIndex).type
                    typeComboBox.displayText = showTypeModel.get(currentIndex).text
                }
            }
        }

        currentIndex: 0
        onCurrentIndexChanged: {
            if (showTypeModel.count === 0) return
            typeComboBox.type = showTypeModel.get(typeComboBox.currentIndex).type
            typeComboBox.displayText = showTypeModel.get(typeComboBox.currentIndex).text
        }
        Component.onCompleted: {
            for (let showType of showManager.availableShowTypes)
            {
                showTypeModel.append({text : typeComboBox.typeName[showType - 1], type: showType})
            }
            type = showManager.availableShowTypes[0]
            displayText = showTypeModel.get(currentIndex).text
        }
    }




    
}



