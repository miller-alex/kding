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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef RESULTLIST_H
#define RESULTLIST_H

#include <QList>
#include <QString>

/**
 * A @c ResultItem holds an entry from the dictionary as well as the priority
 * assigned to this entry for a given request.
 */
class ResultItem {
public:
    ResultItem(QString text = QString(), int prio = 0);
    ResultItem(const ResultItem& other);
    ~ResultItem();
    
    ResultItem& operator=(const ResultItem& other);
    
    QString text() const;
    void setText(const QString& text);
    int priority() const;
    void setPriority(const int priority);
    void addToPriority(const int priority);
    
    bool operator<(const ResultItem& other) const;
    
private:
    QString m_text;
    int m_priority;
};

/**
 * A @c #ResultList is a @c QList of @c ResultItem s.
 */
typedef QList<ResultItem> ResultList;

#endif
