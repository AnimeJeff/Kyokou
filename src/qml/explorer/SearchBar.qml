import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
import QtQuick.Layouts 1.15


RowLayout {
    id:searchBar
    property alias textField: searchTextField
    function search(){
        showManager.search(encodeURIComponent(searchTextField.text), 1, typeComboBox.type)
        root.lastSearch = searchTextField.text
        parent.forceActiveFocus()
    }
    function latest(){
        showManager.latest(1, typeComboBox.type)
    }
    function popular(){
        showManager.popular(1, typeComboBox.type)
    }

    CustomTextField {
        checkedColor: "#727CF5"
        id:searchTextField
        color: "white"
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: 5
        placeholderText: qsTr("Enter query!")
        placeholderTextColor: "gray"
        text: root.lastSearch
        font.pixelSize: 20 * root.aspectRatio
        onAccepted: search()
    }
    
    CustomButton {
        id: searchButton
        text: "Search"
        Layout.fillHeight: true
        Layout.preferredWidth: 1
        Layout.fillWidth: true
        fontSize:20 * root.aspectRatio
        radius: 20
        focusPolicy: Qt.NoFocus
        onClicked: search()
    }

    CustomButton {
        id: latestButton
        text: "Latest"
        Layout.fillHeight: true
        Layout.fillWidth: true
        radius: 20
        fontSize:20 * root.aspectRatio
        Layout.preferredWidth: 1
        focusPolicy: Qt.NoFocus
        onClicked:latest()
    }
    
    CustomButton {
        id: popularButton
        text: "Popular"
        Layout.fillHeight: true
        Layout.fillWidth: true
        fontSize:20 * root.aspectRatio
        radius: 20
        Layout.preferredWidth: 1
        focusPolicy: Qt.NoFocus
        onClicked: popular()
    }

    CustomComboBox {
        id:providersComboBox
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: 2
        contentRadius: 20
        fontSize:20 * root.aspectRatio

        model: showManager
        currentIndex: 0
        text: "text"
        onCurrentIndexChanged: {
            showManager.changeSearchProvider(currentIndex)
            showTypeModel.clear()
            for (let showType of showManager.availableShowTypes) {
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
            else{
                typeComboBox.currentIndex = 0
                typeComboBox.type = showTypeModel.get(typeComboBox.currentIndex).type
                typeComboBox.displayText = showTypeModel.get(typeComboBox.currentIndex).text
            }

        }
    }

    CustomComboBox {
        id:typeComboBox
        Layout.preferredWidth: 2
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentRadius: 20
        fontSize:20 * root.aspectRatio
        model: ListModel{
            id:showTypeModel
        }
        text: "text"
        property var typeName: ["Movie", "Tv Series", "Variety", "Anime", "Documentary", "None"];
        property int type


        currentIndex: 0
        onCurrentIndexChanged: {
            if (showTypeModel.count === 0) return
            typeComboBox.type = showTypeModel.get(typeComboBox.currentIndex).type
            typeComboBox.displayText = showTypeModel.get(typeComboBox.currentIndex).text
        }
        Component.onCompleted:{
            for (let showType of showManager.availableShowTypes)
            {
                showTypeModel.append({text : typeComboBox.typeName[showType - 1], type: showType})
            }
            type = showManager.availableShowTypes[0]
            displayText = showTypeModel.get(currentIndex).text
        }
    }




    
}



