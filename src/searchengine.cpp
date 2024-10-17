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

#include "searchengine.h"
#include "searchbackendfactory.h"
#include <KLocalizedString>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QByteArray>
#include <QRegExp>
#include <QStandardPaths>
#include <QMutableListIterator>
#include <algorithm>

static QString getDictionary() {
    return QStandardPaths::locate(QStandardPaths::AppDataLocation, "de-en.txt");
}

SearchEngine::SearchEngine(QObject* parent)
  : QObject(parent),
    m_process(0),
    m_resultList(0)
{
    m_backendFactory = new SearchBackendFactory(this);
}

SearchEngine::~SearchEngine() {
    
}

/**
 * This is where the search is initiated.
 * The @c QProcess is set up, its signals are connected to the corresponding
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

    // now construct the QProcess object and set up all necessary connections
    m_process = new QProcess();
    m_process->setProcessChannelMode(QProcess::ForwardedErrorChannel);

    connect(m_process, SIGNAL(started()), this, SIGNAL(searchStarted()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(monitorProcess(QProcess::ProcessState)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processFailed(QProcess::ProcessError)));

    QTextCodec *localeCodec = QTextCodec::codecForLocale();
    if (localeCodec->name() == "UTF-8") {
        localeCodec = 0;
    } else {
        // the search term must always be utf-8 encoded (like the dictionary)
        // for the *grep command's args

        // Re-encoding the phrase would work for 8-bit encodings but may
        // lead to invalid code sequences on multibyte encodings:
        //phrase = QString::fromLocal8Bit(phrase.toUtf8());
        // rather temporarily change the locale encoding
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

        // also the LC_CTYPE category must be set to a utf-8 locale for
        // correct interpretation of the search term
        QProcessEnvironment env(QProcessEnvironment::systemEnvironment());
        if (env.contains(QStringLiteral("LC_ALL"))) {
            // LC_ALL must be removed because it overrides LC_CTYPE
            // but we preserve its value for LC_MESSAGES
            env.insert(QStringLiteral("LC_MESSAGES"),
                       env.value(QStringLiteral("LC_ALL")));
            env.remove(QStringLiteral("LC_ALL"));
        }
        env.insert(QStringLiteral("LC_CTYPE"), QStringLiteral("en_US.UTF-8"));
        m_process->setProcessEnvironment(env);
    }

    // set up the command to run
    qDebug() << m_backendFactory->executable()
             << m_backendFactory->argumentList()
             << phrase;
    m_process->setProgram(m_backendFactory->executable());
    m_process->setArguments(m_backendFactory->argumentList() << phrase);

    // pass the dictionary to stdin
    m_process->setStandardInputFile(getDictionary());

    // finally start searching
    emit statusMessage(i18n("Searching for '%1'...", m_searchTerm));
    m_process->start();

    // reset CodecForLocale
    if (localeCodec) QTextCodec::setCodecForLocale(localeCodec);
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
 * This method reads the results from the @c QProcess and creates and sorts the
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
    qDebug() << "Exit code:" << exitCode;
    qDebug() << "Exit status:"
             << (exitStatus == QProcess::NormalExit ? "normal" :
                 (exitStatus == QProcess::CrashExit ? "crashed" : "unknown"));

    m_resultList = new ResultList();
    while(m_process->canReadLine()) {
        ResultItem item;
        item.setText(QString::fromUtf8(m_process->readLine()).trimmed());
        if (!item.text().startsWith('#')) // skip header comments
            m_resultList->append(item);
    }
    
    sortResultsByPriority();
    
    // clean up
    delete m_process;
    m_process = 0;
    
    emit searchFinished();
}

/**
 * Sorts the results by priority.
 * The algorithm to determine the priority of an item is taken from
 * <a href="https://www-user.tu-chemnitz.de/~fri/ding/">Ding</a>.
 *
 * @param resultList a pointer to the @c #ResultList to sort
 */
void SearchEngine::sortResultsByPriority() {
    // the following expressions are independent of the search term
    // and thus can be defined as static const

    // matches line break markers
    static const QChar MULTI_LINE('|');
    // matches abbreviation markers (obsolete)
    static const QLatin1String ABBREVIATION(" : ");
    // matches text in round/curly/square brackets
    static const QRegExp IN_BRACKETS(" (?:\\([^.)]*\\)|\\{[^.}]*\\}|\\[[^]]*\\])");

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
    
    // values that are added to the result's priority depending on the
    // outcome of various regexp tests
    enum SortPriority { STARTS_WITH = 100, CONTAINS = 80, IS_ABBREVIATION = 10 };

    QMutableListIterator<ResultItem> iterator(*m_resultList);
    while(iterator.hasNext()) {
        ResultItem item = iterator.next();
        QString text = item.text();
        
        // remove all text in brackets
        text.replace(IN_BRACKETS, "");
        
        const int LINE_COUNT = text.count(MULTI_LINE);
        if (LINE_COUNT > 0) { // this item is a multi line result
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
    
    std::stable_sort(m_resultList->begin(), m_resultList->end());
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
            qDebug() << "Process is not running";
            break;
        case QProcess::Starting:
            qDebug() << "Process is starting";
            break;
        case QProcess::Running:
            qDebug() << "Process is running";
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
            qDebug() << "Process failed to start";
            break;
        case QProcess::Crashed:
            qDebug() << "Process crashed";
            break;
        case QProcess::Timedout:
            qDebug() << "Process timed out";
            break;
        case QProcess::WriteError:
            qDebug() << "Process could not be written to";
            break;
        case QProcess::ReadError:
            qDebug() << "Process could not be read from";
            break;
        case QProcess::UnknownError:
            qDebug() << "Process encountered an unknown error";
            break;
    }
}

QString SearchEngine::dictionaryVersion() const {
    // The version of the dictionary is given in the first line of the file,
    // but for robustness it's accepted anywhere in the initial comment section.
    // As of v1.5 it is of the form
    //   # Version :: 1.5 2007-04-09
    // for release versions or
    //   # Version :: devel 2007-06-10
    // for development versions of the dictionary.
    // On 2024-10-15, the format changed to:
    //   # Version: devel 2024-10-15
    static const QRegExp ver("#+\\s*Version\\s*:+\\s*(\\S(?:.*\\S)?)?\\s*",
                             Qt::CaseInsensitive);
    
    QFile dictFile(getDictionary());
    if (dictFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&dictFile);
        stream.setCodec("UTF-8");

        QString line;
        do {
            line = stream.readLine();
            if (ver.exactMatch(line)) {
                return ver.cap(1);
            }
        } while (line.startsWith('#'));
    } else if (dictFile.fileName().isEmpty()) {
        qCritical() << "Dictionary not found";
    } else {
        qCritical() << "Failed to open dictionary" << dictFile.fileName();
    }
    
    return i18nc("dictionary version", "unknown");
}

QString SearchEngine::backendVersion() const {
    return "";
}

void SearchEngine::updateSearchBackend() const {
    m_backendFactory->generate();
}

#include "moc_searchengine.cpp"
