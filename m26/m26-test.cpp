#include "m26-test.h"

m26test::m26test(QString rtmp_url)
{
        m26_a_control = new mb86m26_control(this, QString("gpio457"));
        streamer = new Rtmp_streamer(this);

        QObject::connect(m26_a_control, &mb86m26_control::data_ready, streamer, &Rtmp_streamer::data_ready);
        QObject::connect(m26_a_control, &mb86m26_control::restart, streamer, &Rtmp_streamer::restart);

        m26_a_control->set_std(STD_1080i50);
        streamer->start();

        streamer->set_url(rtmp_url);
        streamer->connect();

}

m26test::~m26test()
{
        delete m26_a_control;
        delete streamer;
}