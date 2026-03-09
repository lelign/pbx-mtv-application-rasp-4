#include <QDebug>
#include "mpeg-dump.h"

Mpeg_dump::Mpeg_dump()
{
        f = fopen("/dev/shm/dump.ts", "w+");
}

Mpeg_dump::~Mpeg_dump()
{
        fclose(f);
}

void Mpeg_dump::data_ready(uint8_t * data, int len)
{
        for(int i=0; i<8; i++)
                fwrite(data+(188+4)*i+4, 188, 1, f);
        free(data);
}
