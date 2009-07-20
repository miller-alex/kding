/*
 * Copyright (c) 2009 Michael Rex <me@rexi.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "application.h"
#include "mainwindow.h"
#include <KCmdLineArgs>
#include <KDebug>

Application::Application() : m_mainWindow(0) {
    
}

Application::~Application() {
    delete m_mainWindow;
}

int Application::newInstance() {
    KUniqueApplication::newInstance();
    
    if(m_mainWindow == 0) {
        kdDebug() << "No MainWindow";
        m_mainWindow = new MainWindow();
    }
    
    // handle command line arguments
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    
    if(args->count() > 0) {
        m_mainWindow->translate(args->arg(0));
    }
    
    args->clear();
    
    // exit value for the calling process
    return 0;
}

#include "application.moc"
