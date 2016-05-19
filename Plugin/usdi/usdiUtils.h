#pragma once

namespace usdi {

// Body: [](double time, const Type& value) -> void
template<class Type, class Body>
inline void EachTimeSamples(UsdAttribute& attr, const Body& body)
{
    if (!attr) { return; }

    std::vector<double> timesample;
    if (attr.GetTimeSamples(&timesample)) {
        Type value;
        for (double time : timesample) {
            if (attr.Get(&value)) {
                body(time, value);
            }
        }
    }
}

} // namespace usdi
