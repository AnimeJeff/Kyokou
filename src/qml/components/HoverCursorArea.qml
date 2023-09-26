import QtQuick

MouseArea {
    property int hoveredCursor: Qt.PointingHandCursor
    anchors.fill: parent
    hoverEnabled: true
    onEntered: app.cursor.shape = hoveredCursor
    onExited: app.cursor.shape = Qt.ArrowCursor
    acceptedButtons: Qt.NoButton
}
