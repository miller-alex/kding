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

#include "backendsettings.h"
#include "searchbackendfactory.h"
#include <QRadioButton>
#include <QGroupBox>

BackendSettings::BackendSettings(QWidget* parent) : QWidget(parent) {
    setupUi(this);
    
    radioAgrep->setEnabled(SearchBackendFactory::hasAgrep());
    connect(radioAgrep, SIGNAL(toggled(bool)), advancedOptionsBox, SLOT(setEnabled(bool)));
}

BackendSettings::~BackendSettings() {
    
}

#include "moc_backendsettings.cpp"
