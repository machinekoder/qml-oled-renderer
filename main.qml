import QtQuick 2.6

Item {
    id: root
    visible: true
    width: 128
    height: 64

    Rectangle {
        property bool isRight: false

        id: indicator
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        x: (isRight ? 5 : (root.width - width) - 5)
        width: 15
        height: width
        radius: width / 2
        color: "black"

        Behavior on x {
            PropertyAnimation {
                duration: 500
            }
        }

        Timer {
            id: moveTimer
            running: true
            repeat: true
            interval: 500
            onTriggered: indicator.isRight = !indicator.isRight
        }
    }

    Text {
        anchors.centerIn: parent
        text: "Hello World!"
    }
}
