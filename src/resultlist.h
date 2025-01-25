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
    ResultItem(QString text = QString(), int prio = 0)
      : m_text(text), m_priority(prio) {}
    ResultItem(const ResultItem& other) = default;
    ~ResultItem() = default;
    
    ResultItem& operator=(const ResultItem& other) = default;
    
    QString text() const { return m_text; }
    void setText(const QString& text) { m_text = text; }

    int priority() const { return m_priority; }
    void setPriority(const int priority) { m_priority = priority; }
    void addToPriority(const int priority) { m_priority += priority; }
    
    bool operator<(const ResultItem& other) const {
        // items with higher priority should be sorted first, hence the inversion
        return priority() > other.priority();
    }
    
private:
    QString m_text;
    int m_priority;
};

/**
 * A @c #ResultList is a @c QList of @c ResultItem s.
 */
typedef QList<ResultItem> ResultList;

#endif
