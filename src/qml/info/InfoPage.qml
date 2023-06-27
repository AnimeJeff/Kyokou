import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15
import "./components"
Item{
    id:infoPage
    LoadingScreen{
        id:loadingScreen
        z:10
        anchors.fill: parent
        loading: app.playlistModel.loading
    }

    focus: false
    property real labelFontSize: Math.sqrt(window.width*window.height * 1/2000) //20
    ColumnLayout{
        anchors.fill: parent
        spacing: 0
        RowLayout{
            Layout.fillWidth: true
            Layout.preferredHeight: infoPage.height*2/5
            id: topRow
            spacing: 10
            ColumnLayout{
                Image {
                    id: posterImage
                    source: app.currentShowObject.hasShow ? app.currentShowObject.coverUrl : "qrc:/resources/images/error_image.png"
                    onStatusChanged: if (posterImage.status === Image.Null) source = "qrc:/resources/images/error_image.png"
                    Layout.preferredWidth: window.width/5
                    Layout.preferredHeight: (window.width/5) * 432/305
                }
                Layout.preferredWidth: window.width/5
                CustomComboBox{
                    id:libraryComboBox
                    Layout.preferredWidth: window.width/5
//                        Layout.fillHeight: true
                    currentIndex: app.currentShowObject.listType+1
                    displayText: model.get(currentIndex).text.length !== 0 ? model.get(currentIndex).text : app.currentShowObject.isInWatchList ? "Remove from library" : "Add to library"
                    delegate: ItemDelegate {
                        width: libraryComboBox.width
                        //        enabled: !(root.popup.opened && index === 0 && !app.currentShowObject.isInWatchList) || index !== 0
                        //        visible: !(root.popup.opened && index === 0 && !app.currentShowObject.isInWatchList) || index !== 0
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                if(index!==0){
                                    app.watchList.addCurrentShow(index-1)
                                }else{
                                    if(app.currentShowObject.isInWatchList){
                                        app.watchList.removeCurrentShow()
                                    }
                                }
                                libraryComboBox.popup.close()
                            }
                        }
                        contentItem: Text {
                            text: model.text.length !== 0 ? model.text : app.currentShowObject.isInWatchList ? "Remove from library" : "Add to library"
                            color: libraryComboBox.highlightedIndex === index ? "white" : "black"
                            font.family: "Arial"
                            elide: Text.ElideRight
                            font.pixelSize: libraryComboBox.fontSize
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            width: parent.width
                            height: parent.height
                            color: libraryComboBox.highlightedIndex === index ? libraryComboBox.checkedColor : "#F3F4F5"
                        }
                    }
                    visible: app.currentShowObject.hasShow
                    model: ListModel{
                        ListElement { text: "" }
                        ListElement { text: "Watching" }
                        ListElement { text: "Planned" }
                        ListElement { text: "On Hold" }
                        ListElement { text: "Dropped" }
                        ListElement { text: "Completed" }
                    }
                }

            }

            ColumnLayout{
                id: infoColumnLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: window.height/72
                Layout.alignment: Qt.AlignTop
                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: font.pixelSize * 2
                    id:titleText
                    text: app.currentShowObject.title
                    font.pixelSize: Math.sqrt(window.width*window.height * 1/1300) //24
                    color: "white"
                    wrapMode: Text.Wrap
                }

                Label {
                    id:descText
                    text: "Description"
                    color: "white"
                    font.bold: true
                    font.pixelSize: labelFontSize
                    Layout.preferredHeight: labelFontSize
                }

                Flickable{
                    interactive: true
                    Layout.topMargin: -infoColumnLayout.spacing + 10
                    boundsBehavior: Flickable.StopAtBounds
                    contentHeight: descriptionLabel.contentHeight + 10
                    Layout.preferredHeight: Math.min(descriptionLabel.contentHeight+ 10,100)
                    Layout.fillWidth: true
                    clip: true
                    Label {
                        property alias fontSize : descriptionLabel.font.pixelSize
                        id:descriptionLabel
                        text: app.currentShowObject.desc
                        anchors.fill: parent
                        height: contentHeight
                        color: "white"
                        wrapMode: Text.Wrap
                        font.pixelSize: Math.sqrt(window.width*window.height * 1/3000) //16
                    }
                }

                RowLayout {
                    id:ratingViewsText
                    Label {
                        id:scoresText
                        text: "<b>Scores:</b> " + app.currentShowObject.rating + "/10"
                        font.pixelSize: labelFontSize
                        Layout.fillWidth: true
                        color: "white"
                    }

                    Label {
                        id:viewsText
                        text: "<b>Views:</b> " + app.currentShowObject.views
                        font.pixelSize: labelFontSize
                        Layout.fillWidth: true
                        color: "white"
                    }
                }
                Label {
                    id:statusText
                    Layout.fillWidth: true
                    text: "<b>Status:</b> " + app.currentShowObject.status
                    font.pixelSize: labelFontSize
                    color: "white"
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        id:dateAiredText
                        Layout.fillWidth: true
                        text: "<b>Date aired:</b> " + app.currentShowObject.year
                        font.pixelSize: labelFontSize
                        color: "white"
                    }
                    Label {
                        id:updateTimeText
                        Layout.fillWidth: true
                        text: "<b>Update Time:</b> " + app.currentShowObject.updateTime
                        font.pixelSize: labelFontSize
                        color: "white"
                    }
                }


                Label {
                    text: "Genres"
                    color: "white"
                    font.bold: true
                    font.pixelSize: labelFontSize
                }

                Label {
                    Layout.fillWidth: true
                    text: app.currentShowObject.genresString
                    font.pixelSize: descriptionLabel.fontSize
                    color: "white"
                }

                CustomButton{
                    id:continueWatchingButton
                    visible: app.currentShowObject.lastWatchedIndex !== -1
                    text:"Continue from " + app.episodeListModel.continueEpisodeName
                    onClicked: {
                        app.loadSourceFromList(app.episodeListModel.reversed ? episodeList.count - app.episodeListModel.continueIndex - 1 : app.episodeListModel.continueIndex)
                    }
//                    enabled:episodeList.count >= app.currentShowObject.lastWatchedIndex;
                    Layout.preferredHeight: 60
                    Layout.preferredWidth: 60*3
                }

            }
        }

        EpisodeList{
            id:episodeList
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }


}

