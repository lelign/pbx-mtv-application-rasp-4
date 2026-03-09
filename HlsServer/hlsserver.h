#ifndef HLSSERVER_H
#define HLSSERVER_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QDir>
#include <QMutexLocker>

class HlsServer: public QObject
{
        Q_OBJECT

public:
    HlsServer(QString PlayList_FileName = "./");
    ~HlsServer();

    void add_packet(const char * data, int size);
    void delete_files_from_folder(QString dir_name, QString filter);

private:
    struct chunk_list_t {
        qint32 sequence_number;
        qint32 ext_inf;
        QString file_name;
    };

    QMutex mutex;
    QFile media_file_handle;
    QString media_FileName;
    QString path_to_video_folder;
    quint32 cnt_chunk;

    QList<chunk_list_t> chunk_list;
    QString PlayList_FileName;
    int split_counter;
    void make_playlist_file(quint32 cnt_chunk, quint32 target_duration);
    QString get_full_media_FileName(qint32 num);
    QString get_media_FileName(qint32 num);
    void set_media_file_handle(qint32 cnt_chunk);
    void split_media();
    int is_frame_start(const char * data, int size);
};

#endif // HLSSERVER_H
