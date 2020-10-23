//
// Created by docwei on 2020/9/10.
//

#include "SimpleAudio.h"



SimpleAudio::SimpleAudio(PlayStatus *playStatus) {
  this->playStatus=playStatus;
  queue=new SimpleQueue(playStatus);
}
