#include "pch.h"
#include "USDImporter.h"


class usdiContext
{
public:
    bool open(const char *path);

private:
    UsdStageRefPtr m_stage;
    double m_start_time = 0.0;
    double m_end_time = 0.0;
};

bool usdiContext::open(const char *path)
{
    m_stage = UsdStage::Open(path);
    if (m_stage == UsdStageRefPtr()) {
        return false;
    }

    m_start_time = m_stage->GetStartTimeCode();
    m_end_time = m_stage->GetEndTimeCode();
}

usdiContext* usdiOpen(const char *path)
{
    auto* ret = new usdiContext();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}
