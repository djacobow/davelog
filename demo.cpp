#include <cstdint>
#include <cstdio>
#include <format>
#include <random>
#include <ranges>
#include <string>

#include "dave/error.h"
#include "dave/init.h"
#include "dave/levels.h"
#include "dave/log.h"
#include "dave/strutil.h"
#include "dave/styles.h"
#include "dave/valueor.hpp"
#include "some_c.h"

const double EVEN_PROB = 0.5;

const std::string ee_cummings(
"in Just-\n"
"spring          when the world is mud-\n"
"luscious the little\n"
"lame balloonman\n"
"\n"
"whistles          far          and wee\n"
"\n"
"and eddieandbill come\n"
"running from marbles and\n"
"piracies and it's\n"
"spring\n"
"\n"
"when the world is puddle-wonderful\n"
"\n"
"the queer\n"
"old balloonman whistles\n"
"far          and             wee\n"
"and bettyandisbel come dancing\n"
"\n"
"from hop-scotch and jump-rope and\n"
"\n"
"it's\n"
"spring\n"
"and\n"
"\n"
"         the\n"
"\n"
"                  goat-footed\n"
"\n"
"balloonMan          whistles\n"
"far\n"
"and\n"
"wee\n"
);


auto randomBoolean() -> bool {
  static std::default_random_engine generator(std::random_device{}());
  static std::bernoulli_distribution distribution(EVEN_PROB);
  return distribution(generator);
}

struct bloop_t {
    int32_t aaa;
    std::string bbb;
};

auto bloopify(int i) -> dave::err::ValueOr<bloop_t> {
    if (randomBoolean()) {
        return bloop_t{.aaa = i, .bbb = "everything is fine!" };
    }
    return E_(data_loss, "data was lost");
}

auto makeTrue() -> dave::err::ValueOr<bool> {
    if (randomBoolean()) {
        return true;
    }
    return E_(aborted, "we don't like false");
}

void demo_valueor() {
    for (auto i : std::ranges::views::iota(0,10)) {
        auto rv0 = bloopify(i);
        if (rv0.ok()) {
            L_(info, std::format("bloop_t: {}, {}", rv0.v().aaa, rv0.v().bbb));
        } else {
            L_(error, rv0.why());
        }
        auto rv1 = makeTrue();
        if (rv1.ok()) {
            L_(info, std::format("value was: {}", rv1.v()));
        } else {
            L_(error, rv1.why());
        }
    }
}

void demo_boxes() {
    // call box with array of strings
    L_(warning,
        dave::str::box(
            "ee_cummings",
            dave::str::split(ee_cummings, dave::str::newline)
        )
    );

    // call box with a line that is too long to fit; it will be cut
    L_(info,
        dave::str::box(
            "long line",
            std::vector<std::string>{"this is a very long line, in fact it is too long to fit into the allotted space and will be shortened"}
        )
    );

    L_(notice,
        dave::str::box(
            "wrapped line",
            dave::str::wrap(ee_cummings, 60)
        )
    );
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int {

    const dave::log::InitList_t initlist = {
        dave::log::InitInfo_t{
            "console",
            stdout,
            dave::log::LevelMask_c().AtOrAbove(dave::log::Level_e::debug),
            dave::log::Style_e::tight
        },
        dave::log::InitInfo_t{
            "logfile",
            fopen("demo_output.json", "ae"),
            dave::log::LevelMask_c().All(),
            dave::log::Style_e::json
        },
    };

    dave::log::Init(initlist);

    L_(info, "This is the logger talking!");
    L_(info, "Nothing special here, kinda boring.");
    L_(debug, "but maybe not as boring as this?");
    L_(notice,"This is\na multiline\nstring\nsplit\nup");
    L_(warning, "The number is %d, that is %s", 42, "great!");
    L_(error, "%d is a bad return value", -1);
    L_(notice, dave::str::box("boop", std::vector<std::string>{"thing"}));

    foobs(); 
    demo_valueor();
    demo_boxes();
    L_(vverbose, "yadda yadda yadda");
    L_(debug, "this is a debug message");
    LOGGER << "Well this is another way to do it" << L_ENDL(debug);
    LOGGER << "Die! Die! Die!" << L_ENDL(fatal);
    L_(info, "and now we're leaving, bye!");

    return 0;
}

