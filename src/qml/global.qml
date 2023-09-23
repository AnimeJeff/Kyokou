pragma Singleton
import MpvPlayer 1.0
import QtQuick 2.0

QtObject {
    property MpvObject mpv
    property real searchResultViewLastY
    property real watchListViewLastY
    property string lastSearch:""
}
