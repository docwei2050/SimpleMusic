//
// Created by yangw on 2018-4-1.
//

#ifndef WLMUSIC_PCMBEAN_H
#define WLMUSIC_PCMBEAN_H

#include <SoundTouch.h>

using namespace soundtouch;

class PcmBean {

public:
    char *buffer;
    int buffsize;

public:
    PcmBean(SAMPLETYPE *buffer, int size);
    ~PcmBean();


};


#endif //WLMUSIC_PCMBEAN_H
