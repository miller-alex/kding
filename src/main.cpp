/**
 * @mainpage
 *
 * @section desc Description
 *
 * KDing is a KDE port of
 * <a href="https://www-user.tu-chemnitz.de/~fri/ding/">Ding</a>, a dictionary
 * lookup program. It sits in KDE's system tray and can translate the current
 * clipboard content. Users can also enter single words or phrases for
 * translation. It is intended to be used for translating between German and
 * English, but can be used with every language for which a word list is
 * available for
 * <a href="https://www-user.tu-chemnitz.de/~fri/ding/#other">Ding</a>.
 *
 * @section license License
 *
 * Copyright (c) 2009 Michael Rex <me@rexi.org>
 * Copyright (c) 2017, 2021 Alexander Miller <alex.miller@gmx.de>
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

#include "mainwindow.h"
#include "application.h"
#include <KAboutData>
#include <Kdelibs4ConfigMigrator>
#include <KLocalizedString>
#include <KStartupInfo>
#include <QCommandLineParser>
#include <QIcon>

void migrateConfig() {
    Kdelibs4ConfigMigrator migrator(QStringLiteral("kding"));

    // list of all config files
    migrator.setConfigFiles(QStringList() << QStringLiteral("kdingrc"));
    // list of KXMLGUI files
    migrator.setUiFiles(QStringList() << QStringLiteral("kdingui.rc"));

    migrator.migrate();
}

int main(int argc, char* argv[]) {
    migrateConfig();

    // catch startupId before it's unset by QApplication constructor
    (void) KStartupInfo::startupId();

    Application app(argc, argv);
    KLocalizedString::setApplicationDomain("kding");

    // add About dialog data
    KAboutData about("kding", i18n("KDing"), "0.7.3-dev");
    about.setShortDescription(i18n("KDE port of Ding, "
                                   "the DIctionary Nice Grep"));
    about.setLicense(KAboutLicense::GPL_V2);
    about.setCopyrightStatement(i18n(
        "(c) 2005-2009, Michael Rex\n"
        "(c) 2017, 2021, Alexander Miller\n"
        "Dictionary (c) 1995-2022, Frank Richter"));
    about.setHomepage("https://github.com/miller-alex/kding");
    about.setBugAddress("alex.miller@gmx.de");

    about.addAuthor(i18n("Alexander Miller"), i18n("Maintainer"),
                    "alex.miller@gmx.de");
    about.addAuthor(i18n("Michael Rex"), i18n("Original author"),
                    "me@rexi.org");
    about.addCredit(i18n("Frank Richter"),
                    i18n("Author of Ding, for the inspiration for KDing "
                         "and the dictionary"),
                    0, "https://www-user.tu-chemnitz.de/~fri/ding/");
    about.addCredit(i18n("Alexander Stein"), i18n("Initial port to KDE4"),
                    "alexander.stein@informatik.tu-chemnitz.de");

    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                        i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    KAboutData::setApplicationData(about);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kding")));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(i18nc("command line arg name", "phrase"),
				 i18n("Translate the given phrase"),
				 i18nc("command line arg syntax", "[phrase]"));
    parser.process(app);

    return app.exec();
}
