import QtQuick 2.15
import QtQuick.Controls 2.15
import "../../components"
import QtQuick.Layouts 1.15
Item {
    id:searchBar
    property alias textField: searchTextField
    function search(){
        app.showExplorer.search(searchTextField.text,1,4)
        root.lastSearch = searchTextField.text
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
            text: root.lastSearch
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
        //        ComboBox{
        //            id:providerbox
        //            Layout.fillWidth: true
        //            Layout.fillHeight: true
        //            model: showManager
        //            onCurrentIndexChanged: {
        //                console.log(currentIndex)
        //            }
        //            property color checkedColor: "#1ABC9C"

        //            delegate: ItemDelegate {
        //                width: providerbox.width
        //                contentItem: Text {
        //                    text: modelData//model.text
        //                    color: providerbox.highlightedIndex === index ? "white" : "black"
        //                    font.family: "Arial"
        //                    elide: Text.ElideRight
        //                    font.pixelSize: 14
        //                    verticalAlignment: Text.AlignVCenter
        //                    MouseArea{
        //                        onClicked: {

        //                        }
        //                    }
        //                }
        //                //                MouseArea{
        //                //                    anchors.fill: parent
        //                //                    onClicked: onClickedFun(index,model)
        //                //                }
        //                background: Rectangle {
        //                    width: parent.width
        //                    height: parent.height
        //                    color: providerbox.highlightedIndex === index ? providerbox.checkedColor : "#F3F4F5"
        //                }
        //            }

        //            popup: Popup {
        //                y: root.height - 1
        //                width: root.width
        //                implicitHeight: contentItem.implicitHeight
        //                padding: 0

        //                contentItem: ListView {
        //                    implicitHeight: contentHeight
        //                    model: root.popup.visible ? root.delegateModel : null
        //                    clip: true
        //                    currentIndex: root.highlightedIndex

        //                    ScrollIndicator.vertical: ScrollIndicator { }
        //                }
        //                background: Rectangle {
        //                    color: "#F3F4F5"
        //                    radius: 5
        //                    clip: true
        //                    layer.enabled: root.hovered | root.down
        //                }
        //            }

        //        }


        CustomComboBox{
            id:providersComboBox
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: showManager
            currentIndex: showManager.rowCount() - 1
            onCurrentIndexChanged: {
                showManager.changeSearchProvider(currentIndex)
                providersComboBox.currentIndex = showManager.rowCount()-1

            }
        }

    }


}
