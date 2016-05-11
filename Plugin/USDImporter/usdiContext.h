#pragma once

namespace usdi {

class Context
{
public:
    Context();
    ~Context();
    void unload();

    bool open(const char *path);

private:
    typedef std::map<std::string, std::string> Variants;

    UsdStageRefPtr m_stage;
    double m_start_time = 0.0;
    double m_end_time = 0.0;

    std::string m_prim_root;
    Variants m_variants;
};

} // namespace usdi
