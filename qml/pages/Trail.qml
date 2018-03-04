import QtQuick 2.0
import Sailfish.Silica 1.0

Flickable {
    id: trail

    property TrailModel model

    contentWidth: row.width
    contentHeight: height

    clip: true

    onContentWidthChanged: {
        contentX = Math.max(0, contentWidth - width);
    }

    Row {
        id: row

        width: childrenRect.width
        height: parent.height

        Repeater {
            model: trail.model

            delegate: Button {
                text: model.name

                onClicked: {
                    trail.model.pop(trail.model.count - (index + 1));
                }
            }
        }
    }
}
