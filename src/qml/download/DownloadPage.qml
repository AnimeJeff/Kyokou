import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15

Item {
    id:infoPage

    focus: false
    ListView {
        id:list
        anchors.fill: parent
        ScrollBar.vertical: ScrollBar {
            active: true
        }
        clip: true
        model:app.downloader
//        model:ListModel{
//            ListElement{
//                name:"download item 1"
//                progressValue:100
//                progressText:"test"
//                path:"path"
//            }
//            ListElement{
//                name:"download item 2"
//                progressValue:50
//                progressText:"test"
//                path:"path"
//            }
//        }

        boundsMovement: Flickable.StopAtBounds
        spacing: 10
        delegate: Rectangle {
            width: list.width
            height: 20 * root.aspectRatio * 3
            color: "black"
            required property int progressValue;
            required property string progressText;
            required property string name;
            required property string path;
            GridLayout
            {
                anchors.fill: parent
                rows:3
                columns: 2
                Text {
                    id:episodeStr
                    text:  name + "\t" + path
                    font.pixelSize: 20 * root.aspectRatio
                    Layout.columnSpan: 2
                    wrapMode: Text.Wrap
                    color: "white"
                }
                Text {
                    text:  progressText
                    font.pixelSize: 20 * root.aspectRatio
                    Layout.columnSpan: 2
                    wrapMode: Text.Wrap
                    color: "white"
                }
                ProgressBar {
                    from: 0
                    to: 100
                    value: progressValue
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }
            }
        }

    }





}

