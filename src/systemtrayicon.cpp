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

#include "systemtrayicon.h"
#include "mainwindow.h"
#include "settings.h"
#include <KAboutData>
#include <KAction>
#include <KComponentData>
#include <KGlobal>
#include <KHelpMenu>
#include <KIcon>
#include <KMenu>
#include <KActionCollection>
#include <KDebug>
#include <QMenu>

SystemTrayIcon::SystemTrayIcon(MainWindow* parent) : KSystemTrayIcon("kding", parent) {
    initGui();
    createMenu();
    updateSettings();
}

SystemTrayIcon::~SystemTrayIcon() {
    
}

void SystemTrayIcon::initGui() {
    setToolTip(i18n("KDing - translation tool"));
    
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(handleClicks(QSystemTrayIcon::ActivationReason)));
}

void SystemTrayIcon::createMenu() {
    KActionCollection* actionCollection = static_cast<MainWindow*>(parentWidget())->actionCollection();
    QMenu* menu = contextMenu();
    
    menu->addAction(actionCollection->action("kding_translate_clipboard"));
    menu->addAction(actionCollection->action("kding_translate_word"));
    menu->addSeparator();
    menu->addAction(actionCollection->action("configure_global_shortcuts"));
    menu->addAction(actionCollection->action("options_configure"));
    menu->addSeparator();
    
    KHelpMenu* helpMenu = new KHelpMenu(parentWidget(), KGlobal::mainComponent().aboutData(), false);
    menu->addMenu(helpMenu->menu());
}

/**
 * This method decides what to do when the user clicks on the system tray icon.
 *
 * @param reason the type of click the system tray icon received
 */
void SystemTrayIcon::handleClicks(QSystemTrayIcon::ActivationReason reason) {
    // Trigger and Context are handled by KSystemTrayIcon, so only MiddleClick
    // has to be handled here, unless TranslateOnLeftClick is enabled. In that
    // case, we need to handle Trigger ourself, too
    switch(reason) {
        case QSystemTrayIcon::MiddleClick:
            if(m_translateOnLeftClick) {
                toggleActive();
            } else {
                static_cast<MainWindow*>(parentWidget())->translateClipboard();
            }
            break;
        case QSystemTrayIcon::Unknown:
        case QSystemTrayIcon::Context:
        case QSystemTrayIcon::DoubleClick:
            // fallthrough intended to suppress compiler warnings
            // because of unhandled enumeration values
            break;
        case QSystemTrayIcon::Trigger:
            if(m_translateOnLeftClick) {
                static_cast<MainWindow*>(parentWidget())->translateClipboard();
            }
            break;
    }
}

void SystemTrayIcon::updateSettings() {
    // locally cached so it doesn't have to be read in every time the icon is
    // clicked
    m_translateOnLeftClick = Settings::self()->translateOnLeftClick();
}

#include "systemtrayicon.moc"
