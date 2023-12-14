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
            app.downloader.workDir = text
            text = app.downloader.workDir
        }
    }
    GridLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        columns: 3
        rows: 2
        CustomTextField {
            id: workDirTextField
            checkedColor: "#727CF5"
            Layout.row: 0
            Layout.column: 0
            Layout.fillWidth: true
            Layout.preferredHeight: infoPage.height*0.05
            Layout.preferredWidth: infoPage.width*0.8
            text: app.downloader.workDir
            color: "white"
            placeholderText: qsTr("Enter query!")
            font.pixelSize: 20 * root.aspectRatio
            onAccepted: () => {
                            app.downloader.workDir = text
                            text = app.downloader.workDir
                        }
        }
        CustomButton{
            Layout.row: 0
            Layout.column: 1
            Layout.preferredWidth: infoPage.width*0.1

            text: "Browse"
            onClicked: folderDialog.open()
        }
        CustomButton{
            Layout.row: 0
            Layout.column: 2
            Layout.preferredWidth: infoPage.width*0.1

            text: "Open"
            onClicked: app.downloader.openFolder("file:///" + workDirTextField.text)
        }

        ListView {
            MouseArea {
                anchors.fill: parent
                onClicked: forceActiveFocus()
            }
            id:list
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.row: 1
            Layout.column: 0
            Layout.columnSpan: 3
            ScrollBar.vertical: ScrollBar {
                active: true
            }
            clip: true
            model:app.downloader
            boundsMovement: Flickable.StopAtBounds
            spacing: 10
            delegate: Rectangle {
                width: list.width
                height: 20 * root.aspectRatio * 4
                color: "black"
                required property int progressValue;
                required property string progressText;
                required property string name;
                required property string path;
                GridLayout
                {
                    anchors.fill: parent
                    rows:4
                    columns: 3
                    rowSpacing: 10
                    Text {
                        Layout.row: 0
                        Layout.column: 0
                        id:episodeStr
                        text:  name
                        font.pixelSize: 20 * root.aspectRatio
                        Layout.columnSpan: 3
                        wrapMode: Text.Wrap
                        color: "white"
                    }
                    Text {
                        Layout.row: 1
                        Layout.column: 0
                        id:pathStr
                        text:  path
                        font.pixelSize: 20 * root.aspectRatio
                        Layout.columnSpan: 3
                        wrapMode: Text.Wrap
                        color: "white"
                    }

                    CustomButton{
                        Layout.row: 2
                        Layout.column: 2
                        text: "Cancel"
                        onClicked: console.log("cancelled")
                        Layout.rowSpan: 2
                    }


                    ProgressBar {
                        Layout.row: 2
                        Layout.column: 0
                        from: 0
                        to: 100
                        value: progressValue
                        Layout.columnSpan: 2
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

