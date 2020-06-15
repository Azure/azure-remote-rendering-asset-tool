#pragma once

class Cancellable
{
public:
    virtual ~Cancellable(){};
    virtual void cancel() = 0;
};
