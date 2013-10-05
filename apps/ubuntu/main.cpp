/*****************************************************************************
 * Copyright: 2011-2013 Michael Zanetti <michael_zanetti@gmx.net>            *
 *                                                                           *
 * This file is part of Xbmcremote                                           *
 *                                                                           *
 * Xbmcremote is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Xbmcremote is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "libxbmcremote/xbmc.h"
#include "libxbmcremote/settings.h"
#include "libxbmcremote/eventclient.h"

#include "ubuntuhelper.h"

#include <QGuiApplication>

#include <QDebug>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDir>

int main(int argc, char** argv)
{
    QGuiApplication::setApplicationName("Xbmcremote");

    QGuiApplication application(argc, argv);


    Xbmc::instance()->setDataPath(QDir::homePath() + "/.cache/com.ubuntu.developer.mzanetti.xbmcremote/");
    Xbmc::instance()->eventClient()->setApplicationThumbnail("xbmcremote80.png");

    QQuickView *view = new QQuickView();
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setTitle("Xbmcremote");
    view->engine()->rootContext()->setContextProperty("xbmc", Xbmc::instance());

    Settings settings("com.ubuntu.developer.mzanetti.xbmcremote");
    view->engine()->rootContext()->setContextProperty("settings", &settings);

    if(QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath()) {
        view->setSource(QUrl::fromLocalFile("/usr/share/xbmcremote/qml/main.qml"));
    } else {
        view->setSource(QUrl::fromLocalFile("qml/main.qml"));
    }

    UbuntuHelper helper(&settings);
    Q_UNUSED(helper);

    if(QGuiApplication::arguments().contains("--fullscreen")) {
        view->showFullScreen();
    } else {
        view->resize(QSize(720, 1280));
        view->show();
    }

    return application.exec();
}

