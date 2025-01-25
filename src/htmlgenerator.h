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

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include "resultlist.h"
#include <QUrl>
#include <QObject>
#include <QString>
#include <QRegExp>

/**
 * This class implements the HTML Generator.
 * The HTML Generator is responsible for creating the HTML code that is
 * displayed in the @c TranslationWidget 's @c QTextBrowser. It uses different
 * templates to generate the HTML code, depending on what to display.
 *
 * @see welcomePage()
 * @see resultPage()
 * @see noMatchesPage()
 * @see emptyPage()
 */
class HtmlGenerator : public QObject {
    Q_OBJECT

public:
    HtmlGenerator(int fontSize, QObject* parent = 0);
    ~HtmlGenerator();

    QString welcomePage() const;
    QString emptyPage() const;
    QString resultPage(const QString searchTerm, const ResultList resultList) const;
    QString noMatchesPage() const;
    QUrl styleSheetUrl() const;
    static QStringList resourcePaths();
    
private:
    static void formatFragments(QStringList &list, const QRegExp &SearchTerm);

    int m_fontSize;
    QString m_fontFamily;
    QString m_textColor;
    QString m_alternateBackgroundColor;
};

#endif
