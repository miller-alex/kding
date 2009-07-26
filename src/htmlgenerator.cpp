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

#include "htmlgenerator.h"
#include <KStandardDirs>
#include <KLocale>
#include <KDebug>
#include <QFile>
#include <QTextStream>

const QRegExp HtmlGenerator::SPLITTER = QRegExp("(.*) :: (.*)");    // capture left and right side in groups
const QRegExp HtmlGenerator::OPEN_BRACKETS = QRegExp(" ([[{])");    // match opening brackets: { [
const QRegExp HtmlGenerator::CLOSE_BRACKETS = QRegExp("([]}])");    // match closing brackets: } ]

HtmlGenerator::HtmlGenerator(QObject* parent) : QObject(parent), CSS_FILE(KStandardDirs::locate("appdata", "kding.css")), WELCOME_FILE(KStandardDirs::locate("appdata", "welcome.html")) {
    
}

HtmlGenerator::~HtmlGenerator() {
    
}

QString HtmlGenerator::welcomePage() const {
    QString html = emptyPage();
    QFile welcomeFile(WELCOME_FILE);
    
    if(welcomeFile.open(QFile::ReadOnly)) {
        QTextStream stream(&welcomeFile);
        html = stream.readAll();
        welcomeFile.close();
    } else {
        kError() << "Failed to open" << WELCOME_FILE;
    }
    
    return html;
}

QString HtmlGenerator::emptyPage() const {
    return "<html><body></body></html>";
}

QString HtmlGenerator::resultPage(const QString searchTerm, const ResultList resultList) const {
    QString html = "<html><body><table>";
    html += "<tr><th width=\"50%\">";
    html += i18nc("table caption", "German");
    html += "</th><th width=\"50%\">";
    html += i18nc("table caption", "English");
    html += "</th></tr>";
    
    QRegExp term("(" + searchTerm + ")", Qt::CaseInsensitive);  // match the search term
    
    int bgClass = 0;    // counter used for alternating background colors
    
    foreach(ResultItem item, resultList) {
        QString text = item.text();
        
        text.replace(term, "<span class=\"keyword\">\\1</span>");
        text.replace("|", "<br>&nbsp;&nbsp;");
        text.replace(OPEN_BRACKETS, "&nbsp;<i>\\1");
        text.replace(CLOSE_BRACKETS, "\\1</i>");
        if(SPLITTER.indexIn(text) != -1) {
            QString german = SPLITTER.cap(1).trimmed();
            QString english = SPLITTER.cap(2).trimmed();
            
            html += "<tr><td class=\"c" + QString::number(bgClass % 2)
                 + "\">" + german + "</td><td class=\"c"
                 + QString::number(bgClass % 2)
                 + "\">" + english + "</td></tr>";
            
            ++bgClass;
        } else {
            kError() << "Bad line: splitter not found in" << text;
        }
    }
    
    html += "</table></body></html>";

    return html;
}

QString HtmlGenerator::noResultsPage() const {
    QString html = "<html><body><table><tr><td class=\"error\">No results</td></tr></table></body></html>";
    return html;
}

KUrl HtmlGenerator::styleSheetUrl() const {
    return KUrl::fromPath(CSS_FILE);
}

#include "htmlgenerator.moc"
