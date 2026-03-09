#include <QApplication>
#include <signal.h>
#include <QCommandLineParser>
#include <QDebug>
#include <QObject>
#include "pbx-mtv-508.h"

QT_USE_NAMESPACE

void app_exit(int)
{
        QCoreApplication::quit();
}

int main(int argc, char *argv[])
{
        QApplication a(argc, argv);
        qDebug() << "PBX-MTV-508 application";
        QCommandLineParser parser;
        QCommandLineOption watchdog_option("w", "watchdog");
        parser.setApplicationDescription("PBX-MTV-508 applicaton");
        parser.addOption(watchdog_option);
        parser.addHelpOption();
        parser.process(a);

        signal(SIGINT, app_exit);
        signal(SIGPIPE, SIG_IGN);

        bool watchdog = parser.isSet(watchdog_option);
        PbxMtv508 mtv508(watchdog);

        return a.exec();
}
