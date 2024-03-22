import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
Item {
    id:infoPage
    FolderDialog {
        id:folderDialog
        currentFolder: "file:///" + workDirTextField.text
        onAccepted:
        {
            showManager.downloader.workDir = text
            text = showManager.downloader.workDir
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 5
            Layout.preferredHeight: 1
            CustomTextField {
                id: workDirTextField
                text: showManager.downloader.workDir
                checkedColor: "#727CF5"
                color: "white"
                placeholderText: qsTr("Enter working directory")
                placeholderTextColor: "gray"
                font.pixelSize: 20 * root.aspectRatio
                onAccepted: () => {
                                showManager.downloader.workDir = text
                                text = showManager.downloader.workDir
                            }

                Layout.row: 0
                Layout.column: 0
                Layout.fillWidth: true
                Layout.preferredWidth: 8 // Use weight to allocate 80% of the space
            }
            CustomButton {
                Layout.row: 0
                Layout.column: 1
                text: "Browse"
                onClicked: folderDialog.open()
                Layout.fillWidth: true
                Layout.preferredWidth: 1 // Use weight to allocate 80% of the space
            }
            CustomButton {
                text: "Open"
                onClicked: Qt.openUrlExternally("file:///" + workDirTextField.text)
                Layout.fillWidth: true
                Layout.preferredWidth: 1
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.fillHeight: true
            spacing: 5
            CustomTextField {
                id: downloadNameField
                checkedColor: "#727CF5"
                color: "white"
                placeholderText: qsTr("Enter filename")
                placeholderTextColor: "gray"
                font.pixelSize: 20 * root.aspectRatio
                Layout.row: 1
                Layout.column: 0
                Layout.fillWidth: true
                Layout.preferredWidth: 3
            }

            CustomTextField {
                id: downloadUrlField
                checkedColor: "#727CF5"

                color: "white"
                placeholderText: qsTr("Enter m3u8 link")
                placeholderTextColor: "gray"
                font.pixelSize: 20 * root.aspectRatio
                Layout.row: 1
                Layout.column: 1
                Layout.fillWidth: true
                Layout.preferredWidth: 7
            }

            CustomButton{
                Layout.row: 1
                Layout.column: 2
                text: "Download"
                onClicked: {showManager.downloader.downloadLink(downloadNameField.text, downloadUrlField.text)}
                Layout.fillWidth: true
                Layout.preferredWidth: 1
            }
        }

        ListView {
            id:listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 8
            clip: true
            model:showManager.downloader
            boundsMovement: Flickable.StopAtBounds
            spacing: 10
            delegate: Rectangle {
                width: listView.width
                height: 120
                border.width: 3
                border.color: "white"
                color: "black"
                required property int progressValue;
                required property string progressText;
                required property string name;
                required property string path;
                required property int index;
                GridLayout {
                    anchors{
                        left:parent.left
                        leftMargin: parent.border.width + 2
                        right:parent.right
                        rightMargin: parent.border.width + 2
                        top:parent.top
                        topMargin: parent.border.width + 2
                        bottom:parent.bottom
                        bottomMargin: parent.border.width + 2
                    }
                    rows:4
                    columns: 3
                    rowSpacing: 10
                    Text {
                        Layout.row: 0
                        Layout.column: 0
                        Layout.columnSpan: 2
                        id: nameStr
                        text:  name
                        font.pixelSize: 20 * root.aspectRatio

                        wrapMode: Text.Wrap
                        color: "white"
                    }
                    Text {
                        Layout.row: 1
                        Layout.column: 0
                        Layout.columnSpan: 2
                        id: pathStr
                        text: path
                        font.pixelSize: 20 * root.aspectRatio
                        wrapMode: Text.Wrap
                        color: "white"
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onClicked: {
                                showManager.downloader.openFolder(path);
                            }
                        }
                    }

                    CustomButton{
                        Layout.row: 0
                        Layout.column: 2
                        Layout.rowSpan: 4
                        text: "Cancel"
                        onClicked: showManager.downloader.cancelTask(index)
                    }


                    ProgressBar {
                        Layout.row: 2
                        Layout.column: 0
                        Layout.columnSpan: 2
                        from: 0
                        to: 100
                        value: progressValue

                        Layout.fillWidth: true
                    }
                    Text {
                        Layout.row: 3
                        Layout.column: 0
                        text:  progressText
                        font.pixelSize: 20 * root.aspectRatio
                        Layout.columnSpan: 2
                        wrapMode: Text.Wrap
                        color: "white"
                    }
                }


            }
        }
    }






}

