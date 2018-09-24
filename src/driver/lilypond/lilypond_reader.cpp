#include <stan/driver/lilypond.hpp>
#include <stan/driver/debug.hpp>

// #define BOOST_SPIRIT_X3_DEBUG
#include <boost/spirit/home/x3.hpp>

#include <fstream>
#include <memory>

namespace stan {

// Stan is designed with extensive safety features that make it impossible to
// construct invalid musical objects.  Most of this is static safety, including
// the lack of default constructors.  Lacking default constructors for the data
// model, Spirit X3 becomes very sad.  Instead of breaking the safety features
// for all of stan, here we will just bolt on default constructors in this file
// only.  This is still pretty safe, because X3 won't allow invalid values to
// be parsed (that is it's purpose).

template <>
const auto default_value<stan::pitch> = stan::pitch{ stan::pitchclass::c, stan::octave{ 4 } };

template <>
const auto default_value<stan::value> = stan::value::quarter();

template <>
const auto default_value<stan::rest> = stan::rest{ default_value<stan::value> };

template <>
const auto default_value<stan::note> = stan::note{ default_value<stan::value>, default_value<stan::pitch> };

template <>
const auto default_value<stan::chord> = stan::chord{ default_value<stan::value>, std::vector<stan::pitch>{ default_value<stan::pitch> } };

template <>
const auto default_value<beam> = beam{
    note{ value::eighth(), default_value<pitch> },
    note{ value::eighth(), default_value<pitch> }
};

template <>
const auto default_value<stan::column> = stan::column{ default_value<stan::note> };

} // namespace stan

namespace stan::lilypond {

} // namespace stan::lilypond

namespace stan {

} // namespace stan

namespace boost::spirit::x3::traits {

// Spirit X3 still uses the antique boost::variant for modeling choice values,
// but the C++17 std::variant is used everywhere else in stan.  The template class
// transform_attribute<> is X3's specialization hook to convert otherwise
// incompatible attribute types, which in this case converts from
// boost::variant<> to std::variant<>.  This method works perfect for what I
// needed, but it did take a day's labor to figure this out.  If X3 ever gets
// updated to replace boost::variant<> with std::variant<> under the covers,
// this specialization may be removed.

template <typename... Ts>
struct transform_attribute<std::variant<Ts...>, boost::variant<Ts...>, x3::parser_id>
{
    using type = boost::variant<Ts...>;
    using exposed_type = std::variant<Ts...>;

    static type pre(const exposed_type &ev) { return std::move(type()); }

    static void post(exposed_type &ev, const type &bv)
    {
        std::cout << "post std::variant to boost " << std::endl;
        ev = boost::apply_visitor([](auto &&n) -> exposed_type { return std::move(n); }, bv);
    }
};

template <typename... Ts>
struct transform_attribute<stan::column, boost::variant<Ts...>, x3::parser_id>
{
    using type = boost::variant<Ts...>;
    using exposed_type = stan::column;

    static type pre(const exposed_type &ev) { return stan::default_value<stan::note>; }

    static void post(exposed_type &ev, const type &bv)
    {
        std::cout << "post boost to column " << std::endl;
        ev = stan::copy_variant()(bv);
    }
};

template <typename... Ts>
struct transform_attribute<boost::variant<Ts...>, stan::column, x3::parser_id>
{
    using exposed_type = boost::variant<Ts...>;
    using type = stan::column;

    static type pre(const exposed_type &ev) { return stan::default_value<stan::note>; }

    static void post(exposed_type &ev, const type &bv)
    {
        std::cout << "post column to boost " << std::endl;
        ev = stan::copy_variant()(bv);
    }
};

template <typename T1, typename T2>
struct transform_attribute<stan::default_ctor<T1>, stan::default_ctor<T2>, x3::parser_id>
{
    using type = stan::default_ctor<T2>;
    using exposed_type = stan::default_ctor<T1>;

    static type pre(const exposed_type &ev) { return stan::default_value<T2>; }

