#include <stan/copy.hpp>

namespace stan {

template <typename T>
column copy_visitor::operator()(const T &v) const
{
    return v;
}

column copy_visitor::operator()(const beam &v) const
{
    std::vector<column> elements;
    std::transform(
        v.m_elements.begin(),
        v.m_elements.end(),
        std::back_inserter(elements),
        [this](const column &c) { return column(std::visit(*this, c)); });
    return beam{ elements };
}

column copy_visitor::operator()(const tuplet &v) const
{
    std::vector<column> elements;
    std::transform(
        v.m_elements.begin(),
        v.m_elements.end(),
        std::back_inserter(elements),
        [this](const column &c) { return column(std::visit(*this, c)); });
    return tuplet{ v.m_value, elements };
}

template column copy_visitor::operator()<rest>(const rest &) const;
template column copy_visitor::operator()<note>(const note &) const;
template column copy_visitor::operator()<chord>(const chord &) const;
}

