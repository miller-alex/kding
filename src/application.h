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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class MainWindow;

/**
 * This class implements handling the unique instance of the application.
 */
class Application : public QApplication {
    Q_OBJECT

public:
    Application(int &argc, char **argv) : QApplication(argc, argv) {};
    ~Application();

    static int exec();

public slots:
    static void newInstance(const QStringList &arguments, bool first = false);
    
private:
    static MainWindow *m_mainWindow;
};

#endif
