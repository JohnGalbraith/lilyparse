#include <stan/driver/debug.hpp>
#include <stan/duration.hpp>

#include <fmt/format.h>
#include <numeric>

namespace stan::driver::debug {

writer write;

std::string writer::operator()(rational<std::uint16_t> const &r) const
{
    return fmt::format("{}/{}", r.num(), r.den());
}

std::string writer::operator()(pitch const &r) const
{
    return fmt::format("{}{}",
                       pitchclass_names.at(r.m_pitchclass),
                       std::to_string(static_cast<std::uint8_t>(r.m_octave)));
}

std::string writer::operator()(value const &v) const
{
    return std::to_string(v.den() / (1u << v.dots())) + std::string(v.dots(), '.');
}

std::string writer::operator()(duration const &v) const
{
    return fmt::format("{}/{}", v.num(), v.den());
}

std::string writer::operator()(rest const &r) const
{
    static writer write;
    return fmt::format("r:{}", write(r.m_value));
}

std::string writer::operator()(note const &r) const
{
    static writer write;
    return fmt::format("{}:{}", write(r.m_pitch), write(r.m_value));
}

std::string writer::operator()(chord const &r) const
{
    static writer write;

    std::string pitches = std::accumulate(
        r.m_pitches.begin(),
        r.m_pitches.end(),
        std::string(),
        [](std::string res, pitch p) { return res + write(p) + " "; });
    pitches.resize(pitches.size() - 1);
    return fmt::format("<{}>:{}", pitches, write(r.m_value));
}

std::string writer::operator()(beam const &r) const
{
    static writer write;

    std::string elements = std::accumulate(
        r.m_elements.begin(),
        r.m_elements.end(),
        std::string(),
        [](std::string res, const auto &p) { return res + write(p) + " "; });
    elements.resize(elements.size() - 1);
    return fmt::format("[{}]", elements);
}

std::string writer::operator()(tuplet const &r) const
{
    static writer write;

    std::string elements = std::accumulate(
        r.m_elements.begin(),
        r.m_elements.end(),
        std::string(),
        [](std::string res, const auto &p) { return res + write(p) + " "; });
    elements.resize(elements.size() - 1);
    return fmt::format("{}:{{{}}}]", write(r.m_value), elements);
}

std::string writer::operator()(std::unique_ptr<column> const &ptr) const
{
    return operator()(*ptr);
}

std::string writer::operator()(column const &col) const
{
    static writer write;
    return std::visit([](auto &&v) { return write(v); }, col);
}

} // namespace stan::driver::debug
