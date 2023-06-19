#ifndef VIDEOEXTRACTOR_H
#define VIDEOEXTRACTOR_H

#include "parsers/episode.h"
#include "tools/functions.h"


class VideoExtractor
{
private:
    VideoServer videoServer;
public:
    VideoExtractor(){};
    ~VideoExtractor(){};
    virtual QString extract(std::string link) = 0;
};

#endif // VIDEOEXTRACTOR_H
