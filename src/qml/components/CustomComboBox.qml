import QtQuick 2.0
import QtQuick.Controls 2.0


ComboBox {
    id: comboBox

    property color checkedColor: "#1ABC9C"
    property int fontSize: 15
    property var onClickedFun

    delegate: ItemDelegate {
        width: comboBox.width

//        MouseArea{
//            anchors.fill: parent
//            onClicked: {
//                popup.close()
//                comboBox.currentIndex = index
//            }
//        }
        contentItem: Text {
            text: model.text
            color: comboBox.highlightedIndex === index ? "white" : "black"
            font.family: "Arial"
            elide: Text.ElideRight
            font.pixelSize: fontSize
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
             width: parent.width
             height: parent.height
             color: comboBox.highlightedIndex === index ? comboBox.checkedColor : "#F3F4F5"
         }
    }

    indicator: Canvas {
        id: canvas
        x: comboBox.width - width - 10
        y: (comboBox.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: comboBox
            function onPressedChanged(){
                canvas.requestPaint()
            }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = "white"
            context.fill();
        }
    }

    contentItem: Item {
        width: comboBox.background.width - comboBox.indicator.width - 10
        height: comboBox.background.height

        Text {
            anchors.verticalCenter: parent.verticalCenter
            x: 10
            text: comboBox.displayText
            elide: Text.ElideRight

            font.pixelSize: fontSize
            font.family: "Arial"
            font.weight: Font.Thin
            color: comboBox.down ? Qt.rgba(255, 255, 255, 0.75) : "white"
        }
    }

    background: Rectangle {
        implicitWidth: 102
        implicitHeight: 41
        color: comboBox.down ? Qt.darker(comboBox.checkedColor, 1.2) : comboBox.checkedColor
        radius: 5

        layer.enabled: comboBox.hovered | comboBox.down
//        layer.effect: DropShadow {
//            transparentBorder: true
//            color: comboBox.checkedColor
//            samples: 10 /*20*/
//        }
    }

    popup: Popup {
        y: comboBox.height - 1
        width: comboBox.width
        implicitHeight: contentItem.implicitHeight
        padding: 0

        contentItem: ListView {
            implicitHeight: contentHeight
            model: comboBox.popup.visible ? comboBox.delegateModel : null
            clip: true
            currentIndex: comboBox.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }
        background: Rectangle {
            color: "#F3F4F5"
            radius: 5
            clip: true
            layer.enabled: comboBox.hovered | comboBox.down
        }
    }
}
