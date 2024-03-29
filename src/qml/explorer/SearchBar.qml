import QtQuick 2.15
import QtQuick.Controls 2.15
import "../components"
import QtQuick.Layouts 1.15


RowLayout {
    id:searchBar
    property alias textField: searchTextField
    function search(){
        showManager.search(searchTextField.text, 1)
        root.lastSearch = searchTextField.text
        parent.forceActiveFocus()
    }
    function  latest(){
        showManager.latest(1)
    }
    function popular(){
        showManager.popular(1)
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
        onCurrentIndexChanged: showManager.currentProviderIndex = providersComboBox.currentIndex
        text: "text"
        Component.onCompleted: providersComboBox.currentIndex = showManager.currentProviderIndex

    }

    CustomComboBox {
        id:typeComboBox
        Layout.preferredWidth: 2
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentRadius: 20
        fontSize:20 * root.aspectRatio
        model: showManager.availableShowTypes
        currentIndex: showManager.currentSearchTypeIndex
        onCurrentIndexChanged: showManager.currentSearchTypeIndex = typeComboBox.currentIndex
        Component.onCompleted: typeComboBox.currentIndex = showManager.currentSearchTypeIndex
        Connections {
            target: showManager
            function onCurrentSearchTypeIndexChanged(){
                typeComboBox.currentIndex = showManager.currentSearchTypeIndex;
            }
        }

        delegate: ItemDelegate {
            width: typeComboBox.width
            required property string modelData
            required property real index
            contentItem: Text {
                text: modelData
                color: typeComboBox.highlightedIndex === index ? "white" : "black"
                elide: Text.ElideRight
                // font.pixelSize: fontSize
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignHCenter
            }
            background: Rectangle {
                width: parent.width
                height: parent.height
                color: typeComboBox.highlightedIndex === index ? typeComboBox.checkedColor : "#F3F4F5"
                radius: 20
            }
        }
    }




    
}



