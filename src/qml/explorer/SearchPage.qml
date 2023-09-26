import QtQuick 2.15
import QtQuick.Controls 2.15
import "./components"
import "../components"

Item {
    id: searchPage

    SearchBar{
        id:searchBar
        anchors{
            left: parent.left
            right: parent.right
            top:parent.top
        }
        height: 30
    }

    LoadingScreen{
        id:loadingScreen
        anchors.fill: parent
        loading: app.showExplorer.loading || showManager.loading
    }
    Component.onCompleted: {
        list.contentY = searchResultsViewlastScrollY
        forceActiveFocus()
    }

    GridView {
        id: list
        property real aspectRatio:319/225
        property real itemPerRow: Math.floor(root.width/200)
        property real spacing: 10


        boundsBehavior:Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds

        anchors{
            topMargin: list.spacing
            top: searchBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: list.spacing
        }
        model: app.showExplorer
        cellHeight: cellWidth * aspectRatio + 35
        cellWidth: width/itemPerRow

        delegate:  itemDelegate
        highlight: highlight
        highlightFollowsCurrentItem: false
        clip: true
        onContentYChanged: {
            searchResultsViewlastScrollY = contentY
        }

        //        footer: Rectangle{
        //            color: "transparent"
        //            width: parent.width
        //            height: 100
        //            z:list.z+1
        //            BusyIndicator{
        //                running: true
        //                visible: true
        //                width: width
        //                height: parent.height
        //                anchors.centerIn: parent

        //            }
        //        }

        //        interactive: true
        //        property int realContentHeight: Math.ceil(list.count/6)*cellHeight

        onAtYEndChanged: {
            if(atYEnd && count > 0 && app.showExplorer.canLoadMore()){
                app.showExplorer.loadMore();
            }
        }
        onCountChanged: {
            //            if(Math.ceil(list.count/list.itemPerRow)*cellHeight<height && count>0){
            //                app.showExplorer.loadMore();
            //            }
            //            if(list.count<30){
            //                app.showExplorer.loadMore();
            //            }
        }
        //        onContentHeightChanged: {
        //            if( Math.ceil(list.count/list.itemPerRow)*cellHeight<height && count>0){
        //                app.showExplorer.loadMore();
        //            }
        //        }

        Component {
            id: itemDelegate
            Item {
                id: item
                Image {
                    id:coverImage
                    source:  model.cover// : "qrc:/kyokou/images/error_image.png"
                    onStatusChanged: if (coverImage.status === Image.Error) source = "qrc:/resources/images/error_image.png"
                    width: list.width/list.itemPerRow -list.spacing
                    height: width * list.aspectRatio
                    BusyIndicator {
                        id: busyIndicator
                        anchors.centerIn: parent
                        running: coverImage.status == Image.Loading
                        visible: coverImage.status == Image.Loading
                        z:-1 //100
                        height: 70
                        width: 70
                    }

                    MouseArea{
                        anchors.fill: parent
                        onClicked: (mouse)=>{
                                       app.showExplorer.loadDetails(index)
                                   }
                        hoverEnabled: true
                        onEntered: app.cursor.shape = Qt.PointingHandCursor
                        onExited: app.cursor.shape = Qt.ArrowCursor
                    }
                }
                Text {
                    text: model.title
                    font.bold: ListView.isCurrentItem
                    anchors.top: coverImage.bottom
                    anchors.bottom: parent.bottom
                    width: list.cellWidth
                    wrapMode: Text.Wrap
                    font.pixelSize: 12
                    height: contentHeight
                    color: "white"
                    MouseArea{
                        anchors.fill: parent
                        onClicked: (mouse)=>{
                                       if (mouse.button === Qt.RightButton){
                                           app.showExplorer.loadDetails(index)
                                       }
                                   }
                    }
                }
            }
        }
    }



    Keys.enabled: true
    Keys.onPressed: event => handleKeyPress(event)
    function handleKeyPress(event){

        switch (event.key) {
        case Qt.Key_Escape:
        case Qt.Key_Alt:
            if(searchBar.textField.activeFocus)
                searchPage.forceActiveFocus()
            break;
        case Qt.Key_Enter:
            searchBar.search()
            break;
        case Qt.Key_Slash:
            searchBar.textField.forceActiveFocus()
            break;
        case Qt.Key_P:
            app.showExplorer.popular(1,4)
            break;
        case Qt.Key_L:
            app.showExplorer.latest(1,4)
            break;
        case Qt.Key_Up:
            list.flick(0,500)
            break;
        case Qt.Key_Down:
            list.flick(0,-500)
            break;

        }

    }
}
