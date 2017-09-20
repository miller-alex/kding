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

#include "searchengine.h"
#include "searchbackendfactory.h"
#include <KStandardDirs>
#include <KLocale>
#include <KDebug>
#include <KProcess>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QRegExp>
#include <QMutableListIterator>
#include <QtAlgorithms>

const QRegExp SearchEngine::MULTI_LINE = QRegExp("\\|");    ///< RegExp to match line break markers
const QRegExp SearchEngine::ABBREVIATION = QRegExp(" : ");  ///< RegExp to match abbreviation markers
const QRegExp SearchEngine::ROUND_BRACKETS = QRegExp(" \\([^.\\)]*\\)");    ///< RegExp to match text in round brackets
const QRegExp SearchEngine::CURLY_BRACKETS = QRegExp(" \\{[^.\\}]*\\}");    ///< RegExp to match text in curly brackets
const QRegExp SearchEngine::SQUARE_BRACKETS = QRegExp(" \\[[^\\]]*\\]");    ///< RegExp to match text in square brackets

SearchEngine::SearchEngine(QObject* parent) : QObject(parent), DEFAULT_DICTIONARY(KStandardDirs::locate("appdata", "de-en.txt")), DICTIONARY_VERSION(determineDictionaryVersion()), m_process(0), m_resultList(0), m_backendFactory(0) {
    m_backendFactory = new SearchBackendFactory(this);
}

SearchEngine::~SearchEngine() {
    
}

/**
 * This is where the search is initiated.
 * The @c KProcess is set up, its signals are connected to the corresponding
 * slots of this class, and finally the process is started.
 *
 * @param phrase the phrase to search for
 *
 * @see processFinished()
 * @see cancelSearch()
 */
void SearchEngine::search(QString phrase) {
    // if there is already another search process running emit a signal and
    // return without doing anything
    if(m_process != 0) {
        emit searchRunning();
        return;
    }
    
    // do some preparatory tasks:
    // we need to store the search term to properly sort the entries in the
    // result list by their priority ...
    m_searchTerm = phrase;
    // ... and we delete the old result list as it will be replaced by whatever
    // this search returns
    delete m_resultList;
    m_resultList = 0;
    
    // now construct the KProcess object and set up all necessary connections
    m_process = new KProcess();
    m_process->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    
    connect(m_process, SIGNAL(started()), this, SIGNAL(searchStarted()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(monitorProcess(QProcess::ProcessState)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processFailed(QProcess::ProcessError)));
    
    // this check is needed to pass a correctly encoded search term to the
    // grep command
    if(QString("UTF-8") != KLocale::global()->encoding()) {
        phrase = phrase.toUtf8();
    }
    
    // set up the command to run
    kDebug() << m_backendFactory->executable() << m_backendFactory->argumentList();
    (*m_process) << m_backendFactory->executable() << m_backendFactory->argumentList() << phrase << DEFAULT_DICTIONARY;
    
    // finally start searching
    emit statusMessage(i18n("Searching for '%1'...", phrase));
    m_process->start();
}

/**
 * This method cancels a currently running search operation and resets the
 * search engine.
 *
 * @see search()
 * @see processFinished()
 */
void SearchEngine::cancelSearch() {
    if(m_process != 0) {
        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
        m_process->close();
        
        delete m_process;
        m_process = 0;
    }
    
    if(m_resultList != 0) {
        delete m_resultList;
        m_resultList = 0;
    }
    
    m_searchTerm = QString();
}

/**
 * This method reads the results from the @c KProcess and creates and sorts the
 * ResultList from these.
 * After that, the search is finished and the objects interested in that are
 * notified via the relevant signal.
 *
 * @param exitCode exit code of the process
 * @param exitStatus exit status of the process
 *
 * @see search()
 * @see cancelSearch()
 */
void SearchEngine::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    kDebug() << "Exit code:" << exitCode;
    kDebug() << "Exit status:" << (exitStatus == QProcess::NormalExit ? "normal" : (exitStatus == QProcess::CrashExit ? "crashed" : "unknown"));
    
    m_resultList = new ResultList();
    while(m_process->canReadLine()) {
        ResultItem item;
        item.setText(QString::fromUtf8(m_process->readLine()).trimmed());
        m_resultList->append(item);
    }
    
    sortResultsByPriority(m_resultList);
    
    // clean up
    delete m_process;
    m_process = 0;
    
    emit searchFinished();
}

/**
 * Sorts the results by priority.
 * The algorithm to determine the priority of an item is taken from
 * <a href="http://www-user.tu-chemnitz.de/~fri/ding/">Ding</a>.
 *
 * @param resultList a pointer to the @c #ResultList to sort
 */
