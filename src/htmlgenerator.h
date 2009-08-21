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

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include "resultlist.h"
#include <KUrl>
#include <QObject>
#include <QString>
#include <QRegExp>

/**
 * This class implements the HTML Generator.
 * The HTML Generator is responsible for creating the HTML code that is
 * displayed in the @c TranslationWidget 's @c KHTMLPart. It uses different
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
    KUrl styleSheetUrl() const;
    
private:
    static const QRegExp SPLITTER;
    static const QRegExp OPEN_BRACKETS;
    static const QRegExp CLOSE_BRACKETS;
    
    const QString CSS_FILE;
    const QString WELCOME_FILE;
    const QString RESULT_FILE;
    const QString NO_MATCHES_FILE;
    
    QString loadFile(const QString filename) const;
    
    int m_fontSize;
    QString m_fontFamily;
    QString m_textColor;
    QString m_alternateBackgroundColor;
};

#endif
