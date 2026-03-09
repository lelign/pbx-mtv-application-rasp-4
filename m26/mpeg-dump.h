#ifndef MPEG_DUMP_H
#define MPEG_DUMP_H

#include <QString>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <libusb.h>
#include <QTimer>
#include <QByteArray>
#include <QElapsedTimer>


class Mpeg_dump : public QObject
{
        Q_OBJECT
public:
        Mpeg_dump();
        ~Mpeg_dump();
private:
        FILE * f;
Q_SIGNALS:
public Q_SLOTS:
        void data_ready(uint8_t * data, int len);
};

#endif
