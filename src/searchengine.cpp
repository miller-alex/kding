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
#include <KStandardDirs>
#include <KLocale>
#include <KGlobal>
#include <KDebug>
#include <KProcess>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QRegExp>
#include <QMutableListIterator>
#include <QtAlgorithms>

const QString SearchEngine::SEARCH_CMD = "egrep";
const QStringList SearchEngine::SEARCH_ARGS = QStringList() \
    << "-i"     /* case insensitive */ \
    << "-w";    /* match only whole words */

const QRegExp SearchEngine::MULTI_LINE = QRegExp("\\|");
const QRegExp SearchEngine::ABBREVIATION = QRegExp(" : ");
const QRegExp SearchEngine::ROUND_BRACKETS = QRegExp(" \\([^.\\)]*\\)");
const QRegExp SearchEngine::CURLY_BRACKETS = QRegExp(" \\{[^.\\}]*\\}");
const QRegExp SearchEngine::SQUARE_BRACKETS = QRegExp(" \\[[^\\]]*\\]");

SearchEngine::SearchEngine(QObject* parent) : QObject(parent), DEFAULT_DICTIONARY(KStandardDirs::locate("appdata", "de-en.txt")), DICTIONARY_VERSION(determineDictionaryVersion()), m_process(0), m_resultList(0) {
    
}

SearchEngine::~SearchEngine() {
    
}

/**
 * This is where the search is initiated. The KProcess is set up, its signals
 * are connected to the corresponding slots of this class, and finally the
 * process is started.
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
    if(QString("UTF-8") != KGlobal::locale()->encoding()) {
        phrase = phrase.toUtf8();
    }
    
    // set up the command to run
    (*m_process) << SEARCH_CMD << SEARCH_ARGS << phrase << DEFAULT_DICTIONARY;
    
    // finally start searching
    emit statusMessage(i18n("Searching for '%1'...", phrase));
    m_process->start();
}

/**
 * This method reads the results from the KProcess and creates and sorts the
 * ResultList from these.  After that, the search is finished and the objects
 * interested in that are notified via the relevant signal.
 */
void SearchEngine::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    kdDebug() << "Exit code:" << exitCode;
    kdDebug() << "Exit status:" << (exitStatus == QProcess::NormalExit ? "normal" : (exitStatus == QProcess::CrashExit ? "crashed" : "unknown"));
    
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

void SearchEngine::sortResultsByPriority(ResultList* resultList) {
    // 
    QRegExp startsWith_ger("^" + m_searchTerm + "(;| ::)", Qt::CaseInsensitive);
    // 
    QRegExp startsWith_eng(":: (to )?" + m_searchTerm + "(;|$)", Qt::CaseInsensitive);
    // 
    QRegExp startsWith_ger_multiLine("^" + m_searchTerm + "(;| ).*\\|.* ::", Qt::CaseInsensitive);
    // 
    QRegExp startsWith_eng_multiLine(":: (to )?" + m_searchTerm + "(;.*| )\\|", Qt::CaseInsensitive);
    // 
    QRegExp singleWord("; " + m_searchTerm + "(;|$)", Qt::CaseInsensitive);
    // 
    QRegExp firstLine_ger("^[^\\|]*" + m_searchTerm + ".*\\|.* ::", Qt::CaseInsensitive);
    // 
    QRegExp firstLine_eng(":: [^\\|]*" + m_searchTerm + ".*\\|", Qt::CaseInsensitive);
    
    kdDebug() << startsWith_ger.pattern();
    kdDebug() << startsWith_eng.pattern();
    kdDebug() << startsWith_ger_multiLine.pattern();
    kdDebug() << startsWith_eng_multiLine.pattern();
    kdDebug() << singleWord.pattern();
    kdDebug() << firstLine_ger.pattern();
    kdDebug() << firstLine_eng.pattern();
    
    QMutableListIterator<ResultItem> iterator(*resultList);
    while(iterator.hasNext()) {
        ResultItem item = iterator.next();
        QString text = item.text();
        
        text.replace(ROUND_BRACKETS, "");
        text.replace(CURLY_BRACKETS, "");
        text.replace(SQUARE_BRACKETS, "");
        
        if(text.contains(MULTI_LINE)) { // this item is a multi line result
            if(text.contains(startsWith_ger_multiLine) || text.contains(startsWith_eng_multiLine)) {
                item.addToPriority(STARTS_WITH);
                kdDebug() << "STARTS_WITH";
            } else if(text.contains(firstLine_ger) || text.contains(firstLine_eng)) {
                item.addToPriority(FIRST_LINE);
                kdDebug() << "FIRST_LINE";
            }
        } else { // this item is a single line result
            if(text.contains(startsWith_ger) || text.contains(startsWith_eng)) {
                item.addToPriority(STARTS_WITH);
                kdDebug() << "STARTS_WITH";
            } else if(text.contains(singleWord)) {
                item.addToPriority(SINGLE_WORD);
                kdDebug() << "SINGLE_WORD";
            }
            
            if(text.contains(ABBREVIATION)) {
                item.addToPriority(IS_ABBREVIATION);
                kdDebug() << "IS_ABBREVIATION";
            }
        }
        
        iterator.setValue(item);
    }
    
    qStableSort(*resultList);
    
    foreach(ResultItem item, *resultList) {
        kdDebug() << "xx" << item.priority();
    }
}

QString SearchEngine::searchTerm() const {
    return m_searchTerm;
}

ResultList& SearchEngine::results() const {
    return *m_resultList;
}

/**
 * This debug method is used to keep track of the current state of the process.
 * It is connected to QProcess::stateChanged(QProcess::ProcessState).
 */
void SearchEngine::monitorProcess(QProcess::ProcessState newState) {
    switch(newState) {
        case QProcess::NotRunning:
            kdDebug() << "Process is not running";
            break;
        case QProcess::Starting:
            kdDebug() << "Process is starting";
            break;
        case QProcess::Running:
            kdDebug() << "Process is running";
            break;
    }
}

/**
 * This debug method is used to give information on errors when running the
 * process. It is connected to QProcess::error(QProcess::ProcessError).
 */
void SearchEngine::processFailed(QProcess::ProcessError error) {
    switch(error) {
        case QProcess::FailedToStart:
            kdDebug() << "Process failed to start";
            break;
        case QProcess::Crashed:
            kdDebug() << "Process crashed";
            break;
        case QProcess::Timedout:
            kdDebug() << "Process timed out";
            break;
        case QProcess::WriteError:
            kdDebug() << "Process could not be written to";
            break;
        case QProcess::ReadError:
            kdDebug() << "Process could not be read from";
            break;
        case QProcess::UnknownError:
            kdDebug() << "Process encountered an unknown error";
            break;
    }
}

QString SearchEngine::dictionaryVersion() const {
    return DICTIONARY_VERSION;
}

QString SearchEngine::grepVersion() const {
    return "";
}

QString SearchEngine::determineDictionaryVersion() {
    /*
     * The version of the dictionary is given in the first line of the file.
     * As of v1.5 it is of the form
     *   # Version :: 1.5 2007-04-09
     * for release versions or
     *   # Version :: devel 2007-06-10
     * for development versions of the dictionary.
     */
    
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
        kdError() << "Failed to open dictionary";
    }
    
    return version;
}

QString SearchEngine::determineGrepVersion() {
    return "";
}

#include "searchengine.moc"
