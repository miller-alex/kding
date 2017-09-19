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
#include <K4AboutData>
#include <KComponentData>
#include <KGlobal>
#include <KGlobalSettings>
#include <KColorScheme>
#include <KDebug>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QFont>
#include <QColor>
#include <QPalette>
#include <QBrush>

const QRegExp HtmlGenerator::SPLITTER = QRegExp("(.*) ::(?: |&nbsp;)(.*)");    ///< capture left and right side of a line (the German and the English part) in groups
const QRegExp HtmlGenerator::OPEN_BRACKETS = QRegExp("([[{])");    ///< match opening brackets: { [
const QRegExp HtmlGenerator::CLOSE_BRACKETS = QRegExp("([]}])");    ///< match closing brackets: } ]

/**
 * This creates a new instance of @c HtmlGenerator.
 * It takes the font size to be used in the HTML code and, optionally, a parent
 * object for this object.
 *
 * @param fontSize font size in pixels
 * @param parent parent object
 */
HtmlGenerator::HtmlGenerator(int fontSize, QObject* parent) : QObject(parent), CSS_FILE(KStandardDirs::locate("appdata", "html/kding.css")), WELCOME_FILE(KStandardDirs::locate("appdata", "html/welcome.html")), RESULT_FILE(KStandardDirs::locate("appdata", "html/result.html")), NO_MATCHES_FILE(KStandardDirs::locate("appdata", "html/nomatches.html")), m_fontSize(fontSize), m_fontFamily(KGlobalSettings::generalFont().family()) {
    // set the foreground and alternate background colors for the result page
    // to the one from KDE's current color scheme
    KColorScheme colorScheme(QPalette::Normal);
    
    QColor color = colorScheme.foreground().color();
    m_textColor = QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    
    color = colorScheme.background(KColorScheme::AlternateBackground).color();
    m_alternateBackgroundColor = QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

HtmlGenerator::~HtmlGenerator() {
    
}

/**
 * This method generates the HTML code of the welcome page from the template
 * and returns it.
 *
 * @return HTML code of the welcome page
 */
QString HtmlGenerator::welcomePage() const {
    QString html = loadFile(WELCOME_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        SearchEngine searchEngine;
        
        // replace placeholders
        QString infocss = QUrl::fromLocalFile(KStandardDirs::locate("data", "kf5/infopage/kde_infopage.css")).toString(); // %1
        QString rtlcss = QApplication::isRightToLeft() ? QString("@import \"%1\"").arg(QUrl::fromLocalFile(KStandardDirs::locate("data", "kf5/infopage/kde_infopage_rtl.css")).toString()) : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString appTitle = KGlobal::mainComponent().aboutData()->programName(); // %4
        QString catchPhrase = i18n("Dictionary Lookup for KDE"); // %5
        QString shortDescription = KGlobal::mainComponent().aboutData()->shortDescription(); // %6
        QString description = i18n("Using dictionary version %1", searchEngine.dictionaryVersion()); // %7
        QString welcomeCss = QUrl::fromLocalFile(KStandardDirs::locate("appdata", "html/welcome.css")).toString(); // %8

        html = html.arg(infocss).arg(rtlcss).arg(fontSize).arg(appTitle).arg(catchPhrase).arg(shortDescription).arg(description).arg(welcomeCss);
    }
    
    return html;
}

/**
 * This method generates the HTML code of an empty page and returns it.
 *
 * @return HTML code of an empty page
 */
QString HtmlGenerator::emptyPage() const {
    return "<html><body></body></html>";
}

/**
 * This method generates the HTML code of the result page from the template
 * and returns it.
 * It uses @p searchTerm to determine what phrase to highlight.
 *
 * @param searchTerm the phrase to highlight
 * @param resultList @c #ResultList containing the matches found
 *
 * @return HTML code of the result page
 */
QString HtmlGenerator::resultPage(const QString searchTerm, const ResultList resultList) const {
    QString html = loadFile(RESULT_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        // create the table
        QRegExp term("(" + searchTerm + ")", Qt::CaseInsensitive);  // match the search term
        QRegExp space_itag(" +(<i>)");
        int bgClass = 0;    // counter used for alternating background colors
        QString table = "";
        
        foreach(ResultItem item, resultList) {
            QString text = item.text();
            // mark search term before escaping, replace with tags afterwards;
            // use newline as special marker since text is a single line
            text = text.replace(term, "\n[\\1\n]").toHtmlEscaped();
            text.replace("\n[", "<span class=\"keyword\">").replace("\n]", "</span>");

            text.replace("|", "<br>&nbsp;&nbsp;");
            text.replace(OPEN_BRACKETS, "<i>\\1").replace(space_itag, "&nbsp;\\1");
            text.replace(CLOSE_BRACKETS, "\\1</i>");
            if(SPLITTER.indexIn(text) != -1) {
                QString cellClass = (bgClass % 2) == 0 ? "" : " class=\"alternate\"";
                QString german = SPLITTER.cap(1).trimmed();
                QString english = SPLITTER.cap(2).trimmed();
                
                table += QString("<tr><td%1>%2</td><td%1>%3</td></tr>").arg(cellClass).arg(german).arg(english);
                
                ++bgClass;
            } else {
                kError() << "Bad line: splitter not found in" << text;
            }
        }
        
        // replace placeholders
        QString fontSize = QString::number(m_fontSize); // %3
        QString captionGerman = i18nc("result table caption", "German"); // %6
        QString captionEnglish = i18nc("result table caption", "English"); // %7
        QString css = QUrl::fromLocalFile(CSS_FILE).toString();

        html = html.arg(css).arg(m_fontFamily).arg(fontSize).arg(m_textColor).arg(m_alternateBackgroundColor).arg(captionGerman).arg(captionEnglish).arg(table);
    }
    
    return html;
}

/**
 * This method generates the HTML code of the failure page from the template
 * and returns it.
 *
 * @return HTML code of the failure page
 */
QString HtmlGenerator::noMatchesPage() const {
    QString html = loadFile(NO_MATCHES_FILE);
    
    if(html.isEmpty()) {
        html = emptyPage();
    } else {
        // replace placeholders
        QString infocss = QUrl::fromLocalFile(KStandardDirs::locate("data", "kf5/infopage/kde_infopage.css")).toString(); // %1
        QString rtlcss = QApplication::isRightToLeft() ? QString("@import \"%1\"").arg(QUrl::fromLocalFile(KStandardDirs::locate("data", "kf5/infopage/kde_infopage_rtl.css")).toString()) : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString message = i18n("No matches found"); // %74
        
        html = html.arg(infocss).arg(rtlcss).arg(fontSize).arg(message);
    }
    
    return html;
}

QUrl HtmlGenerator::styleSheetUrl() const {
    return QUrl::fromLocalFile(CSS_FILE);
}

/**
 * This method reads in a file and returns its contents.
 *
 * @param filename the file to read
 *
 * @return content of the file to read, or an empty string in case an error
 *         occurred while trying to read the file
 */
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
