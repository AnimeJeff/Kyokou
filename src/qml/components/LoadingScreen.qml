import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {
    id: overlay
    color: "#80000000"
    visible: loading
    z: parent.z + 1
    signal cancelled()
    signal timedOut()
    property bool loading:false
    property bool timeoutEnabled:true
    property int timeoutInterval:5000
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
        }
    }
}

