#pragma once

#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <system_error>

#include <spirv_glsl.hpp>

struct CustomFloatFormatter : spirv_cross::FloatFormatter
{
    static uint32_t bits(float v)
    {
        uint32_t u;
        std::memcpy(&u, &v, sizeof(u));
        return u;
    }

    static bool same_float(float a, float b)
    {
        return bits(a) == bits(b);
    }

    static std::string ensure_glsl_float_syntax(std::string s)
    {
        if (s == "-0") s = "0";
        if (s == "inf") return "1.0/0.0";
        if (s == "-inf") return "-1.0/0.0";
        if (s == "nan") return "0.0/0.0";

        if (s.find_first_of(".eE") == std::string::npos)
            s += ".0";
        return s;
    }

    static std::string ensure_glsl_double_syntax(std::string s)
    {
        if (s == "-0") s = "0";
        if (s == "inf") return "1.0lf/0.0lf";   // or adapt to what you want
        if (s == "-inf") return "-1.0lf/0.0lf";
        if (s == "nan") return "0.0lf/0.0lf";

        if (s.find_first_of(".eE") == std::string::npos)
            s += ".0";
        if (s.find_first_of("lfLF") == std::string::npos)
            s += "lf";
        return s;
    }

    std::string format_float(float v) override
    {
        if (std::isnan(v)) return "0.0/0.0";
        if (std::isinf(v)) return v < 0.0f ? "-1.0/0.0" : "1.0/0.0";
        if (v == 0.0f) return "0.0";

        char buf[64];
        auto r = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general);
        if (r.ec == std::errc{})
        {
            std::string s(buf, r.ptr);

            float parsed = 0.0f;
            auto rp = std::from_chars(s.data(), s.data() + s.size(), parsed, std::chars_format::general);
            if (rp.ec == std::errc{} && same_float(v, parsed))
                return ensure_glsl_float_syntax(s);
        }

        r = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general,
                          std::numeric_limits<float>::max_digits10);
        return ensure_glsl_float_syntax(std::string(buf, r.ptr));
    }

    std::string format_double(double v) override
    {
        if (std::isnan(v)) return "0.0lf/0.0lf";
        if (std::isinf(v)) return v < 0.0 ? "-1.0lf/0.0lf" : "1.0lf/0.0lf";
        if (v == 0.0) return "0.0lf";

        char buf[128];
        auto r = std::to_chars(buf, buf + sizeof(buf), v, std::chars_format::general,
                               std::numeric_limits<double>::max_digits10);
        return ensure_glsl_double_syntax(std::string(buf, r.ptr));
    }
};
