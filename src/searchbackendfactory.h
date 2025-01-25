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
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SEARCHBACKENDFACTORY_H
#define SEARCHBACKENDFACTORY_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * This class generates the command line to use the backend and options
 * selected by the user.
 */
class SearchBackendFactory : public QObject {
    Q_OBJECT

public:
    SearchBackendFactory(QObject* parent = 0);
    ~SearchBackendFactory();
    
    QString executable() const;
    QStringList argumentList() const;
    
    static bool hasAgrep();
    
public slots:
    void generate();
    
private:
    static const QString EGREP_CMD;
    static const QStringList EGREP_DEFAULT_ARGS;
    
    static const QString AGREP_CMD;
    static const QStringList AGREP_DEFAULT_ARGS;
    
    static const QString CASE_INSENSITIVE_ARG;
    static const QString WHOLE_WORDS_ARG;
    static const QString MAXIMUM_ERROR_NUMBER_ARG;
    
    void generateEgrepCmdLine();
    void generateAgrepCmdLine();
    void addBasicOptions();
    void addAdvancedOptions();
    
    QString m_executable;
    QStringList m_argumentList;
};

#endif
