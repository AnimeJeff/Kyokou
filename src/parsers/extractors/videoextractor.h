#ifndef VIDEOEXTRACTOR_H
#define VIDEOEXTRACTOR_H

#include "tools/functions.h"


class VideoExtractor
{
public:
    VideoExtractor(){};
    ~VideoExtractor(){};

    virtual QString extractLink(std::string link) = 0;
};

#endif // VIDEOEXTRACTOR_H
