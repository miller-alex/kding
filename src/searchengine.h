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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "resultlist.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QRegExp>

class MainWindow;
class SearchBackendFactory;

/**
 * This class implements the search engine that looks up phrases in the
 * dictionary and provides a @c #ResultList of matching entries.
 */
class SearchEngine : public QObject {
    Q_OBJECT

public:
    SearchEngine(QObject* parent = 0);
    ~SearchEngine();
    
    QString searchTerm() const;
    ResultList& results() const;
    QString dictionaryVersion() const;
    QString backendVersion() const;
    
signals:
    void searchStarted();
    void searchFinished();
    void searchRunning();
    void statusMessage(QString message);
    
public slots:
    void search(QString phrase);
    void cancelSearch();
    void updateSearchBackend() const;
    
private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processFailed(QProcess::ProcessError error) const;
    void monitorProcess(QProcess::ProcessState newState) const;
    
private:
    /// This enum defines values that are added to the result's priority
    /// depending on the outcome of various regexp tests
    enum SortPriority { STARTS_WITH = 100, CONTAINS = 80, IS_ABBREVIATION = 10 };
    
    // the following regular expressions are independent of the search term
    // and thus can be defined at class level
    static const QRegExp MULTI_LINE;
    static const QRegExp ABBREVIATION;
    static const QRegExp ROUND_BRACKETS;
    static const QRegExp CURLY_BRACKETS;
    static const QRegExp SQUARE_BRACKETS;
    
    // cannot make this a static member because an application instance
    // is needed to use QStandardPaths
    const QString DEFAULT_DICTIONARY;
    
    const QString DICTIONARY_VERSION;
    
    void sortResultsByPriority(ResultList* resultList) const;
    QString determineDictionaryVersion();
    QString determineBackendVersion();
    
    QProcess* m_process;
    QString m_searchTerm;
    ResultList* m_resultList;
    SearchBackendFactory* m_backendFactory;
};

#endif
