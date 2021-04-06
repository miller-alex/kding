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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <KStatusNotifierItem>

class MainWindow;
class KHelpMenu;

/**
 * This class implements the system tray icon.
 */
class SystemTrayIcon : public KStatusNotifierItem {
    Q_OBJECT

public:
    SystemTrayIcon(MainWindow* parent);
    ~SystemTrayIcon();

signals:
    void translateClipboardRequested(bool &flag);

private slots:
    void activate(const QPoint &pos);
    void activateSecondary(const QPoint &pos);
    void updateSettings();
    
private:
    void initGui();
    void createMenu(MainWindow* parent);
    
    bool m_translateOnLeftClick;
    KHelpMenu* m_helpMenu;
};

#endif
