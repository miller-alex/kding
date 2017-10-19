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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>
#include <QString>

class SystemTrayIcon;
class TranslationWidget;
class QKeyEvent;

/**
 * This class implements the main window of the application.
 */
class MainWindow : public KXmlGuiWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public slots:
    void translateClipboard(bool &flag);
    void translateClipboard();
    void translateWord();
    void translate(QString phrase);
    //void showStatusMessage(QString message);
    //void clearStatusMessage();

protected:
    void keyPressEvent(QKeyEvent* event);

private slots:
    void showPreferences();
    void showShortcuts();
    void toggleMenuBar();
    void saveSettings();
    void iconActivated(bool active);

private:
    void initGui();
    void setupActions();
    void showRaised();
    void updatePosition(bool initial = false);

    SystemTrayIcon* m_systemTrayIcon;
    TranslationWidget* m_translationWidget;
};

#endif
