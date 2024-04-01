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
    property real labelFontSize: 24 * root.fontSizeMultiplier
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
        font.pixelSize: 26 * root.fontSizeMultiplier
        anchors {
            top: parent.top
            left:posterImage.right
            right: episodeList.left
        }
        MouseArea{
            anchors.fill: parent
            onClicked:  Qt.openUrlExternally(`https://anilist.co/search/anime?search=${encodeURIComponent(titleText.text)}`);
            cursorShape: Qt.PointingHandCursor
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
            font.pixelSize: 22 * root.fontSizeMultiplier
        }
    }

    CustomComboBox {
        id:libraryComboBox
        anchors {
            top: posterImage.bottom
            left: parent.left
            right: posterImage.right
            topMargin: 5
        }
        focus: false
        activeFocusOnTab: false
        height: parent.height * 0.07

        currentIndex: app.currentShow.listType + 1
        Component.onCompleted: {
            if (app.currentShow.inWatchList)
                listTypeModel.set(0, {text: "Remove from Library"})
            else
                listTypeModel.set(0, {text: "Add to Library"})
        }

        fontSize: 20
        onActivated: (index) => {
                         if (index === 0) {
                             app.removeCurrentShowFromLibrary()
                             listTypeModel.set(0, {text: "Add to Library"})
                         } else {
                             app.addCurrentShowToLibrary(index - 1)
                             listTypeModel.set(0, {text: "Remove from Library"})
                         }
                     }

        model: ListModel{
            id: listTypeModel
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
        fontSize: 20
        radius: height
        anchors {
            top: descriptionBox.bottom
            horizontalCenter: descriptionBox.horizontalCenter
            topMargin: 10
        }
        width: descriptionBox.width * 0.5
        height: libraryComboBox.height
    }
    Keys.enabled: true
    Keys.onPressed: (event) => {
                        switch (event.key){
                            case Qt.Key_Space:
                            if (continueWatchingButton.visible) {
                                app.continueWatching();
                            }
                            break
                        }
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

            Layout.preferredHeight: implicitHeight
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.preferredHeight: implicitHeight
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            // Layout.preferredHeight: implicitHeight
            Text {
                text: "<b>GENRE(S):</b>"
                font.pixelSize: 24 * root.fontSizeMultiplier
                color: "white"
                Layout.fillHeight: true
            }
            Text {
                text: currentShow.genresString
                font.pixelSize: 23 * root.fontSizeMultiplier
                color: "white"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                Layout.fillHeight: true

            }
        }

        // Text {
        //     text: `<b>GENRE(S):</b> <font size="-1.0">${currentShow.genresString}</font>`
        //     color: "white"
        //     font.bold: true
        //     font.pixelSize: labelFontSize

        //     Layout.fillWidth: true
        // }


    }





}

