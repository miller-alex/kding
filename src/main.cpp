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

#include "mainwindow.h"
#include "application.h"
#include <KAboutData>
#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KLocale>

int main(int argc, char* argv[]) {
    // add About dialog data
    KAboutData* about = new KAboutData("kding", 0, ki18n("KDing"), "0.5.2");
    about->setShortDescription(ki18n("KDE port of Ding, the DIctionary Nice Grep"));
    about->setLicense(KAboutData::License_GPL_V2);
    about->setCopyrightStatement(ki18n("(c) 2005-2009, Michael Rex"));
    about->setHomepage("http://kding.sourceforge.net/");
    about->setBugAddress("me@rexi.org");
    
    about->addAuthor(ki18n("Michael Rex"), ki18n("Maintainer"), "me@rexi.org");
    about->addCredit(ki18n("Frank Richter"), ki18n("original author of Ding, for the inspiration for KDing and the dictionary"), 0, "http://www-user.tu-chemnitz.de/~fri/ding/");
    about->addCredit(ki18n("Alexander Stein"), ki18n("initial port to KDE4"), "alexander.stein@informatik.tu-chemnitz.de");
    
    KCmdLineArgs::init(argc, argv, about);
    
    // add command line options
    KCmdLineOptions options;
    options.add("+[phrase]", ki18n("Translate the given phrase"));
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication::addCmdLineOptions();
    
    Application app;
    
    return app.exec();
}

/**
 * @mainpage
 *
 * @section desc Description
 *
 * KDing is a KDE port of Ding, a dictionary lookup program. It sits in
 * KDE's system tray and can translate the current clipboard content. Users can
 * also enter single words or phrases for translation. It is intended to be used
 * for translating between German and English, but can be used with every
 * language for which a word list is available for Ding.
 *
 * @section license License
 *
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
