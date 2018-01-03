/* =================================================
 * This file is part of the TTK Music Player project
 * Copyright (C) 2015 - 2017 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ================================================= */

import QtQuick 2.5
import QtQuick.Controls 1.4
import "Core"

Rectangle{
    id: ttkMusicAboutPage

    visible: false
    anchors.fill: parent
    color: ttkTheme.color_alpha_lv12

    MouseArea {
        anchors.fill: parent
        onClicked: {
            ttkMusicAboutPage.visible = false;
        }
    }

    Rectangle {
        id: mainRectangle
        color: ttkTheme.color_white
        radius: 10
        width: 0.7*parent.width
        height: 0.3*parent.height
        anchors.centerIn: parent

        Text {
            id: textArea
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: parent.height/11
            text: qsTr("TTKMusicPlayer") + "\n\n" +
                  qsTr("Directed By Greedysky") +
                  ("\nCopyright© 2015-2017") +
                  ("\nMail:Greedysky@163.com");
        }

        TTKTextButton {
            anchors {
                bottom: parent.bottom
                left: parent.left
                bottomMargin: ttkGlobal.dpHeight(20)
                leftMargin: ttkGlobal.dpWidth(50)
            }
            textColor: ttkTheme.topbar_background
            text: qsTr("关于")
            onClicked: {
                Qt.openUrlExternally("https://github.com/Greedysky/TTKMusicplayer")
            }
        }

        TTKTextButton {
            anchors {
                bottom: parent.bottom
                right: parent.right
                bottomMargin: ttkGlobal.dpHeight(20)
                rightMargin: ttkGlobal.dpWidth(50)
            }
            textColor: ttkTheme.topbar_background
            text: qsTr("确定")
        }
    }

    Component.onCompleted:
    {
        var docRoot = ttkMusicAboutPage.parent;
        while(docRoot.parent)
        {
            docRoot = docRoot.parent;
        }
        ttkMusicAboutPage.parent = docRoot;
    }
}