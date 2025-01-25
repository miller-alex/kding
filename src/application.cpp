/*
 * Copyright (c) 2009 Michael Rex <me@rexi.org>
 * Copyright (c) 2017 Alexander Miller <alex.miller@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "application.h"
#include "mainwindow.h"
#include <KDBusService>
#include <KStartupInfo>

MainWindow *Application::m_mainWindow = 0;

Application::~Application() {
    delete m_mainWindow;
    m_mainWindow = 0;
}

int Application::exec() {
    KDBusService dbusService(KDBusService::Unique
			     | KDBusService::NoExitOnFailure);

    // only the first instance reaches this point
    connect(&dbusService,
	    SIGNAL(activateRequested(const QStringList&, const QString&)),
	    instance(), SLOT(newInstance(const QStringList &)));

    newInstance(arguments(), true);

    return QApplication::exec();
}

/**
 * Creates a new instance of the application if required.
 * If there are arguments given on the command line, they are passed to the
 * application to be translated.
 */
void Application::newInstance(const QStringList &arguments, bool first) {
    if (!m_mainWindow) {
        m_mainWindow = new MainWindow();
    }

    if (first) {
        KStartupInfo::appStarted();
    } else {
        KStartupInfo::setNewStartupId(m_mainWindow->windowHandle(), KStartupInfo::startupId());
    }

    // no need for a command line parser here
    // all arguments (except at(0)) form a phrase
    if (arguments.count() > 1) {
        m_mainWindow->translate(arguments.mid(1).join(' '));
    } else if (!first) {
        m_mainWindow->translateWord();
    }
}

#include "moc_application.cpp"
