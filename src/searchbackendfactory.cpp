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

#include "searchbackendfactory.h"
#include "settings.h"
#include <QProcess>
#include <QDebug>

const QString SearchBackendFactory::EGREP_CMD = "grep";
const QStringList SearchBackendFactory::EGREP_DEFAULT_ARGS = {
    "-E",   // egrep variant (extended REs)
    "-h",   // do not display filenames
};
const QString SearchBackendFactory::AGREP_CMD = "agrep";
const QStringList SearchBackendFactory::AGREP_DEFAULT_ARGS = {
    "-h",   // do not display filenames
};
const QString SearchBackendFactory::CASE_INSENSITIVE_ARG = "-i";
const QString SearchBackendFactory::WHOLE_WORDS_ARG = "-w";
const QString SearchBackendFactory::MAXIMUM_ERROR_NUMBER_ARG = "-%1";

SearchBackendFactory::SearchBackendFactory(QObject* parent) : QObject(parent) {
    generate();
}

SearchBackendFactory::~SearchBackendFactory() {
}

/**
 * Generate the command line.
 * Depending on the settings, this selects the appropriate backend and command
 * line switches.
 */
void SearchBackendFactory::generate() {
    switch(Settings::self()->selectedBackend()) {
        case Settings::EnumSelectedBackend::egrep:
            generateEgrepCmdLine();
            break;
        case Settings::EnumSelectedBackend::agrep:
            generateAgrepCmdLine();
            break;
        default:
            qCritical() << "Invalid value when specifying backend";
    }
}

/**
 * Generate the command line for using @c egrep.
 */
void SearchBackendFactory::generateEgrepCmdLine() {
    m_executable = EGREP_CMD;
    m_argumentList = EGREP_DEFAULT_ARGS;
    
    addBasicOptions();
}

/**
 * Generate the command line for using @c agrep.
 */
void SearchBackendFactory::generateAgrepCmdLine() {
    m_executable = AGREP_CMD;
    m_argumentList = AGREP_DEFAULT_ARGS;
    
    addBasicOptions();
    addAdvancedOptions();
}

/**
 * Adds basic options to the command line.
 * Options are considered @em basic when they are independent of the backend.
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
 * Adds advanced options to the command line.
 * Options are considered @em advanced when they are supported by @c agrep only.
 */
void SearchBackendFactory::addAdvancedOptions() {
    const Settings& settings = *Settings::self();
    
    m_argumentList << MAXIMUM_ERROR_NUMBER_ARG.arg(settings.maximumErrorNumber());
}

/**
 * This method tries to determine whether @c agrep is available.
 * It does so by trying to run it. In case this fails we assume that @c agrep is
 * not present on the system.
 *
 * @return @c true if @c agrep was found, @c false otherwise
 *
 * @see QProcess::execute()
 */
bool SearchBackendFactory::hasAgrep() {
    if(QProcess::execute(AGREP_CMD, QStringList()) != -2) {
        return true;
    } else {
        return false;
    }
}

/**
 * Returns the name of the executable to use.
 *
 * @return the executable to use
 */
QString SearchBackendFactory::executable() const {
    return m_executable;
}

/**
 * Returns the argument list to use.
 *
 * @return @c QStringList containing the seperate arguments to use
 */
QStringList SearchBackendFactory::argumentList() const {
    return m_argumentList;
}

#include "moc_searchbackendfactory.cpp"
