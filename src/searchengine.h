/*
 * Copyright (c) 2009 Michael Rex <me@rexi.org>
 * Copyright (c) 2024 Alexander Miller <alex.miller@gmx.de>
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

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "resultlist.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QRegExp>

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
    void sortResultsByPriority();
    
    QProcess* m_process;
    QString m_searchTerm;
    ResultList* m_resultList;
    SearchBackendFactory* m_backendFactory;
};

#endif
