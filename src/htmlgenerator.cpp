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
#include "searchengine.h"
#include <KStandardDirs>
#include <KLocale>
#include <KAboutData>
#include <KComponentData>
#include <KGlobal>
#include <KGlobalSettings>
#include <KDebug>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QFont>

const QRegExp HtmlGenerator::SPLITTER = QRegExp("(.*) :: (.*)");    // capture left and right side in groups
const QRegExp HtmlGenerator::OPEN_BRACKETS = QRegExp(" ([[{])");    // match opening brackets: { [
const QRegExp HtmlGenerator::CLOSE_BRACKETS = QRegExp("([]}])");    // match closing brackets: } ]

HtmlGenerator::HtmlGenerator(int fontSize, QObject* parent) : QObject(parent), CSS_FILE(KStandardDirs::locate("appdata", "html/kding.css")), WELCOME_FILE(KStandardDirs::locate("appdata", "html/welcome.html")), RESULT_FILE(KStandardDirs::locate("appdata", "html/result.html")), NO_MATCHES_FILE(KStandardDirs::locate("appdata", "html/nomatches.html")), m_fontSize(fontSize), m_fontFamily(KGlobalSettings::generalFont().family()) {
    
}

HtmlGenerator::~HtmlGenerator() {
    
}

QString HtmlGenerator::welcomePage() const {
    QString html = loadFile(WELCOME_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        SearchEngine searchEngine;
        
        // replace placeholders
        QString infocss = KStandardDirs::locate("data", "kdeui/about/kde_infopage.css"); // %1
        QString rtlcss = QApplication::isRightToLeft() ? QString("@import \"%1\"").arg(KStandardDirs::locate("data", "kdeui/about/kde_infopage_rtl.css")) : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString appTitle = KGlobal::mainComponent().aboutData()->programName(); // %4
        QString catchPhrase = i18n("Dictionary Lookup for KDE"); // %5
        QString shortDescription = KGlobal::mainComponent().aboutData()->shortDescription(); // %6
        QString description = i18n("Using dictionary version %1", searchEngine.dictionaryVersion()); // %7
        QString welcomeCss = KStandardDirs::locate("appdata", "html/welcome.css"); // %8
        
        html = html.arg(infocss).arg(rtlcss).arg(fontSize).arg(appTitle).arg(catchPhrase).arg(shortDescription).arg(description).arg(welcomeCss);
    }
    
    return html;
}

QString HtmlGenerator::emptyPage() const {
    return "<html><body></body></html>";
}

QString HtmlGenerator::resultPage(const QString searchTerm, const ResultList resultList) const {
    QString html = loadFile(RESULT_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        // create the table
        QRegExp term("(" + searchTerm + ")", Qt::CaseInsensitive);  // match the search term
        int bgClass = 0;    // counter used for alternating background colors
        QString table = "";
        
        foreach(ResultItem item, resultList) {
            QString text = item.text();
            
            text.replace(term, "<span class=\"keyword\">\\1</span>");
            text.replace("|", "<br>&nbsp;&nbsp;");
            text.replace(OPEN_BRACKETS, "&nbsp;<i>\\1");
            text.replace(CLOSE_BRACKETS, "\\1</i>");
            if(SPLITTER.indexIn(text) != -1) {
                QString german = SPLITTER.cap(1).trimmed();
                QString english = SPLITTER.cap(2).trimmed();
                
                table += "<tr><td class=\"line" + QString::number(bgClass % 2)
                    + "\">" + german + "</td><td class=\"line"
                    + QString::number(bgClass % 2)
                    + "\">" + english + "</td></tr>";
                
                ++bgClass;
            } else {
                kError() << "Bad line: splitter not found in" << text;
            }
        }
        
        // replace placeholders
        QString fontSize = QString::number(m_fontSize); // %3
        QString captionGerman = i18nc("result table caption", "German"); // %4
        QString captionEnglish = i18nc("result table caption", "English"); // %5
        
        html = html.arg(CSS_FILE).arg(m_fontFamily).arg(fontSize).arg(captionGerman).arg(captionEnglish).arg(table);
    }
    
    return html;
}

QString HtmlGenerator::noMatchesPage() const {
    QString html = loadFile(NO_MATCHES_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        // replace placeholders
        QString infocss = KStandardDirs::locate("data", "kdeui/about/kde_infopage.css"); // %1
        QString rtlcss = QApplication::isRightToLeft() ? QString("@import \"%1\"").arg(KStandardDirs::locate("data", "kdeui/about/kde_infopage_rtl.css")) : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString message = i18n("No matches found"); // %74
        
        html = html.arg(infocss).arg(rtlcss).arg(fontSize).arg(message);
    }
    
    return html;
}

KUrl HtmlGenerator::styleSheetUrl() const {
    return KUrl::fromPath(CSS_FILE);
}

QString HtmlGenerator::loadFile(const QString filename) const {
    QString contents = "";
    QFile file(filename);
    
    if(file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        contents = stream.readAll();
        file.close();
    } else {
        kError() << "Failed to open" << filename;
    }
    
    return contents;
}

#include "htmlgenerator.moc"
