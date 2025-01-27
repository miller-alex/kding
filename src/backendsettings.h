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

#ifndef BACKENDSETTINGS_H
#define BACKENDSETTINGS_H

#include "ui_backendsettingsbase.h"
#include <QWidget>

/**
 * This class implements the "Backend" section of the configuration dialog.
 */
class BackendSettings : public QWidget, public Ui::BackendSettingsBase {
    Q_OBJECT

public:
    BackendSettings(QWidget* parent = 0);
    ~BackendSettings();
};

#endif