    static void post(exposed_type &ev, const type &bv)
    {
        // ev = stan::default_value<T1>;
        const T2 &t = bv;
        std::cout << "post ctor to ctor " << typeid(T2).name() << std::endl;
        stan::column t1 = stan::copy_variant()(t);
        ev = t1;
        // ev = T1{ stan::copy_variant()(bv) };
    }
};

template <typename T>
struct transform_attribute<T, stan::default_ctor<T>, x3::parser_id>
{
    using type = stan::default_ctor<T>;
    using exposed_type = T;

    static type pre(const exposed_type &ev) { return stan::default_value<T>; }

    static void post(exposed_type &ev, const type &bv)
    {
        // ev = stan::default_value<T1>;
        const T &t = bv;
        std::cout << "post T to ctor " << typeid(T).name() << " " << stan::driver::debug::write(t) << std::endl;
        // stan::column t1 = stan::copy_variant()(t);
        ev = t;
        // ev = T1{ stan::copy_variant()(bv) };
    }
};

} // namespace boost::spirit::x3::traits

// Metaprogram to compute a boost::variant<> from std::variant<>.
template <typename T>
struct std_variant_to_boost;

template <typename... Ts>
struct std_variant_to_boost<std::variant<Ts...>>
{
    using type = boost::variant<Ts...>;
};

namespace x3 = boost::spirit::x3;

