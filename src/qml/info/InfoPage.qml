import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15

Item {
    id:infoPage
    LoadingScreen {
        id:loadingScreen
        z:10
        anchors.centerIn: parent
        loading: app.playList.loading
    }
    focus: false
    property real aspectRatio: root.width/root.height
    property real labelFontSize: 24 * root.aspectRatio
    property var currentShow: app.currentShow


    Image {
        id: posterImage
        source: app.currentShow.exists ? currentShow.coverUrl : "qrc:/resources/images/error_image.png"
        onStatusChanged: if (posterImage.status === Image.Null) source = "qrc:/resources/images/error_image.png"

        anchors{
            top: parent.top
            left: parent.left
        }
        width: parent.width * 0.2
        height: parent.height * 0.4
    }

    EpisodeList {
        id: episodeList
        Layout.fillHeight: true
        Layout.fillWidth: true
        anchors{
            right:parent.right
            top:parent.top
            bottom: parent.bottom
        }
        width: parent.width * 0.3

    }

    Text {
        id: titleText
        text: currentShow.title.toUpperCase()
        font.bold: true
        color: "white"
        wrapMode: Text.Wrap
        font.pixelSize: 26 * root.aspectRatio
        anchors {
            top: parent.top
            left:posterImage.right
            right: episodeList.left
        }
        height: contentHeight
    }

    Flickable{
        id:descriptionBox
        interactive: true
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: descriptionLabel.contentHeight + 10
        clip: true
        anchors{
            left: posterImage.right
            right: episodeList.left
            top: titleText.bottom
            bottom: continueWatchingButton.visible ? posterImage.bottom : libraryComboBox.bottom
        }

        Text {
            id:descriptionLabel
            text: currentShow.description
            anchors.fill: parent
            height: contentHeight
            color: "white"
            wrapMode: Text.Wrap
            font.pixelSize: 22 * root.aspectRatio
        }
    }

    CustomComboBox {
        id:libraryComboBox
        anchors {
            top: posterImage.bottom
            left: parent.left
            right: posterImage.right
            topMargin: 10
        }
        height: parent.height * 0.05

        currentIndex: app.currentShow.listType + 1
        displayText: currentText.length === 0 ?
                         app.currentShow.inWatchList ? "Remove from library" : "Add to library" : currentText

        fontSize: 20 * root.aspectRatio

        delegate: ItemDelegate {
            width: libraryComboBox.width
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (index !== 0) {
                        app.addCurrentShowToLibrary(index - 1)
                    } else {
                        app.removeCurrentShowFromLibrary()
                    }
                    libraryComboBox.popup.close()
                }
            }
            contentItem: Text {
                text: model.text ? model.text : app.currentShow.inWatchList ? "Remove from library" : "Add to library"
                color: libraryComboBox.highlightedIndex === index ? "white" : "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize:libraryComboBox.fontSize
            }

            background: Rectangle {
                width: parent.width
                height: parent.height
                color: libraryComboBox.highlightedIndex === index ? libraryComboBox.checkedColor : "#F3F4F5"
            }
        }
        visible: app.currentShow.exists
        model: ListModel{
            ListElement { text: "" }
            ListElement { text: "Watching" }
            ListElement { text: "Planned" }
            ListElement { text: "On Hold" }
            ListElement { text: "Dropped" }
            ListElement { text: "Completed" }
        }
    }

    CustomButton {
        id:continueWatchingButton
        Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        visible: app.currentShow.episodeList.lastWatchedIndex !== -1
        text: "Continue from " + app.currentShow.episodeList.continueEpisodeName
        onClicked: app.continueWatching()
        fontSize: 20 * root.aspectRatio
        radius: height
        anchors {
            top: descriptionBox.bottom
            horizontalCenter: descriptionBox.horizontalCenter
            topMargin: 10
        }
        width: parent.width * 0.2
        height: libraryComboBox.height
    }


    ColumnLayout{
        anchors {
            top: libraryComboBox.bottom
            left: parent.left
            right: episodeList.left

        }

        Text {
            id:scoresText
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            text: `<b>SCORE:</b> <font size="-0.5">${currentShow.rating}</font>`
            font.pixelSize: labelFontSize
            Layout.preferredHeight: implicitHeight
            //Layout.fillHeight: true
            Layout.fillWidth: true

            color: "white"

        }

        Text {
            id:statusText
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

            text: `<b>STATUS:</b> <font size="-0.5">${currentShow.status}</font>`
            font.pixelSize: labelFontSize
            color: "white"



            // Layout.preferredWidth: 2
            Layout.preferredHeight: implicitHeight
            //Layout.fillHeight: true
            Layout.fillWidth: true

        }
        Text {
            id:viewsText
            Layout.alignment: Qt.AlignTop | Qt.AlignCenter
            text: `<b>VIEWS:</b> <font size="-0.5">${currentShow.views}</font>`
            font.pixelSize: labelFontSize

            color: "white"


            // Layout.preferredWidth: 2
            Layout.preferredHeight: implicitHeight
            //Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Text {
            id:dateAiredText
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

            text: `<b>DATE AIRED:</b> <font size="-0.5">${currentShow.releaseDate}</font>`
            font.pixelSize: labelFontSize
            color: "white"


            Layout.preferredHeight: implicitHeight
            //Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Text {
            id:updateTimeText
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

            text: `<b>UPDATE TIME:</b> <font size="-0.5">${currentShow.updateTime}</font>`
            font.pixelSize: labelFontSize
            color: "white"

            // Layout.preferredWidth: 2
            Layout.preferredHeight: implicitHeight
            //Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Text {
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            text: `<b>GENRE(S):</b> <font size="-1.0">${currentShow.genresString}</font>`
            color: "white"
            font.bold: true
            font.pixelSize: labelFontSize
            Layout.preferredHeight: implicitHeight
            Layout.fillWidth: true

        }


    }





}

