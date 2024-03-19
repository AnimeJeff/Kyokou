import QtQuick 2.15
import QtQuick.Controls 2.15
Popup {
    id: overlay
    dim: true
    visible: loading
    signal cancelled()
    signal timedOut()
    property bool loading:false
    property bool timeoutEnabled:true
    property int timeoutInterval:5000
    background: Rectangle {
        color: "black"
    }
    onLoadingChanged: {
        if (loading && timeoutEnabled)
        {
            loadingTimer.start()
        }
        else{
            loadingTimer.stop()
        }
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        preventStealing: true
        onClicked: {
            cancelled()
            overlay.close()
        }
    }


    AnimatedImage {
        anchors.centerIn: parent
        source: "qrc:/resources/gifs/loading-totoro.gif"
        width: 150
        height: width * 1.5
        visible: loading
        playing: loading
    }

    Timer {
        id: loadingTimer
        interval: timeoutInterval
        running: false
        repeat: false

        onTriggered: {
            cancelled()
            timedOut()
            overlay.close()
        }
    }
}