namespace stan::lilypond {

struct pitchclass_ : x3::symbols<stan::pitchclass>
{
    pitchclass_()
    {
        using pc = stan::pitchclass;

        // clang-format off
        add
	    ("aff", pc::aff)("af", pc::af)("a", pc::a)("as", pc::as)("ass", pc::ass)
	    ("bff", pc::bff)("bf", pc::bf)("b", pc::b)("bs", pc::bs)("bss", pc::bss)
	    ("cff", pc::cff)("cf", pc::cf)("c", pc::c)("cs", pc::cs)("css", pc::css)
	    ("dff", pc::dff)("df", pc::df)("d", pc::d)("ds", pc::ds)("dss", pc::dss)
	    ("eff", pc::eff)("ef", pc::ef)("e", pc::e)("es", pc::es)("ess", pc::ess)
	    ("fff", pc::fff)("ff", pc::ff)("f", pc::f)("fs", pc::fs)("fss", pc::fss)
	    ("gff", pc::gff)("gf", pc::gf)("g", pc::g)("gs", pc::gs)("gss", pc::gss)
	;
        // clang-format on
    }
} pitchclass;

struct basevalue_ : x3::symbols<default_ctor<stan::value>>
{
    basevalue_()
    {
        // clang-format off
        add
	    ("1", stan::value::whole())
	    ("2", stan::value::half())
	    ("4", stan::value::quarter())
	    ("8", stan::value::eighth())
	    ("16", stan::value::sixteenth())
	    ("32", stan::value::thirtysecond())
	    ("64", stan::value::sixtyfourth())
	    ;
        // clang-format on
    }
} basevalue;

// struct clef_ : x3::symbols<stan::clef> {
//     clef_() {
//         add
//         ("bass", stan::clef::bass())
//         ("treble", stan::clef::treble())
//         ;
//     }
// } clef_library;

// struct mode_ : x3::symbols<std::string> {
//     mode_() {
//         add
//         ("\\major", "major")
//         ("\\minor", "minor")
//         ;
//     }
// } mode;

using boost::fusion::at_c;
using x3::_attr;
using x3::_val;
using x3::attr;
using x3::eps;
using x3::lit;
using x3::repeat;
using x3::string;
using x3::ushort_;
using x3::ascii::char_;

x3::rule<struct ppitch, default_ctor<stan::pitch>> ppitch = "pitch";
x3::rule<struct poctave, stan::octave> poctave = "octave";
x3::rule<struct pvalue, default_ctor<stan::value>> pvalue = "value";
x3::rule<struct prest, default_ctor<stan::rest>> prest = "rest";
x3::rule<struct pnote, default_ctor<stan::note>> pnote = "note";
x3::rule<struct pchord, default_ctor<stan::chord>> pchord = "chord";
x3::rule<struct pbeam, default_ctor<stan::beam>> pbeam = "beam";
x3::rule<struct ptuplet, default_ctor<stan::tuplet>> ptuplet = "tuplet";
// using variant2 = boost::variant<default_ctor<stan::rest>, stan::note, stan::chord, stan::beam>;
// x3::rule<struct pcolumn, variant2> column = "column";
x3::rule<struct pcolumn, default_ctor<stan::column>> column = "column";

// x3::rule<struct pmusic, std::shared_ptr<stan::column>> music = "music";
// x3::rule<struct music_list, stan::sequential> music_list = "music_list";
// x3::rule<struct key, stan::key> key = "key";
// x3::rule<struct meter, stan::meter> meter = "meter";
// x3::rule<struct clef, stan::clef> clef = "clef";

// Define semantic actions separately, because C++ reserves [[]] syntax.  The
// semantic actions count the number of octave ticks, and convert them to a
// small number that is actually stored in stan::octave.
auto raise_octave = [](auto &ctx) { _val(ctx) = stan::octave{ 4 + _attr(ctx).size() }; };
auto lower_octave = [](auto &ctx) { _val(ctx) = stan::octave{ 4 - _attr(ctx).size() }; };
auto default_octave = [](auto &ctx) { _val(ctx) = stan::octave{ 4 }; };
auto const poctave_def =
    repeat(1, /*(int)stan::octave::max()*/ 7 - 4)[char_(R"(')")][raise_octave] |
    repeat(1, 4 - /*(int)stan::octave::min()*/ 0)[char_(R"(,)")][lower_octave] |
    eps[default_octave];

// Use semantic actions to maintain a running value, for parses like "{ c4 d }".
struct value_tag
{
};
//auto store_running_value = [](auto& ctx) { x3::get<value_tag>(ctx).get() = _attr(ctx); };
// auto use_running_value = [](auto& ctx) { _attr(ctx) = x3::get<value_tag>(ctx); };
// auto const note_def = note %= pitch >>
//    (value/*[store_running_value]*/ | attr(stan::value::quarter())/*[use_running_value]*/);

template <typename T, int... ArgOrder>
struct construct
{
    template <typename Context>
    void operator()(Context &ctx)
    {
        x3::_val(ctx) = T{ at_c<ArgOrder>(x3::_attr(ctx))... };
    }
};

template <typename T>
struct construct<T>
{
    template <typename Context>
    void operator()(Context &ctx)
    {
        std::cout << "start construct<T>" << std::endl; // stan::driver::debug::write(T{ x3::_attr(ctx) }) << std::endl;
        x3::_val(ctx) = T{ x3::_attr(ctx) };
        std::cout << "stop construct<T>" << std::endl;
    }
};

template <>
struct construct<stan::column>
{
    template <typename Context>
    void operator()(Context &ctx)
    {
        std::cout << "start construct<column>" << std::endl;
        boost::variant<stan::rest, stan::note, stan::chord, stan::beam> v = x3::_attr(ctx);
        x3::_val(ctx) = stan::copy_variant()(v);
        std::cout << "stop construct<column>" << std::endl;
    }
};

auto const prest_def = x3::lit('r') >> pvalue[construct<stan::rest>()];
auto const pnote_def = (ppitch >> pvalue)[construct<stan::note, 1, 0>()];
auto const ppitch_def = (pitchclass >> poctave)[construct<stan::pitch, 0, 1>()];

auto add_dot = [](auto &ctx) { _val(ctx) = dot(_val(ctx)); };

auto const pvalue_def = basevalue[construct<stan::value>()] >> x3::repeat(0, 2)[lit('.')[add_dot]];
auto const pchord_def = ('<' >> +ppitch >> '>' >> pvalue)[construct<stan::chord, 1, 0>()];
auto const pbeam_def = '[' >> (+column)[construct<stan::beam>()] >> ']';
auto const column_def = (prest | pnote | pchord | pbeam)[construct<stan::column>()];

// auto make_shared = [](auto &ctx) { _val = std::make_shared<column>(std::move(_attr(ctx))); };
// auto const music_def = column[make_shared];
// auto const variant_def = note | chord_body | key | meter | clef ;
// auto const music_list_def = lit('{') >> +variant >> '}';

// BOOST_SPIRIT_DEFINE(pitch, octave, value, note, column) // , chord_body, variant, music_list);

BOOST_SPIRIT_DEFINE(ppitch)
BOOST_SPIRIT_DEFINE(poctave)
BOOST_SPIRIT_DEFINE(pvalue)
BOOST_SPIRIT_DEFINE(prest)
BOOST_SPIRIT_DEFINE(pnote)
BOOST_SPIRIT_DEFINE(pchord)
BOOST_SPIRIT_DEFINE(pbeam)
// BOOST_SPIRIT_DEFINE(ptuplet)
BOOST_SPIRIT_DEFINE(column)
// BOOST_SPIRIT_DEFINE(music)

// auto construct_key = [](auto& ctx) {
//     // _attr(ctx) = stan::key(stan::pitchclass::d, "minor");
//     _attr(ctx) = stan::key(boost::fusion::at_c<0>(_val(ctx)), boost::fusion::at_c<1>(_val(ctx)));
// };
// auto const key_def = key %= lit("\\key") >> (pitchclass >> mode)[construct_key];
// auto const meter_def = lit("\\time") >> ushort_ >> '/' >> basevalue;
// auto const clef_def = lit("\\clef") >> clef_library;
//
// BOOST_SPIRIT_DEFINE(key, meter, clef);

// Keep a master list of all defined rules.  This helps Hana metaprograms
// automatically associate requested attribute types with rules, and explicit
// template instantiations, reducing boilerplate.
//auto rules  = hana::make_tuple(
//       column
// pitch, octave, value, note
// chord_body, variant, music_list,
// key, meter, clef
// );

// template <typename Event>
stan::column
// std::shared_ptr<stan::column>
parse(const std::string &lily)
{
// Define a boolean predicate to detect if a parsing rule, such as
// parse::voice, returns at attribute of type Event (true) or something
// else (false).
#if 0
    auto has_Event_attribute = [](auto rule) {
        using Attr = typename decltype(rule)::attribute_type;
        return hana::equal(hana::type_c<Attr>, hana::type_c<Event>);
    };

    // Find the specific rule that will return an Event
    auto rule = hana::find_if(rules, has_Event_attribute);
    static_assert(rule != hana::nothing, "no parse rule defined for Event");

    stan::value run = stan::value::quarter();
    auto parse = x3::with<value_tag>(std::ref(run)) [rule.value()];
#endif
    // Now that the appropriate rule is discovered, use it to parse an Event
    // variant2 music = stan::default_value<stan::note>;
    stan::column music{ stan::default_value<stan::note> };
    auto iter = lily.begin();
    std::cout << "start parse" << std::endl;
    if (!x3::phrase_parse(iter, lily.end(), column, x3::space, music)) {
        throw std::runtime_error("parse error");
    }
    if (iter != lily.end()) {
        throw std::runtime_error("incomplete parse");
    }

    std::cout << "done parse" << std::endl;
    return stan::copy_variant()(music);
    // return music;
    // std::cout << "parsed " << driver::debug::write(ev) << std::endl;
    // return stan::column(std::move(ev));
    // return stan::column(boost::apply_visitor([](auto &&n) -> stan::variant { return std::move(n); }, ev));
    // return boost::apply_visitorstan::column(
}

#if 0
// template <typename Event>
stan::column parse(std::ifstream &is)
{
    std::string lily{
        (std::istreambuf_iterator<char>(is)),
        (std::istreambuf_iterator<char>())
    };
    return parse(lily);
}
#endif

#if 0
// Coax explicit instantiations of specialized read<> templates for every rule
static auto parse_instances = hana::transform(rules, [](auto rule) {
    using Event = typename decltype(rule)::attribute_type;
    // return static_cast<std::unique_ptr<Event> (*)(std::ifstream&)>(std::move(parse<Event>));
    return [](std::ifstream& is) -> Event {
        auto ev = parse<Event>(is);
        return ev;
        };
});
#endif

#if 0
template<>
stan::column parse<stan::column>(std::ifstream&);
#endif

// std::shared_ptr<stan::column>
stan::column reader::operator()(const std::string &lily)
{
    return parse(lily);
}

} // namespace stan::lilypond
