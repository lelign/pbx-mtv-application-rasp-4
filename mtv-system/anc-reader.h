#if !defined(ANC_READER)
#define ANC_READER

#include "../ipout/ts-in-reader.h"
#include <QByteArray>

class AncReader : public QThread
{
        Q_OBJECT
public:
        AncReader(const char * in_fname, QObject *parent);
        void stop();
        void run() override;
private:
        bool show_debug = true;
        int thread_exit;
        int readout_cnt;
        void process(data_t data);
protected:
        TsInReader * ts_reader;
Q_SIGNALS:
        void scte_104_data(int channel, QByteArray data);
        void op47_data(int channel, QByteArray data);
        void op42_data(int channel, QByteArray data);
};


#endif
