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

#include "translationwidget.h"
#include "searchengine.h"
#include "htmlgenerator.h"
#include "settings.h"
#include <KAnimatedButton>
#include <QFontDatabase>
#include <QIcon>
#include <KIconLoader>
#include <QLineEdit>
#include <QRegExp>
#include <QScrollBar>
#include <QSize>
#include <QString>
#include <QTextEdit>

TranslationWidget::TranslationWidget(QWidget* parent) : QWidget(parent), m_searchEngine(0), m_htmlGenerator(0) {
    setupUi(this);
    initGui();
    
    m_searchEngine = new SearchEngine(this);
    connect(m_searchEngine, SIGNAL(searchFinished()), this, SLOT(processSearchResults()));
    connect(m_searchEngine, SIGNAL(searchRunning()), this, SLOT(anotherSearchRunning()));
    connect(m_searchEngine, SIGNAL(statusMessage(QString)), this, SIGNAL(statusMessage(QString)));
    
    // calculate pixel size from point size
    int pointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    int pixelSize = (pointSize * textBrowser->logicalDpiY() + 36) / 72;
    m_htmlGenerator = new HtmlGenerator(pixelSize, this);
    
    // set resource search paths, needed to find images/CSS w/ relative URIs
    textBrowser->setSearchPaths(m_htmlGenerator->resourcePaths());

    // leave full control over margins to style sheet, no extra border
    textBrowser->document()->setDocumentMargin(0);

    // display the initial "Welcome" page
    displayHtml(m_htmlGenerator->welcomePage());
}

TranslationWidget::~TranslationWidget() {
    saveSettings();
}

void TranslationWidget::initGui() {
    historyInput->setDuplicatesEnabled(false);
    historyInput->setHistoryItems(Settings::recent(), true);    // set history and completion list
    connect(historyInput, SIGNAL(cleared()), this, SLOT(saveSettings()));
    connect(historyInput, SIGNAL(returnPressed()), this, SLOT(startSearch()));

    //buttonIcon->setIcon(QIcon::fromTheme("kding"));
    connect(buttonIcon, SIGNAL(clicked()),
            parentWidget(), SLOT(translateClipboard()));

    //m_buttonTranslateIcon = QIcon::fromTheme("go-jump-locationbar");
    m_buttonTranslateIcon = buttonTranslate->icon();
    const QSize iconSize = buttonTranslate->iconSize();
    // load animation displayed when busy
    buttonTranslate->setAnimationPath(
            KIconLoader::global()->iconPath("process-working",
                -qMin(iconSize.width(), iconSize.height())));
    // restore default icon
    buttonTranslate->setIcon(m_buttonTranslateIcon);
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    
    focusInputWidget();
}

/**
 * Display the given HTML page in the @c QTextBrowser.
 *
 * @param html HTML code to display
 *
 * @see HtmlGenerator
 */
void TranslationWidget::displayHtml(QString html) {
    textBrowser->setHtml(html);
}

/**
 * This method starts a search for the phrase entered in the input widget.
 */
void TranslationWidget::startSearch() {
    QString phrase(historyInput->currentText());

    if(phrase == "about:kding") {
        displayHtml(m_htmlGenerator->welcomePage());
        focusInputWidget();
    } else {
        translate(phrase);
    }
}

/**
 * This method cancels a currently running search and resets the
 * @c TranslationWidget 's state.
 */
void TranslationWidget::stopSearch() {
    m_searchEngine->cancelSearch();
    emit statusMessage(i18n("Search cancelled."));

    disconnect(buttonTranslate, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));

    buttonTranslate->stop();
    buttonTranslate->setIcon(m_buttonTranslateIcon);

    focusInputWidget();
}

void TranslationWidget::anotherSearchRunning() {
    emit statusMessage(i18n("Another search is currently running."));
}

/**
 * Checks whether @p phrase is a valid search term and its results
 * aren't already displayed.
 */
bool TranslationWidget::checkSearchTerm(QString phrase) const {
    phrase = phrase.trimmed();

    if (phrase.isEmpty() || (m_searchEngine->searchTerm() == phrase)) {
        return false;
    }
    return true;
}

/**
 * This method starts a search for the phrase passed in @p phrase.
 *
 * @param phrase the phrase to search for
 */
void TranslationWidget::translate(QString phrase) {
    phrase = phrase.trimmed();
    
    if (phrase.isEmpty()) {
        return;
    }
    
    disconnect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(stopSearch()));
    buttonTranslate->start();

    historyInput->setEditText(phrase);
    m_searchEngine->search(phrase);
}

/**
 * Scrolls the HTML page in the @c QTextBrowser by @p delta units verically
 * or horzontally depending on @p orientation. Call this slot to handle
 * input events which request scrolling.
 */
void TranslationWidget::scrollResults(int delta, Qt::Orientation orientation) {
    if (!isVisible() || window()->isMinimized()) {
        return;
    }

    QScrollBar *sb;
    if (orientation == Qt::Horizontal)
        sb = textBrowser->horizontalScrollBar();
    else
        sb = textBrowser->verticalScrollBar();

    sb->setValue(sb->value() - delta);
}

/**
 * This method fetches the @c #ResultList from the @c SearchEngine,
 * employs the @c HtmlGenerator to create a HTML representation of it,
 * and displays it in the @c QTextBrowser.
 */
void TranslationWidget::processSearchResults() {
    disconnect(buttonTranslate, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    buttonTranslate->stop();
    buttonTranslate->setIcon(m_buttonTranslateIcon);

    ResultList resultList = m_searchEngine->results();
    QString searchTerm = m_searchEngine->searchTerm();
    
    if(resultList.isEmpty()) {
        displayHtml(m_htmlGenerator->noMatchesPage());
    } else {
        displayHtml(m_htmlGenerator->resultPage(searchTerm, resultList));
        historyInput->addToHistory(searchTerm);
        historyInput->setCurrentIndex(0);
        saveSettings();
    }
    
    QString msgStr = i18np("Found 1 match for '%2'.", "Found %1 matches for '%2'.", resultList.size(), searchTerm);
    emit statusMessage(msgStr);
    
    focusInputWidget();
}

/**
 * Set the focus to the input widget and select its contents, if any.
 */
void TranslationWidget::focusInputWidget() {
    historyInput->setFocus(Qt::OtherFocusReason);
    historyInput->lineEdit()->selectAll();
}

void TranslationWidget::clearDisplay() {
    historyInput->clearEditText();
    m_searchEngine->cancelSearch();
    displayHtml(m_htmlGenerator->emptyPage());
}

void TranslationWidget::clearText() {
    historyInput->clearEditText();
    historyInput->setFocus(Qt::OtherFocusReason);
}

QString TranslationWidget::dictionaryVersion() const {
    return m_searchEngine->dictionaryVersion();
}

void TranslationWidget::saveSettings() {
    QStringList list = historyInput->historyItems();
    Settings::self()->setRecent(list);
    
    Settings::self()->save();
}

void TranslationWidget::updateSearchBackend() {
    m_searchEngine->updateSearchBackend();
}

#include "moc_translationwidget.cpp"
