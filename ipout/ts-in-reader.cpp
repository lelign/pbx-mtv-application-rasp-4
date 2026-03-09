#include "ts-in-reader.h"
#include <QDebug>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define QUEUE_SIZE (256)
#define BUF_SIZE (16*1024)

void TsInReader::run()
{
        uint8_t buf[BUF_SIZE];
        int ret;

        f_ts = open(fname, O_RDONLY);
        while(thread_exit==0){
                ret = read(f_ts, buf, BUF_SIZE);
                push_data(buf, ret);
        }
        close(f_ts);
}

void TsInReader::stop()
{
        thread_exit = 1;
        wait();
}

TsInReader::TsInReader(const char * fname):
        TsReader(),
        QThread(NULL),
        fname(fname),
        thread_exit(0),
        f_ts(0)
{

}