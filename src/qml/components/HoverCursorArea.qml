import QtQuick

MouseArea {
    property int hoveredCursor: Qt.PointingHandCursor
    anchors.fill: parent
    hoverEnabled: true
    onEntered: cursor.setCursorShape(hoveredCursor)
    onExited: cursor.setCursorShape(Qt.ArrowCursor)
    acceptedButtons: Qt.NoButton
}
