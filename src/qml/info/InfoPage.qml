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
        loading: showManager.playList.loading
    }
    focus: false
    property real aspectRatio: root.width/root.height
    property real labelFontSize: 22 * root.aspectRatio
    property var currentShow: showManager.currentShow
    GridLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignTop
        columns : 3
        rows: 9
        rowSpacing: 5
        Image {
            id: posterImage
            source: showManager.hasCurrentShow ? currentShow.coverUrl : "qrc:/resources/images/error_image.png"
            onStatusChanged: if (posterImage.status === Image.Null) source = "qrc:/resources/images/error_image.png"
            Layout.preferredWidth: root.height * 0.4 * 0.7
            Layout.preferredHeight: root.height * 0.4
            Layout.rowSpan: 7
            Layout.alignment: Qt.AlignVCenter
        }
        Text {
            id: titleText
            text: currentShow.title.toUpperCase()
            font.pixelSize: 24 * root.aspectRatio
            font.bold: true
            color: "white"
            wrapMode: Text.Wrap
            Layout.row: 0
            Layout.column: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            Layout.alignment: Qt.AlignTop
        }
        Text {
            id:descText
            text: "DESCRIPTION"
            color: "white"
            font.bold: true
            font.pixelSize: labelFontSize
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: labelFontSize
            Layout.columnSpan: 2
            Layout.row: 1
            Layout.column: 1
        }
        Flickable{
            Layout.alignment: Qt.AlignTop
            interactive: true
            boundsBehavior: Flickable.StopAtBounds
            contentHeight: descriptionLabel.contentHeight + 10
            clip: true
            Layout.row: 2
            Layout.column: 1
            Layout.columnSpan: 2
            Layout.preferredHeight: Math.min(descriptionLabel.contentHeight + 10,100)
            Layout.fillWidth: true
            Text {
                property alias fontSize : descriptionLabel.font.pixelSize
                id:descriptionLabel
                text: currentShow.description
                anchors.fill: parent
                height: contentHeight
                color: "white"
                wrapMode: Text.Wrap
                font.pixelSize: 20 * root.aspectRatio
            }
        }
        Text {
            id:scoresText
            Layout.alignment: Qt.AlignTop
            text: `<b>SCORE:</b> <font size="-0.5">${currentShow.rating}</font>`
            font.pixelSize: labelFontSize
            Layout.fillWidth: true
            color: "white"
            Layout.row: 3
            Layout.column: 1
            Layout.columnSpan: 2
        }

        Text {
            id:statusText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: `<b>STATUS:</b> <font size="-0.5">${currentShow.status}</font>`
            font.pixelSize: labelFontSize
            color: "white"
            Layout.row: 4
            Layout.column: 1
        }
        Text {
            id:viewsText
            Layout.alignment: Qt.AlignTop
            text: `<b>VIEWS:</b> <font size="-0.5">${currentShow.views}</font>`
            font.pixelSize: labelFontSize
            Layout.fillWidth: true
            color: "white"
            Layout.row: 4
            Layout.column: 2
        }

        Text {
            id:dateAiredText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: `<b>DATE AIRED:</b> <font size="-0.5">${currentShow.releaseDate}</font>`
            font.pixelSize: labelFontSize
            color: "white"
            Layout.row: 5
            Layout.column: 1
        }
        Text {
            id:updateTimeText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: `<b>UPDATE TIME:</b> <font size="-0.5">${currentShow.updateTime}</font>`
            font.pixelSize: labelFontSize
            color: "white"
            Layout.row: 5
            Layout.column: 2
        }
        Text {
            Layout.alignment: Qt.AlignTop
            text: `<b>GENRE(S):</b> <font size="-0.5">${currentShow.genresString}</font>`
            color: "white"
            font.bold: true
            font.pixelSize: labelFontSize
            Layout.row: 6
            Layout.column: 1
            Layout.columnSpan: 2
        }


        CustomComboBox {
            id:libraryComboBox
            Layout.alignment: Qt.AlignBottom
            Layout.row: 7
            Layout.column: 0
            Layout.preferredWidth: posterImage.width
            Layout.preferredHeight: continueWatchingButton.height
            currentIndex: showManager.currentShowListType + 1
            displayText: currentText.length === 0 ?
                             showManager.currentShowIsInWatchList ? "Remove from library" : "Add to library"
                            : currentText
            fontSize:18 * root.aspectRatio

            delegate: ItemDelegate {
                width: libraryComboBox.width
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (index !== 0) {
                            showManager.addCurrentShowToLibrary(index - 1)
                        } else {
                            showManager.removeCurrentShowFromLibrary()
                        }
                        libraryComboBox.popup.close()
                    }
                }
                contentItem: Text {
                    text: model.text ? model.text : showManager.currentShowIsInWatchList ? "Remove from library" : "Add to library"
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
            visible: showManager.hasCurrentShow
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
            Layout.alignment: Qt.AlignBottom
            visible: showManager.episodeList.lastWatchedIndex !== -1
            text: "Continue from " + showManager.episodeList.continueEpisodeName
            onClicked: showManager.continueWatching()
            fontSize: 18 * root.aspectRatio
            Layout.preferredHeight: fontSize * 2.5
            Layout.preferredWidth: libraryComboBox.width * 2
            Layout.row: 7
            Layout.column: 1
        }

        EpisodeList {
            id: episodeList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.row: 8
            Layout.column: 0
            Layout.columnSpan: 3
//            Layout.preferredHeight: infoPage.height * 0.6
        }

    }



}

