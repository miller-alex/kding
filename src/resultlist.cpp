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

#include "resultlist.h"

ResultItem::ResultItem(QString text, int prio)
  : m_text(text),
    m_priority(prio)
{}

ResultItem::ResultItem(const ResultItem& other)
  : m_text(other.m_text),
    m_priority(other.m_priority)
{}

ResultItem::~ResultItem() {
}

ResultItem& ResultItem::operator=(const ResultItem& other) {
    m_text = other.m_text;
    m_priority = other.m_priority;
    
    return *this;
}

QString ResultItem::text() const {
    return m_text;
}

void ResultItem::setText(const QString& text) {
    m_text = text;
}

int ResultItem::priority() const {
    return m_priority;
}

void ResultItem::setPriority(const int priority) {
    m_priority = priority;
}

void ResultItem::addToPriority(const int priority) {
    m_priority += priority;
}

bool ResultItem::operator<(const ResultItem& other) const {
    // items with higher priority should be sorted first, hence the inversion
    return priority() > other.priority();
}
