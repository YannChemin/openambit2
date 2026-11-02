/*
 * (C) Copyright 2013 Emil Ljungdahl
 *
 * This file is part of Openambit.
 *
 * Openambit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *
 */
#include <unistd.h>
#include "mainwindow.h"
#include <QSettings>
#include <QStyleFactory>
#include <QPalette>
#include <QProcess>

#include "i18n.h"
#include "single_application.h"
#include "signalhandler.h"

static bool isGnomeDarkMode()
{
    QProcess p;
    p.start("gsettings", {"get", "org.gnome.desktop.interface", "color-scheme"});
    if (!p.waitForFinished(500)) return false;
    return p.readAllStandardOutput().contains("dark");
}

static void applySystemTheme(QApplication &app)
{
    app.setStyle(QStyleFactory::create("Fusion"));
    if (!isGnomeDarkMode()) return;

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x24, 0x24, 0x24));
    dark.setColor(QPalette::WindowText,      QColor(0xee, 0xee, 0xee));
    dark.setColor(QPalette::Base,            QColor(0x1e, 0x1e, 0x1e));
    dark.setColor(QPalette::AlternateBase,   QColor(0x2c, 0x2c, 0x2c));
    dark.setColor(QPalette::ToolTipBase,     QColor(0x1e, 0x1e, 0x1e));
    dark.setColor(QPalette::ToolTipText,     QColor(0xee, 0xee, 0xee));
    dark.setColor(QPalette::Text,            QColor(0xee, 0xee, 0xee));
    dark.setColor(QPalette::Button,          QColor(0x35, 0x35, 0x35));
    dark.setColor(QPalette::ButtonText,      QColor(0xee, 0xee, 0xee));
    dark.setColor(QPalette::BrightText,      Qt::red);
    dark.setColor(QPalette::Link,            QColor(0x35, 0x84, 0xe4));
    dark.setColor(QPalette::Highlight,       QColor(0x35, 0x84, 0xe4));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    dark.setColor(QPalette::Disabled, QPalette::Text,       QColor(0x88, 0x88, 0x88));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x88, 0x88, 0x88));
    app.setPalette(dark);
}

int main(int argc, char *argv[])
{
    // Set application settings
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName("Openambit");
    QCoreApplication::setApplicationName("Openambit");

    for(int x = 1; x < argc; ++x) {
        if (QString(argv[x]) == "--version") {
            printf("%s - Version %s\n", "Openambit", APP_VERSION);
            return 0;
        }
    }
    // Handle foreground arguments
    // NOTE: It would be preferable to handle all arguments at the same place,
    // but fork needs to be done before Qt initialize it seems
    bool background = true;
    for (int i=0; i<argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            background = false;
            break;
        }
    }
    if (background) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        // Windows
#else
        // Fork for background running
        if ( fork() > 0 ) {
            // Exit the parent process
            return 0;
        }
        // Set the child to the new process group leader
        setsid();
#endif
    }

    SingleApplication a(argc, argv, "openambit_single_application_lock");

    if (a.isRunning()) {
        a.sendMessage("focus");
        return 0;
    }

    applySystemTheme(a);

    // Handle forced localisation / translation ("-locale:fr_FR")
    QString forcedLocale;
    Q_FOREACH(QString argu, a.arguments()) {
        const QString localeParam = QStringLiteral("-locale:");
        if (argu.startsWith(localeParam)) {
           forcedLocale = argu.mid(localeParam.length());
           break;
        }
    }

    // Initialize translations from the forced locale, the settings or the
    // environment (LANGUAGE, LC_ALL, LC_MESSAGES, LANG)
    I18n::install(forcedLocale);

    MainWindow w;

    // Connect single application message bus
    QObject::connect(&a, SIGNAL(messageAvailable(QString)), &w, SLOT(singleApplicationMsgRecv(QString)));

    // Handle signals
    SignalHandler sigHandler;
    QObject::connect(&sigHandler, SIGNAL(signalReceived(int)), &w, SLOT(closeRequested()));

    w.show();

    return a.exec();
}
