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

#include "searchbackendfactory.h"
#include "settings.h"
#include <KDebug>

const QString SearchBackendFactory::EGREP_CMD = "egrep";
const QStringList SearchBackendFactory::EGREP_DEFAULT_ARGS = QStringList() \
    << "-h";    /* do not display filenames */

const QString SearchBackendFactory::AGREP_CMD = "agrep";
const QStringList SearchBackendFactory::AGREP_DEFAULT_ARGS = QStringList() \
    << "-h";    /* do not display filenames */

const QString SearchBackendFactory::CASE_INSENSITIVE_ARG = "-i";
const QString SearchBackendFactory::WHOLE_WORDS_ARG = "-w";
const QString SearchBackendFactory::MAXIMUM_ERROR_NUMBER_ARG = "-%1";

SearchBackendFactory::SearchBackendFactory(QObject* parent) : QObject(parent) {
    generate();
}

SearchBackendFactory::~SearchBackendFactory() {
    
}

/**
 *
 */
void SearchBackendFactory::generate() {
    kDebug() << "Generating backend";
    
    switch(Settings::self()->selectedBackend()) {
        case Settings::EnumSelectedBackend::egrep:
            generateEgrepCmdLine();
            break;
        case Settings::EnumSelectedBackend::agrep:
            generateAgrepCmdLine();
            break;
        default:
            kError() << "Invalid value when specifying backend";
    }
    
    kDebug() << "Executable:" << m_executable;
    kDebug() << "Argument List:" << m_argumentList;
}

/**
 *
 */
void SearchBackendFactory::generateEgrepCmdLine() {
    kDebug() << "egrep selected";
    m_executable = EGREP_CMD;
    m_argumentList = EGREP_DEFAULT_ARGS;
    addBasicOptions();
}

/**
 *
 */
void SearchBackendFactory::generateAgrepCmdLine() {
    kDebug() << "agrep selected";
    m_executable = AGREP_CMD;
    m_argumentList = AGREP_DEFAULT_ARGS;
    addBasicOptions();
    addAdvancedOptions();
}

/**
 *
 */
void SearchBackendFactory::addBasicOptions() {
    const Settings& settings = *Settings::self();
    
    if(!settings.caseSensitive()) {
        m_argumentList << CASE_INSENSITIVE_ARG;
    }
    
    if(settings.wholeWords()) {
        m_argumentList << WHOLE_WORDS_ARG;
    }
}

/**
 *
 */
void SearchBackendFactory::addAdvancedOptions() {
    const Settings& settings = *Settings::self();
    
    m_argumentList << MAXIMUM_ERROR_NUMBER_ARG.arg(settings.maximumErrorNumber());
}

/**
 *
 * @return 
 */
QString SearchBackendFactory::executable() const {
    return m_executable;
}

/**
 *
 * @return 
 */
QStringList SearchBackendFactory::argumentList() const {
    return m_argumentList;
}

#include "searchbackendfactory.moc"
