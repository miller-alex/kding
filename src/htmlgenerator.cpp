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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "htmlgenerator.h"
#include "searchengine.h"
#include <KLocalizedString>
#include <KAboutData>
#include <KColorScheme>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QApplication>
#include <QFontDatabase>
#include <QColor>
#include <QPalette>
#include <QBrush>

const QRegExp HtmlGenerator::SPLITTER = QRegExp("(.*) :: (.*)");    ///< capture left and right side of a line (the German and the English part) in groups
const QRegExp HtmlGenerator::OPEN_BRACKETS = QRegExp("([[{])");    ///< match opening brackets: { [
const QRegExp HtmlGenerator::CLOSE_BRACKETS = QRegExp("([]}])");    ///< match closing brackets: } ]

static inline QString locateAppData(const QString &fileName) {
    return QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
}

static inline QString locateGenericData(const QString &fileName) {
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, fileName);
}

static inline QString fileUrl(const QString &fileName) {
    return QUrl::fromLocalFile(fileName).toString();
}

static inline QString appDataUrl(const QString &fileName) {
    return fileUrl(locateAppData(fileName));
}

static inline QString infopageUrl(const QString &fileName) {
    return fileUrl(locateGenericData(QStringLiteral("kf5/infopage/")
                                     + fileName));
}

/**
 * This creates a new instance of @c HtmlGenerator.
 * It takes the font size to be used in the HTML code and, optionally, a parent
 * object for this object.
 *
 * @param fontSize font size in pixels
 * @param parent parent object
 */
HtmlGenerator::HtmlGenerator(int fontSize, QObject* parent)
  : QObject(parent),
    CSS_FILE_URL(appDataUrl("html/kding.css")),
    WELCOME_FILE(locateAppData("html/welcome.html")),
    RESULT_FILE(locateAppData("html/result.html")),
    NO_MATCHES_FILE(locateAppData("html/nomatches.html")),
    m_fontSize(fontSize),
    m_fontFamily(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family())
{
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
        QString infocss = infopageUrl("kde_infopage.css"); // %1
        QString rtlcss = QApplication::isRightToLeft()
            ? QString("@import \"%1\"").arg(infopageUrl("kde_infopage_rtl.css"))
            : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString appTitle = KAboutData::applicationData().displayName(); // %4
        QString catchPhrase = i18n("Dictionary Lookup for KDE"); // %5
        QString shortDescription = KAboutData::applicationData()
            .shortDescription(); // %6
        QString description = i18n("Using dictionary version %1",
                                   searchEngine.dictionaryVersion()); // %7
        QString welcomeCss = appDataUrl("html/welcome.css"); // %8

        html = html.arg(infocss, rtlcss, fontSize, appTitle, catchPhrase,
                        shortDescription, description, welcomeCss);
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
        bool alt_bg = false; // for alternating background colors
        QString table;
        // matches & captures the search term:
        QRegExp term("(" + searchTerm + ")", Qt::CaseInsensitive);

        // create the table
        foreach(ResultItem item, resultList) {
            if (SPLITTER.indexIn(item.text().simplified()) < 0) {
                qCritical() << "Bad line: splitter not found in" << item.text();
                continue;
            }
            QStringList left = SPLITTER.cap(1).split(" | ");
            QStringList right = SPLITTER.cap(2).split(" | ");

            formatFragments(left, term);
            formatFragments(right, term);

            if (left.length() != right.length()) {
                // lines don't match up so collapse entry to a single table row
                left = QStringList("<div>" + left.join(QLatin1String(
                          "</div><div class=\"continued\">")) + "</div>");
                right = QStringList("<div>" + right.join(QLatin1String(
                          "</div><div class=\"continued\">")) + "</div>");
            }

            table.append(alt_bg ? QLatin1String("<tbody class=\"alternate\">")
                                : QLatin1String("<tbody>"));
            for (int i = 0; i < left.length(); ++i) {
                table.append(QStringLiteral("<tr%1><td>%2</td><td>%3</td></tr>")
                             .arg(i > 0 ? QStringLiteral(" class=\"continued\"")
                                  : QString(), left.at(i), right.at(i)));
            }
            table.append(QLatin1String("</tbody>\n"));

            alt_bg = !alt_bg;
        }
        
        // replace placeholders
        QString fontSize = QString::number(m_fontSize); // %3
        QString captionGerman = i18nc("result table caption", "German"); // %6
        QString captionEnglish = i18nc("result table caption", "English"); // %7

        html = html.arg(CSS_FILE_URL, m_fontFamily, fontSize,
                        m_textColor, m_alternateBackgroundColor,
                        captionGerman, captionEnglish, table);
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
        QString infocss = infopageUrl("kde_infopage.css"); // %1
        QString rtlcss = QApplication::isRightToLeft()
            ? QString("@import \"%1\"").arg(infopageUrl("kde_infopage_rtl.css"))
            : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString message = i18n("No matches found"); // %74

        html = html.arg(infocss, rtlcss, fontSize, message);
    }
    
    return html;
}

QUrl HtmlGenerator::styleSheetUrl() const {
    return QUrl(CSS_FILE_URL);
}

/**
 * Returns the path where resources referenced by HTML pages and style sheets
 * can reside, e.g. images and CSS files.
 */
QStringList HtmlGenerator::resourcePaths() {
    return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)
        .replaceInStrings(QRegExp("$"), "/html")
        + QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)
        .replaceInStrings(QRegExp("$"), "/kf5/infopage");
}

/**
 * Convert all strings in @p list to HTML and mark all occurrences of
 * @p searchTerm and phrases in brackets with appropriate tags.
 */
void HtmlGenerator::formatFragments(QStringList &list,
                                    const QRegExp &searchTerm)
{
    // mark search term before escaping, replace with tags afterwards;
    // use newline as special marker since text is a single line
    list.replaceInStrings(searchTerm, QStringLiteral("\n[\\1\n]"));

    // <> becomes left right arrow
    list.replaceInStrings(QStringLiteral("<>"), QChar(0x2194));

    for (QString &s : list) s = s.toHtmlEscaped();

    list.replaceInStrings(QStringLiteral("\n["),
                          QStringLiteral("<span class=\"keyword\">"))
        .replaceInStrings(QStringLiteral("\n]"),
                          QStringLiteral("</span>"))
        // FIXME: ensure open/close tags match and are properly nested
        .replaceInStrings(OPEN_BRACKETS, QStringLiteral("<i>\\1"))
        .replaceInStrings(QStringLiteral(" <i>"), QStringLiteral("&nbsp;<i>"))
        .replaceInStrings(CLOSE_BRACKETS, QStringLiteral("\\1</i>"));
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
        qCritical() << "Failed to open" << filename;
    }
    
    return contents;
}

#include "moc_htmlgenerator.cpp"
