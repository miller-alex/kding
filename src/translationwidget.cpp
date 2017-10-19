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

#include "translationwidget.h"
#include "searchengine.h"
#include "htmlgenerator.h"
#include "settings.h"
#include <KAnimatedButton>
#include <KIconLoader>
#include <KHTMLPart>
#include <KHTMLView>
#include <QFontDatabase>
#include <QIcon>
#include <QSize>
#include <QString>
#include <QLineEdit>

TranslationWidget::TranslationWidget(QWidget* parent) : QWidget(parent), m_searchEngine(0), m_htmlGenerator(0) {
    setupUi(this);
    initGui();
    
    m_searchEngine = new SearchEngine(this);
    connect(m_searchEngine, SIGNAL(searchStarted()), busyAnimation, SLOT(start()));
    connect(m_searchEngine, SIGNAL(searchFinished()), busyAnimation, SLOT(stop()));
    connect(m_searchEngine, SIGNAL(searchFinished()), this, SLOT(processSearchResults()));
    connect(m_searchEngine, SIGNAL(searchRunning()), this, SLOT(anotherSearchRunning()));
    connect(m_searchEngine, SIGNAL(statusMessage(QString)), this, SIGNAL(statusMessage(QString)));
    
    // calculate pixel size from point size
    int pointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    int pixelSize = (pointSize * m_htmlPart->view()->logicalDpiY() + 36) / 72;
    m_htmlGenerator = new HtmlGenerator(pixelSize, this);
    
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
    
    labelIcon->setPixmap(QIcon::fromTheme("kding").pixmap(48,48));
    
    const int baIconSize = qMin(busyAnimation->iconSize().width(), busyAnimation->iconSize().height());
    busyAnimation->setAnimationPath(KIconLoader::global()->iconPath("process-working", -baIconSize));
    
    buttonTranslate->setIcon(QIcon::fromTheme("go-jump-locationbar"));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    
    // set up the HTML view
    m_htmlPart = new KHTMLPart(this);
    layoutMain->addWidget(m_htmlPart->view());
    m_htmlPart->setJScriptEnabled(false);
    m_htmlPart->setJavaEnabled(false);
    m_htmlPart->setMetaRefreshEnabled(false);
    m_htmlPart->setPluginsEnabled(false);
    
    focusInputWidget();
}

/**
 * Display the given HTML page in the @c KHTMLPart.
 *
 * @param html HTML code to display
 *
 * @see HtmlGenerator
 */
void TranslationWidget::displayHtml(QString html) {
    m_htmlPart->begin();
    //m_htmlPart->setUserStyleSheet(m_htmlGenerator->styleSheetUrl());
    m_htmlPart->write(html);
    m_htmlPart->end();
    
    // scroll to the top of the view
    m_htmlPart->view()->ensureVisible(0, 0);
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
    
    disconnect(buttonTranslate, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    buttonTranslate->setIcon(QIcon::fromTheme("go-jump-locationbar"));
    
    busyAnimation->stop();
    
    focusInputWidget();
    
    emit statusMessage(i18n("Search cancelled."));
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
    buttonTranslate->setIcon(QIcon::fromTheme("process-stop"));
    
    historyInput->setEditText(phrase);
    m_searchEngine->search(phrase);
}

/**
 * Scrolls the results in the @c KHTMLPart by @p delta units verically
 * or horzontally depending on @p orientation. Call this slot to handle
 * input events which request scrolling.
 */
void TranslationWidget::scrollResults(int delta, Qt::Orientation orientation) {
    if (!isVisible() || window()->isMinimized()) {
        return;
    }

    if (orientation == Qt::Horizontal) {
        m_htmlPart->view()->scrollBy(-delta, 0);
    } else {
        m_htmlPart->view()->scrollBy(0, -delta);
    }
}

/**
 * This method fetches the @c #ResultList from the @c SearchEngine and employs
 * the @c HtmlGenerator to create a HTML representation of it, suitable to be
 * displayed in the @c KHTMLPart.
 */
void TranslationWidget::processSearchResults() {
    disconnect(buttonTranslate, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    buttonTranslate->setIcon(QIcon::fromTheme("go-jump-locationbar"));
    
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

#include "translationwidget.moc"
