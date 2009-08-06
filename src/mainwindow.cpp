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

#include "mainwindow.h"
#include "systemtrayicon.h"
#include "translationwidget.h"
#include "settings.h"
#include "generalsettings.h"
#include <KAction>
#include <KToggleAction>
#include <KActionCollection>
#include <KApplication>
#include <KLocale>
#include <KMenuBar>
#include <KStandardAction>
#include <KStatusBar>
#include <KShortcutsDialog>
#include <KIcon>
#include <KDebug>
#include <KConfigDialog>
#include <QClipboard>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

MainWindow::MainWindow() : KXmlGuiWindow(), m_systemTrayIcon(0), m_translationWidget(0) {
    // we do not want the application to exit when the window is closed
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    // the order is important here, do not change it!
    setupActions();
    setupGUI();
    initGui();
    
    //showStatusMessage(i18n("Dictionary version %1", m_translationWidget->dictionaryVersion()));
}

MainWindow::~MainWindow() {
    saveSettings();
}

/**
 * Create all required actions.
 */
void MainWindow::setupActions() {
    KStandardAction::close(this, SLOT(hide()), actionCollection());
    //KStandardAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    
    KStandardAction::showMenubar(this, SLOT(toggleMenuBar()), actionCollection());
    KStandardAction::preferences(this, SLOT(showPreferences()), actionCollection());
    
    KAction* translateClipboardAction = new KAction(this);
    translateClipboardAction->setText(i18n("&Translate Clipboard Content"));
    translateClipboardAction->setIcon(KIcon("kding_babelfish"));
    translateClipboardAction->setToolTip(i18n("Translates the current content of the clipboard."));
    translateClipboardAction->setStatusTip(i18n("Translates the current content of the clipboard."));
    translateClipboardAction->setObjectName("kding_translate_clipboard");
    translateClipboardAction->setGlobalShortcut(KShortcut(Qt::SHIFT + Qt::META + Qt::Key_T));
    actionCollection()->addAction("kding_translate_clipboard", translateClipboardAction);
    connect(translateClipboardAction, SIGNAL(triggered(bool)), this, SLOT(translateClipboard()));
    
    KAction* translateWordAction = new KAction(this);
    translateWordAction->setText(i18n("Translate &Word..."));
    translateWordAction->setIcon(KIcon("kding_babelfish"));
    translateWordAction->setToolTip(i18n("Opens a dialog where you can enter the word to be translated."));
    translateWordAction->setStatusTip(i18n("Opens a dialog where you can enter the word to be translated."));
    translateWordAction->setObjectName("kding_translate_word");
    translateWordAction->setGlobalShortcut(KShortcut(Qt::SHIFT + Qt::META + Qt::Key_W));
    actionCollection()->addAction("kding_translate_word", translateWordAction);
    connect(translateWordAction, SIGNAL(triggered(bool)), this, SLOT(translateWord()));
    
    KAction* configureGlobalShortcutsAction = new KAction(this);
    configureGlobalShortcutsAction->setText(i18n("Configure &Global Shortcuts..."));
    configureGlobalShortcutsAction->setIcon(KIcon("configure-shortcuts"));
    actionCollection()->addAction("configure_global_shortcuts", configureGlobalShortcutsAction);
    connect(configureGlobalShortcutsAction, SIGNAL(triggered(bool)), this, SLOT(showShortcuts()));
}

/**
 * Create and set up all of the GUI widgets.
 */
void MainWindow::initGui() {
    // create the statusbar
    statusBar();
    
    // create the main view
    m_translationWidget = new TranslationWidget(this);
    setCentralWidget(m_translationWidget);
    m_translationWidget->show();
    connect(m_translationWidget, SIGNAL(statusMessage(QString)), this, SLOT(showStatusMessage(QString)));
    
    // create system tray icon
    m_systemTrayIcon = new SystemTrayIcon(this);
    m_systemTrayIcon->show();
    
    // restore settings
    if(Settings::self()->centerWindow()) {
        centerWindow();
    }
    
    if(Settings::self()->rememberPosition()) {
        move(Settings::self()->position());
    }
}

void MainWindow::translateClipboard() {
    QClipboard* clipboard = KApplication::clipboard();
    
    m_translationWidget->clearDisplay();
    
    if(!isVisible()) {
        show();
    }
    
    activateWindow();
    raise();
    
    m_translationWidget->translate(clipboard->text(QClipboard::Selection));
}

void MainWindow::translateWord() {
    if(!isVisible()) {
        show();
    }
    
    activateWindow();
    raise();
    
    m_translationWidget->focusInputWidget();
}

void MainWindow::translate(QString phrase) {
    m_translationWidget->clearDisplay();
    
    if(!isVisible()) {
        show();
    }
    
    activateWindow();
    raise();
    
    m_translationWidget->translate(phrase);
}

void MainWindow::showStatusMessage(QString message) {
    statusBar()->showMessage(message);
}

void MainWindow::clearStatusMessage() {
    statusBar()->clearMessage();
}

void MainWindow::showPreferences() {
    // an instance of the config dialog could already be created and cached,
    // try showing it first
    if(KConfigDialog::showDialog("settings")) {
        return;
    }
    
    // create an instance of the config dialog
    KConfigDialog* dialog = new KConfigDialog(this, "settings", Settings::self());
    dialog->addPage(new GeneralSettings(), i18n("General Settings"), "configure");
    connect(dialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(saveSettings()));
    connect(dialog, SIGNAL(settingsChanged(const QString&)), m_systemTrayIcon, SLOT(updateSettings()));
    
    dialog->show();
}

void MainWindow::showShortcuts() {
    KActionCollection globalActions((QObject*)this);
    
    globalActions.addAction("kding_translate_clipboard", actionCollection()->action("kding_translate_clipboard"));
    globalActions.addAction("kding_translate_word", actionCollection()->action("kding_translate_word"));
    
    KShortcutsDialog::configure(&globalActions);
}

void MainWindow::centerWindow() {
    QRect desktop = QApplication::desktop()->availableGeometry(this);
    
    move(desktop.width() / 2 - width() / 2, desktop.height() / 2 - height() / 2);
}

void MainWindow::toggleMenuBar() {
    if(menuBar()->isVisible()) {
        menuBar()->hide();
    } else {
        menuBar()->show();
    }
}

void MainWindow::saveSettings() {
    Settings::self()->setPosition(pos());
    
    Settings::self()->writeConfig();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // allow the window to be hidden by pressing Escape
    if(event->key() == Qt::Key_Escape) {
        hide();
    }
}

#include "mainwindow.moc"
