import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0

PreviewItem {

    Audio {
        id: audio
        source: fileInfo.uri

        onPositionChanged: {
            if (! slider.down)
            {
                slider.value = position;
            }
        }

        onDurationChanged: {
            slider.maximumValue = duration;
        }
    }

    IconButton {
        anchors.bottom: slider.top
        anchors.horizontalCenter: parent.horizontalCenter

        icon.source: audio.playbackState === Audio.PlayingState
                     ? "image://theme/icon-l-pause"
                     : "image://theme/icon-l-play"

        onClicked: {
            if (audio.playbackState === Audio.PlayingState)
            {
                audio.pause();
            }
            else
            {
                audio.play();
            }
        }
    }

    Slider {
        id: slider

        function toTime(s)
        {
            s /= 1000;
            var seconds = Math.floor(s) % 60;
            s /= 60;
            var minutes = Math.floor(s) % 60;
            s /= 60;
            var hours = Math.floor(s);

            if (seconds < 10)
            {
                seconds = "0" + seconds;
            }
            if (minutes < 10)
            {
                minutes = "0" + minutes;
            }

            if (hours > 0)
            {
                return hours + ":" + minutes + ":" + seconds;
            }
            else
            {
                return minutes + ":" + seconds;
            }
        }

        width: parent.width
        anchors.bottom: parent.bottom

        handleVisible: false
        minimumValue: 0
        valueText: toTime(value)

        onDownChanged: {
            if (! down)
            {
                audio.seek(value);
            }
        }
    }

}
