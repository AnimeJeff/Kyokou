import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15

Item {
    id:infoPage
    LoadingScreen {
        id:loadingScreen
        z:10
        anchors.fill: parent
        loading: app.playlist.loading
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
        }
        Flickable{
            Layout.alignment: Qt.AlignTop
            interactive: true
            boundsBehavior: Flickable.StopAtBounds
            contentHeight: descriptionLabel.contentHeight + 10
            clip: true
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
            text: "<b>SCORE:</b> " + currentShow.rating + "/10"
            font.pixelSize: labelFontSize
            Layout.fillWidth: true
            color: "white"
        }
        Text {
            id:viewsText
            Layout.alignment: Qt.AlignTop
            text: "<b>VIEWS:</b> " + currentShow.views
            font.pixelSize: labelFontSize
            Layout.fillWidth: true
            color: "white"
        }
        Text {
            id:statusText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: "<b>STATUS:</b> " + currentShow.status
            font.pixelSize: labelFontSize
            color: "white"
            Layout.columnSpan: 2
        }

        Text {
            id:dateAiredText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: "<b>DATE AIRED:</b> " + currentShow.releaseDate
            font.pixelSize: labelFontSize
            color: "white"
        }
        Text {
            id:updateTimeText
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            text: "<b>UPDATE TIME:</b> " + currentShow.updateTime
            font.pixelSize: labelFontSize
            color: "white"
        }
        Text {
            Layout.alignment: Qt.AlignTop
            text: "GENRE(S):"
            color: "white"
            font.bold: true
            font.pixelSize: labelFontSize
        }

        Text {
            //                    Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            text: currentShow.genresString
            font.pixelSize: descriptionLabel.fontSize
            color: "white"
        }

        CustomComboBox {
            id:libraryComboBox
            Layout.alignment: Qt.AlignBottom
            Layout.preferredWidth: posterImage.width
            Layout.preferredHeight: continueWatchingButton.height
            currentIndex: showManager.currentShowListType + 1
            displayText: currentText.length === 0 ?
                             showManager.currentShow.isInWatchList ? "Remove from library" : "Add to library"
                            : currentText
            fontSize:18 * root.aspectRatio

            delegate: ItemDelegate {
                width: libraryComboBox.width
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (index !== 0)
                        {
                            app.watchList.addCurrentShow(index-1)
                        }
                        else
                        {
                            if (showManager.currentShowIsInWatchList)
                            {
                                app.watchList.removeCurrentShow()
                            }
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
            visible: showManager.currentShowLastWatchedIndex !== -1
            text: "Continue from " + app.episodeListModel.continueEpisodeName
            onClicked: {
                app.playlist.continueFromLastWatched()
            }
            fontSize: 18 * root.aspectRatio
            Layout.preferredHeight: fontSize * 2.5
            Layout.preferredWidth: libraryComboBox.width * 2
            Layout.columnSpan: 2
        }

        EpisodeList {
            Layout.columnSpan: 3
            id:episodeList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
//            Layout.preferredHeight: infoPage.height * 0.6
        }
    }



}

