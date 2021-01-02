//
// Created by docwei on 2020/10/22.
//

#ifndef SIMPLEMUSIC_PLAYSTATUS_H
#define SIMPLEMUSIC_PLAYSTATUS_H


class PlayStatus {
public:
    bool exit = false;
    bool load = true;
    bool seek = false;
    bool pause=false;

    PlayStatus();

    ~PlayStatus();
};


#endif //SIMPLEMUSIC_PLAYSTATUS_H
