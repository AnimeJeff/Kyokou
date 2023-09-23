import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    header: Label {
        text: qsTr("Settings")
        font.pixelSize: 20
        padding: 10
    }
    ColumnLayout{
        anchors.fill: parent
        Row{
            Layout.preferredHeight: 20
            Layout.fillWidth: true
            spacing: 10
            Label{
                text: "General"
            }
            Label{
                text: "Proxy"
            }
            Label{
                text: "Play"
            }
        }
        Row{
            Layout.fillHeight: true
            Layout.fillWidth: true
            TextField{

            }
            Button{
                text: "Download"
                onClicked: {
                    let url = "https://api.nivodz.com/play/mud.m3u8/WEB/1.0?ce=34aec2363f0f1b29582a6320e34910cd5b125f1839de237fa736b2b07adc40299915d382de6aa74f757dbb72264f230c6d2748b964c69651675be7b9bcf8a0d1c8b86992c98944488b4052847613da844ac43074d87643d9&is_ad=1&pf=3&uk=2d28e124b97a0da76549626ae1a6eaa5&rx=6728&expire=1694013607478&ip=12.195.86.32&sign=05e4699f637f8b9cb0286a8f9ae41545&_ts=1693988407478"
                    app.downloader.downloadM3u8(url,"第一序列_91",'D:\\TV\\temp',url,"www.nivod4.tv/")

                }
            }
        }

        SwipeView{
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Item{
                Rectangle{
                    color: "red"
                    anchors.fill: parent
                }
            }

            Item{
                Rectangle{
                    color: "orange"
                    anchors.fill: parent
                }
            }
        }

    }



}
