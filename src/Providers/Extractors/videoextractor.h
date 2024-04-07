#pragma once

#include "Providers/Components/functions.h"
#include "Common/network.h"

class VideoExtractor
{
public:
    VideoExtractor(){};
    ~VideoExtractor(){};

    virtual QString extractLink(std::string link) = 0;
};


