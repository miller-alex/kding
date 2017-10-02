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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "systemtrayicon.h"
#include "mainwindow.h"
#include "settings.h"
#include <KAboutData>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KMenu>
#include <KActionCollection>
#include <KDebug>
#include <QAction>
#include <QMenu>

SystemTrayIcon::SystemTrayIcon(MainWindow* parent)
  : KStatusNotifierItem(parent), m_helpMenu(0)
{
    initGui();
    createMenu(parent);
    updateSettings();
}

SystemTrayIcon::~SystemTrayIcon() {
    delete m_helpMenu;
}

void SystemTrayIcon::initGui() {
    setIconByName("kding");
    setStatus(KStatusNotifierItem::Active);
    setToolTipTitle(i18n("KDing - Translation tool"));

    // our base class already calls activate(), so we only have to
    // connect to secondaryActivateRequest()
    connect(this, SIGNAL(secondaryActivateRequested(const QPoint&)),
	    this, SLOT(activateSecondary(const QPoint&)));
}

void SystemTrayIcon::createMenu(MainWindow* parent) {
    KActionCollection* actionCollection = parent->actionCollection();
    QMenu* menu = contextMenu();

    menu->addAction(actionCollection->action("kding_translate_clipboard"));
    menu->addAction(actionCollection->action("kding_translate_word"));
    menu->addSeparator();
    menu->addAction(actionCollection->action("configure_global_shortcuts"));
    menu->addAction(actionCollection->action("options_configure"));
    menu->addSeparator();

    m_helpMenu = new KHelpMenu(parent, KAboutData::applicationData(), false);
    menu->addMenu(m_helpMenu->menu());
}

void SystemTrayIcon::activate(const QPoint &pos) {
    if (m_translateOnLeftClick) {
        emit translateClipboardRequested();
    } else {
        KStatusNotifierItem::activate(pos);
    }
}

void SystemTrayIcon::activateSecondary(const QPoint &pos) {
    if (m_translateOnLeftClick) {
        KStatusNotifierItem::activate(pos);
    } else {
        emit translateClipboardRequested();
    }
}

void SystemTrayIcon::updateSettings() {
    // locally cached so it doesn't have to be read in every time the icon is
    // clicked
    m_translateOnLeftClick = Settings::self()->translateOnLeftClick();
}

#include "systemtrayicon.moc"
