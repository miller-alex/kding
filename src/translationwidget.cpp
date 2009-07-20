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

#include "translationwidget.h"
#include "searchengine.h"
#include "htmlgenerator.h"
#include "settings.h"
#include <KAnimatedButton>
#include <KIcon>
#include <KHTMLPart>
#include <KHTMLView>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <QSize>
#include <QString>
#include <QLineEdit>

TranslationWidget::TranslationWidget(QWidget* parent) : QWidget(parent), m_searchEngine(0), m_htmlGenerator(0) {
    m_htmlGenerator = new HtmlGenerator(this);
    
    setupUi(this);
    initGui();
    
    m_searchEngine = new SearchEngine(this);
    connect(m_searchEngine, SIGNAL(searchStarted()), m_busyAnimation, SLOT(start()));
    connect(m_searchEngine, SIGNAL(searchFinished()), m_busyAnimation, SLOT(stop()));
    connect(m_searchEngine, SIGNAL(searchFinished()), this, SLOT(processSearchResults()));
    connect(m_searchEngine, SIGNAL(searchRunning()), this, SLOT(anotherSearchRunning()));
    connect(m_searchEngine, SIGNAL(statusMessage(QString)), this, SIGNAL(statusMessage(QString)));
}

TranslationWidget::~TranslationWidget() {
    saveSettings();
}

void TranslationWidget::initGui() {
    historyInput->setDuplicatesEnabled(false);
    historyInput->setHistoryItems(Settings::recent(), true);    // set history and completion list
    connect(historyInput, SIGNAL(cleared()), this, SLOT(saveSettings()));
    connect(historyInput, SIGNAL(returnPressed()), this, SLOT(startSearch()));
    
    labelIcon->setPixmap(KIcon("kding").pixmap(48,48));
    
    m_busyAnimation = new KAnimatedButton(this);
    m_busyAnimation->setIconSize(QSize(22, 22));
    m_busyAnimation->setIcons("kding_search");
    layoutInfo->addWidget(m_busyAnimation);
    
    buttonTranslate->setIcon(KIcon("go-jump-locationbar"));
    connect(buttonTranslate, SIGNAL(clicked()), this, SLOT(startSearch()));
    
    // set up the HTML view
    m_htmlPart = new KHTMLPart(this);
    layoutMain->addWidget(m_htmlPart->view());
    m_htmlPart->setJScriptEnabled(false);
    m_htmlPart->setJavaEnabled(false);
    m_htmlPart->setMetaRefreshEnabled(false);
    m_htmlPart->setPluginsEnabled(false);
    
    // display the initial "Welcome" page
    displayHtml(m_htmlGenerator->welcomePage());
    
    focusInputWidget();
}

void TranslationWidget::displayHtml(QString html) {
    m_htmlPart->begin();
    m_htmlPart->setUserStyleSheet(m_htmlGenerator->styleSheetUrl());
    m_htmlPart->write(html);
    m_htmlPart->end();
}

void TranslationWidget::startSearch() {
    QString phrase(historyInput->currentText());
    
    if(!phrase.isEmpty()) {
        translate(phrase);
    } else {
        kdDebug() << "Input field is empty, doing nothing";
    }
}

void TranslationWidget::stopSearch() {
    kdDebug() << "TODO: stopSearch()";
}

void TranslationWidget::anotherSearchRunning() {
    emit statusMessage(i18n("Another search is currently running."));
}

void TranslationWidget::translate(QString phrase) {
    //KIcon("process-stop")
    //buttonTranslate->setIcon(KIcon("go-jump-locationbar"));
    historyInput->setEditText(phrase);
    m_searchEngine->search(phrase);
}

void TranslationWidget::processSearchResults() {
    ResultList resultList = m_searchEngine->results();
    QString searchTerm = m_searchEngine->searchTerm();
    
    if(resultList.isEmpty()) {
        displayHtml(m_htmlGenerator->noResultsPage());
    } else {
        displayHtml(m_htmlGenerator->resultPage(searchTerm, resultList));
        historyInput->addToHistory(searchTerm);
        historyInput->setCurrentIndex(0);
        saveSettings();
    }
    
    QString msgStr = i18np("Found 1 match for '%2'.", "Found %1 matches for '%2'.", resultList.size()).arg(searchTerm);
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
    
    Settings::self()->writeConfig();
}

#include "translationwidget.moc"
