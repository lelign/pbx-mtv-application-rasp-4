#include <QLoggingCategory>

#include <bitstream/mpeg/ts.h>

#include "hlsserver.h"

static QLoggingCategory category("HLS Server");

#define VIDEO_PID (101)
#define TARGET_DURATION (2)
#define CHUNK_COUNT (10)

HlsServer::HlsServer(QString path_to_video_folder)
{
    qDebug(category) << "Creating...";

    this->path_to_video_folder = path_to_video_folder;
    delete_files_from_folder(path_to_video_folder, "*.m3u8");
    delete_files_from_folder(path_to_video_folder, "*.ts");
    split_counter = 0;

    cnt_chunk = 0;
    set_media_file_handle(cnt_chunk);

    if(this->path_to_video_folder == "")
        PlayList_FileName = "playlist.m3u8";
    else
        PlayList_FileName = path_to_video_folder + "playlist.m3u8";

}

HlsServer::~HlsServer()
{
    media_file_handle.close();
}

QString HlsServer::get_media_FileName(qint32 num)
{
    return  QString("media_%1.ts").arg(num);
}

QString HlsServer::get_full_media_FileName(qint32 num)
{
QString media_FileName;

    if(path_to_video_folder == "")
        media_FileName = get_media_FileName(num);
    else
        media_FileName = path_to_video_folder + get_media_FileName(num);

    return media_FileName;
}

void HlsServer::make_playlist_file(quint32 cnt_chunk, quint32 target_duration)
{
static QList<chunk_list_t> chunk_list;
qint32 sequence_number;

    QFile file(PlayList_FileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
       qDebug(category) << PlayList_FileName << file.errorString();
       return;
    }

    chunk_list_t chunk;
    chunk.sequence_number = cnt_chunk;
    chunk.ext_inf = target_duration;
    chunk.file_name = get_media_FileName(cnt_chunk);
    chunk_list.append(chunk);

    if(chunk_list.size() > CHUNK_COUNT) chunk_list.removeFirst();

    sequence_number = chunk_list[0].sequence_number;

    QTextStream out(&file);
    out << QString("#EXTM3U\n");
    out << QString("#EXT-X-VERSION:3\n");
    out << QString("#EXT-X-MEDIA-SEQUENCE:%1\n").arg(sequence_number);
    out << QString("#EXT-X-TARGETDURATION:%1\n").arg(target_duration);

    for(int i = 0; i < chunk_list.size(); i++){
        out << QString("#EXTINF:%1\n").arg(chunk_list[i].ext_inf);
        out << QString("%1\n").arg(chunk_list[i].file_name);
    }

    file.close();
}

void HlsServer::split_media()
{
    QMutexLocker locker(&mutex);

    media_file_handle.close();

    make_playlist_file(cnt_chunk, TARGET_DURATION);

    cnt_chunk++;
    set_media_file_handle(cnt_chunk);

    // delete old files
    QString old_FileName = get_full_media_FileName(cnt_chunk - CHUNK_COUNT - 2);
    QFile file(old_FileName);
    if(file.exists()) file.remove();
 }

void HlsServer::set_media_file_handle(qint32 cnt_chunk)
{
    media_FileName = get_full_media_FileName(cnt_chunk);

    media_file_handle.setFileName(media_FileName);
    if(!media_file_handle.open(QIODevice::WriteOnly | QIODevice::Text)){
       qDebug(category) << media_FileName << media_file_handle.errorString();
    }
}

void HlsServer::add_packet(const char * data, int size)
{
    if(is_frame_start(data, size)){
        split_counter = (split_counter+1) % (25*TARGET_DURATION);
        if(split_counter==0)
            split_media();
    }
    media_file_handle.write(data, size);
}

void HlsServer::delete_files_from_folder(QString dir_name, QString filter)
{
    QDir dir(dir_name);
    dir.setNameFilters(QStringList() << filter);
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}

int HlsServer::is_frame_start(const char * data, int size)
{
    if(ts_get_unitstart((const uint8_t*) data) && ts_get_pid((const uint8_t*) data)==VIDEO_PID)
        return 1;
    else
        return 0;
}