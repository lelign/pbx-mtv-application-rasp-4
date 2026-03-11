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

/*
 * pbx-mtv-508-main.cpp     34      PbxMtv508 mtv508;
 * pbx-mtv-508.h            25 -31  explicit PbxMtv508();
 * pbx-mtv-508.cpp          20      PbxMtv508::PbxMtv508(       qDebug() << "Program start";
 * pbx-mtv-508.cpp          28      mtvsystem = new PbxMtvSystem
 * mtv-system.cpp           265     anc_reader = new AncReader(ANCIN, this); anc_reader->start();
 *      anc-reader.cpp data.size :  0
        ts-reader.cpp 47  wait_satus :  false  data_invalid
        anc-reader.cpp data.size :  0

 *
 *
 * */
