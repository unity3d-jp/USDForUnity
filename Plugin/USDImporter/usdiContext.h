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
    typedef std::unique_ptr<Schema> SchemaPtr;
    typedef std::vector<SchemaPtr> Schemas;

    UsdStageRefPtr m_stage;
    Schemas m_schemas;
    double m_start_time = 0.0;
    double m_end_time = 0.0;

    std::string m_prim_root;
    Variants m_variants;
};

} // namespace usdi
