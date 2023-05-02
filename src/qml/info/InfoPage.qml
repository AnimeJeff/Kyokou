import QtQuick 2.15
import QtQuick.Controls 2.15
import "./../components"
import QtQuick.Layouts 1.15
Item{
    id:infoPage
    LoadingScreen{
        id:loadingScreen
        z:10
        anchors.fill: parent
        loading: app.playlistModel.loading
    }
    Connections{
        target: global.currentShowObject
        function onListTypeChanged(){
            //            addToLibraryComboBox.model.setProperty(0,"text",)

        }
        function onShowChanged(){
            //            console.log(global.currentShowObject.lastWatchedIndex)
            if(global.currentShowObject.lastWatchedIndex >-1){

            }
        }
    }
    focus: false


    ColumnLayout{
        anchors.fill: parent
        spacing: 0
        RowLayout{
            Layout.fillWidth: true
            Layout.preferredHeight: infoPage.height*2/5
            id: topRow
            spacing: 10
            Image {
                id: posterImage
                source: global.currentShowObject.hasShow ? global.currentShowObject.coverUrl : "qrc:/resources/images/error_image.png"
                onStatusChanged: if (posterImage.status === Image.Null) source = "qrc:/resources/images/error_image.png"
                Layout.preferredWidth: window.width/5
                Layout.preferredHeight: (window.width/5) * 432/305
            }
            ColumnLayout{
                id: infoColumnLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: window.height/72
                Layout.alignment: Qt.AlignTop
                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: font.pixelSize
                    id:titleText
                    text: global.currentShowObject.title
                    font.pixelSize: Math.sqrt(window.width*window.height * 1/1300) //24
                    color: "white"
                    wrapMode: Text.Wrap
                }

                Label {
                    id:descText
                    text: "Description"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.sqrt(window.width*window.height * 1/2000) //20
                    Layout.preferredHeight: descText.font.pixelSize
                }

                Flickable{
                    interactive: true
                    Layout.topMargin: -infoColumnLayout.spacing + 10
                    boundsBehavior: Flickable.StopAtBounds
                    contentHeight: descriptionLabel.contentHeight + 10
                    Layout.preferredHeight: 100
                    Layout.fillWidth: true
                    clip: true
                    Label {
                        property alias fontSize : descriptionLabel.font.pixelSize
                        id:descriptionLabel
                        text: global.currentShowObject.desc
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
                        id:ratingText
                        text: "<b>Rating:</b> " + global.currentShowObject.rating + "/10"
                        font.pixelSize: descriptionLabel.fontSize
                        Layout.fillWidth: true
                        color: "white"
                    }

                    Label {
                        text: "<b>Views:</b> " + global.currentShowObject.views.toLocaleString()
                        font.pixelSize: descriptionLabel.fontSize
                        Layout.fillWidth: true
                        color: "white"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        id:yearText
                        Layout.fillWidth: true
                        text: "<b>Year:</b> " + global.currentShowObject.year
                        font.pixelSize: descriptionLabel.fontSize
                        color: "white"
                    }
                    Label {
                        Layout.fillWidth: true
                        text: "<b>Status:</b> " + global.currentShowObject.status
                        font.pixelSize: descriptionLabel.fontSize
                        color: "white"
                    }
                    Label {
                        Layout.fillWidth: true
                        text: "<b>Update Time:</b> " + global.currentShowObject.updateTime
                        font.pixelSize: descriptionLabel.fontSize
                        color: "white"
                    }
                }

                Label {
                    text: "Genres"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 20
                }

                Label {
                    Layout.fillWidth: true
                    text: global.currentShowObject.genresString
                    font.pixelSize: descriptionLabel.fontSize
                    color: "white"
                }

                RowLayout{
                    spacing: 10
                    Layout.preferredHeight: 60
                    CustomComboBox{
                        id:libraryComboBox
                        Layout.preferredWidth: 200
//                        Layout.fillHeight: true
                        currentIndex: global.currentShowObject.listType+1
                        displayText: model.get(currentIndex).text.length !== 0 ? model.get(currentIndex).text : global.currentShowObject.isInWatchList ? "Remove from library" : "Add to library"
                        delegate: ItemDelegate {
                            width: libraryComboBox.width
                            //        enabled: !(root.popup.opened && index === 0 && !global.currentShowObject.isInWatchList) || index !== 0
                            //        visible: !(root.popup.opened && index === 0 && !global.currentShowObject.isInWatchList) || index !== 0
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if(index!==0){
                                        app.watchList.addCurrentShow(index-1)
                                    }else{
                                        if(global.currentShowObject.isInWatchList){
                                            app.watchList.removeCurrentShow()
                                        }
                                    }
                                    libraryComboBox.popup.close()
                                }
                            }
                            contentItem: Text {
                                text: model.text.length !== 0 ? model.text : global.currentShowObject.isInWatchList ? "Remove from library" : "Add to library"
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
                        visible: global.currentShowObject.hasShow
                        model: ListModel{
                            ListElement { text: "" }
                            ListElement { text: "Watching" }
                            ListElement { text: "Planned" }
                            ListElement { text: "On Hold" }
                            ListElement { text: "Dropped" }
                        }
                    }

                    CustomButton{
                        id:continueWatchingButton
                        visible: global.currentShowObject.lastWatchedIndex !== -1
                        text:"Continue from " + app.episodeListModel.continueEpisodeName
                        onClicked: {
                            app.loadSourceFromList(app.episodeListModel.reversed ? episodeListView.count - app.episodeListModel.continueIndex - 1 : app.episodeListModel.continueIndex)
                        }
                    }


                }
            }
        }

        Rectangle{
            id:listHeader
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "grey"
            RowLayout{
                anchors.fill: parent
                Label{
                    id: countLabel
                    Layout.alignment: Qt.AlignTop
                    //                Layout.fillHeight: true
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    text: episodeListView.count + " episodes"
                    font.bold: true
                    color: "white"
                    font.pixelSize: 25
                    visible: episodeListView.count > 0
                }
                Button{
                    Layout.fillHeight: true

                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 100
                    id:reverseButton
                    text: "reverse"

                    font.pixelSize: 25
                    onClicked: {
                        app.episodeListModel.reversed = !app.episodeListModel.reversed
                    }
                    visible: true
                    background: Rectangle{
                        color: "white"
                    }

                }
            }

        }

        ListView{
            onCountChanged: {
                if(global.currentShowObject.lastWatchedIndex>-1){
                    episodeListView.positionViewAtIndex(global.currentShowObject.lastWatchedIndex,ListView.Beginning)
                }
            }
            id:episodeListView
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model:app.episodeListModel
            boundsMovement: Flickable.StopAtBounds
            delegate: Rectangle {
                id: delegateRect
                width: episodeListView.width
                height: 50 < (episodeStr.height + 10) ? (episodeStr.height + 10) : 50
                color: {
                    if(global.currentShowObject.lastWatchedIndex === -1)return "black"
                    let lastWatchedIndex = app.episodeListModel.reversed ? episodeListView.count -  global.currentShowObject.lastWatchedIndex - 1 : global.currentShowObject.lastWatchedIndex;
                    if(lastWatchedIndex === index){
                        return "red"
                    }
                    return "black"
                }
                Text {
                    id:episodeStr
                    text:  "Episode " + model.number.toString() + (model.title === undefined || parseInt(model.title) === model.number ? "" : "\n" + model.title)
                    font.pixelSize: 16

                    anchors{
                        left:parent.left
                        right:parent.right

                        top:parent.top
                        leftMargin: 10
                        rightMargin: 10
                        topMargin: 10
                        bottomMargin: 10
                    }
                    wrapMode: Text.Wrap
                    color: "white"
                    font.family: "Arial"
                }
                MouseArea {
                    anchors.fill: delegateRect
                    //                onEntered: delegateRect.color = "#ccc"
                    //                onExited: delegateRect.color = "#f2f2f2"
                    onClicked: (mouse)=>{
                                   app.loadSourceFromList(app.episodeListModel.reversed ? episodeListView.count - index - 1 : index)
                               }
                }

            }
            ScrollBar.vertical: ScrollBar {
                active: true
            }

        }

    }


}

