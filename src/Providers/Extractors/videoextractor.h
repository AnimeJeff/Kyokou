#ifndef VIDEOEXTRACTOR_H
#define VIDEOEXTRACTOR_H

#include "Components/functions.h"
#include "Components/network.h"

class VideoExtractor
{
public:
    VideoExtractor(){};
    ~VideoExtractor(){};

    virtual QString extractLink(std::string link) = 0;
};

#endif // VIDEOEXTRACTOR_H