void SearchEngine::sortResultsByPriority(ResultList* resultList) const {
    // the result starts with the search term
    QRegExp startsWith_DE("^" + m_searchTerm + "(;| ::)", Qt::CaseInsensitive);
    QRegExp startsWith_EN(":: (to )?" + m_searchTerm + "(;|$)", Qt::CaseInsensitive);
    // the result contains the search term somewhere
    QRegExp contains("; " + m_searchTerm + "(;|$)", Qt::CaseInsensitive);
    
    // the first line of a multi line result starts with the search term
    QRegExp startsWith_multiLine_DE("^" + m_searchTerm + "(;| ).*\\|.* ::", Qt::CaseInsensitive);
    QRegExp startsWith_multiLine_EN(":: (to )?" + m_searchTerm + "(;.*| )\\|", Qt::CaseInsensitive);
    // the first line of a multi line result contains the search term somewhere
    QRegExp firstLine_DE("^[^|]*" + m_searchTerm + ".*\\|.* ::", Qt::CaseInsensitive);
    QRegExp firstLine_EN(":: [^|]*" + m_searchTerm + ".*\\|", Qt::CaseInsensitive);
    
    QMutableListIterator<ResultItem> iterator(*resultList);
    while(iterator.hasNext()) {
        ResultItem item = iterator.next();
        QString text = item.text();
        
        // remove all text in brackets
        text.replace(ROUND_BRACKETS, "");
        text.replace(CURLY_BRACKETS, "");
        text.replace(SQUARE_BRACKETS, "");
        
        if(text.contains(MULTI_LINE)) { // this item is a multi line result
            const int LINE_COUNT = text.count(MULTI_LINE);
            item.setPriority(0);
            
            if(text.contains(startsWith_multiLine_DE) || text.contains(startsWith_multiLine_EN)) {
                item.addToPriority(STARTS_WITH + LINE_COUNT);
            } else if(text.contains(firstLine_DE) || text.contains(firstLine_EN)) {
                item.addToPriority(LINE_COUNT);
            }
        } else { // this item is a single line result
            item.setPriority(2);
            
            if(text.contains(startsWith_DE) || text.contains(startsWith_EN)) {
                item.addToPriority(STARTS_WITH);
            } else if(text.contains(contains)) {
                item.addToPriority(CONTAINS);
            }
            
            if(text.contains(ABBREVIATION)) {
                item.addToPriority(IS_ABBREVIATION);
            }
        }
        
        iterator.setValue(item);
    }
    
    qStableSort(*resultList);
}

QString SearchEngine::searchTerm() const {
    return m_searchTerm;
}

ResultList& SearchEngine::results() const {
    return *m_resultList;
}

/**
 * This debug method is used to keep track of the current state of the process.
 * It is connected to @c QProcess::stateChanged(QProcess::ProcessState).
 *
 * @see processFailed()
 */
void SearchEngine::monitorProcess(QProcess::ProcessState newState) const {
    switch(newState) {
        case QProcess::NotRunning:
            kDebug() << "Process is not running";
            break;
        case QProcess::Starting:
            kDebug() << "Process is starting";
            break;
        case QProcess::Running:
            kDebug() << "Process is running";
            break;
    }
}

/**
 * This debug method is used to give information on errors when running the
 * process.
 * It is connected to @c QProcess::error(QProcess::ProcessError).
 *
 * @see monitorProcess()
 */
void SearchEngine::processFailed(QProcess::ProcessError error) const {
    switch(error) {
        case QProcess::FailedToStart:
            kDebug() << "Process failed to start";
            break;
        case QProcess::Crashed:
            kDebug() << "Process crashed";
            break;
        case QProcess::Timedout:
            kDebug() << "Process timed out";
            break;
        case QProcess::WriteError:
            kDebug() << "Process could not be written to";
            break;
        case QProcess::ReadError:
            kDebug() << "Process could not be read from";
            break;
        case QProcess::UnknownError:
            kDebug() << "Process encountered an unknown error";
            break;
    }
}

QString SearchEngine::dictionaryVersion() const {
    return DICTIONARY_VERSION;
}

QString SearchEngine::backendVersion() const {
    return "";
}

QString SearchEngine::determineDictionaryVersion() {
    // The version of the dictionary is given in the first line of the file.
    // As of v1.5 it is of the form
    //   # Version :: 1.5 2007-04-09
    // for release versions or
    //   # Version :: devel 2007-06-10
    // for development versions of the dictionary.
    
    QString version(i18nc("dictionary version", "unknown"));
    
    QFile dictFile(DEFAULT_DICTIONARY);
    if(dictFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&dictFile);
        QString line;
        
        line = stream.readLine();
        if(line != 0) {
            int pos = line.indexOf("::");
            if(pos != -1) {
                version = line.mid(pos + 2).trimmed();
            }
        }
        
        dictFile.close();
    } else {
        kError() << "Failed to open dictionary" << dictFile.fileName();
    }
    
    return version;
}

QString SearchEngine::determineBackendVersion() {
    return "";
}

void SearchEngine::updateSearchBackend() const {
    m_backendFactory->generate();
}

#include "searchengine.moc"
