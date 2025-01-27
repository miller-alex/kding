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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include "ui_generalsettingsbase.h"
#include <QWidget>

/**
 * This class implements the "General Settings" section of the configuration
 * dialog.
 */
class GeneralSettings : public QWidget, public Ui::GeneralSettingsBase {
    Q_OBJECT

public:
    GeneralSettings(QWidget* parent = 0);
    ~GeneralSettings();
};

#endif
