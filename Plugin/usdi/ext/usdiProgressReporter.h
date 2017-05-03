#pragma once

namespace usdi {

class IProgressReporter
{
public:
    virtual ~IProgressReporter() {}
    virtual void write(const char *message) = 0;
};

IProgressReporter* CreateProgressReporter();

} // namespace usdi
