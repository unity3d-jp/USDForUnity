#include "pch.h"
#include "usdiProgressReporter.h"

namespace usdi {

class ProgressReporter : public IProgressReporter
{
public:
    ProgressReporter();
    ~ProgressReporter() override;
    void write(const char *message) override;

private:
    static int s_ref_count;
#ifdef _WIN32
    HANDLE m_cs_in = nullptr;
    HANDLE m_cs_out = nullptr;
#endif
};


int ProgressReporter::s_ref_count = 0;

ProgressReporter::ProgressReporter()
{
    if (s_ref_count++ == 0) {
#ifdef _WIN32
        ::AllocConsole();
#endif
    }
#ifdef _WIN32
    m_cs_in = ::GetStdHandle(STD_INPUT_HANDLE);
    m_cs_out = ::GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}
ProgressReporter::~ProgressReporter()
{
    if (--s_ref_count == 0) {
#ifdef _WIN32
        ::FreeConsole();
#endif
    }
}

void ProgressReporter::write(const char *message)
{
#ifdef _WIN32
    DWORD written = 0;
    WriteConsoleA(m_cs_out, message, (DWORD)strlen(message), &written, nullptr);
#else
    printf(message);
#endif
}


IProgressReporter* CreateProgressReporter()
{
    return new ProgressReporter();
}

} // namespace usdi
