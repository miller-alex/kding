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

static inline QString locateAppData(const QString &fileName) {
    return QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
}

static inline QString locateGenericData(const QString &fileName) {
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, fileName);
}

static inline QString locatePage(const QString &fileName) {
    return locateAppData(QStringLiteral("html/") + fileName);
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

static QString simplePage(const QString &body) {
    return QStringLiteral("<html><body>")
         + body
         + QStringLiteral("</body></html>");
}

/**
 * This function reads a file in and returns its contents.
 *
 * @param filename the file to read
 *
 * @return content of the file to read, or an empty string in case an error
 *         occurred while trying to read the file
 */
static QString loadFile(const QString &filename) {
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
        qCritical() << "Failed to open" << filename;
        return QString();
    }

    QTextStream stream(&file);
    return stream.readAll();
}

static QString loadPage(const QString &filename, const QString &fallback) {
    QString html = loadFile(locatePage(filename));

    if (html.isNull())
        return simplePage(fallback);

    return html;
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
    QString html = loadPage("welcome.html",
            "<!-- %1 %2 %3 --><h1>%4</h1><h2>%5</h2><h3>%6</h3>%7");

    if (1) {
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
    // captures left and right side of a line (the German and the English part) in groups
    static const QRegExp SPLITTER = QRegExp("(.*) :: (.*)");

    QString html = loadPage("result.html",
            "<!-- %1 %2 %3 %4 %5 --><table><tr><th>%6</th><th>%7</th></tr>%8</table>");
    
    if (1) {
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
        QString cssUrl = appDataUrl("html/kding.css");
        QString fontSize = QString::number(m_fontSize); // %3
        QString captionGerman = i18nc("result table caption", "German"); // %6
        QString captionEnglish = i18nc("result table caption", "English"); // %7

        html = html.arg(cssUrl, m_fontFamily, fontSize,
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
    QString html = loadPage("nomatches.html", "<!-- %1 %2 %3 -->%4");
    
    if (1) {
        // replace placeholders
        QString infocss = infopageUrl("kde_infopage.css"); // %1
        QString rtlcss = QApplication::isRightToLeft()
            ? QString("@import \"%1\"").arg(infopageUrl("kde_infopage_rtl.css"))
            : ""; // %2
        QString fontSize = QString::number(m_fontSize); // %3
        QString message = i18n("No matches found"); // %4

        html = html.arg(infocss, rtlcss, fontSize, message);
    }
    
    return html;
}

QUrl HtmlGenerator::styleSheetUrl() const {
    return QUrl(appDataUrl("html/kding.css"));
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
    // matches opening brackets: { [
    static const QRegExp OPEN_BRACKETS = QRegExp("([[{])");
    // matches closing brackets: } ]
    static const QRegExp CLOSE_BRACKETS = QRegExp("([]}])");

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

#include "moc_htmlgenerator.cpp"
