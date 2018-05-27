#pragma once

#include "BFState.hh"

class JittedFun
{
  public:
    virtual void call(BFState *) = 0;
};
