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

#ifndef TRANSLATIONWIDGET_H
#define TRANSLATIONWIDGET_H

#include "ui_translationwidgetbase.h"
#include <QIcon>
#include <QString>
#include <QWidget>

class SearchEngine;
class HtmlGenerator;

/**
 * This class implements the translation widget.
 * The translation widget consists of an input field to enter the phrase to
 * translate, and a HTML view to display the results. It manages the whole
 * translation process by employing functionality provided by the
 * @c SearchEngine and @c HtmlGenerator classes.
 */
class TranslationWidget : public QWidget, public Ui::TranslationWidgetBase {
    Q_OBJECT

public:
    TranslationWidget(QWidget* parent = 0);
    ~TranslationWidget();
    
    void clearDisplay();
    void focusInputWidget();
    QString dictionaryVersion() const;
    bool checkSearchTerm(QString phrase) const;

public slots:
    void translate(QString phrase);
    void scrollResults(int delta, Qt::Orientation orientation);
    
signals:
    void statusMessage(QString message);
    
private slots:
    void clearText();
    void startSearch();
    void stopSearch();
    void anotherSearchRunning();
    void processSearchResults();
    void saveSettings();
    void updateSearchBackend();
    
private:
    void initGui();
    void displayHtml(QString html);
    
    SearchEngine* m_searchEngine;
    HtmlGenerator* m_htmlGenerator;
    QIcon m_buttonTranslateIcon;
};

#endif
