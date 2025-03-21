#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <random>
#include <ranges>
#include <string>

#include "dave/inc/async/tpool.h"
#include "dave/inc/err/error.h"
#include "dave/inc/err/valueor.hpp"
#include "dave/inc/event/event.h"
#include "dave/inc/log/init.h"
#include "dave/inc/log/levels.h"
#include "dave/inc/log/log.h"
#include "dave/inc/log/styles.h"
#include "dave/inc/util/hexprint.h"
#include "dave/inc/util/strutil.h"
#include "dave/inc/util/winsize.h"

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


static auto randomBoolean() -> bool {
  static std::default_random_engine generator(std::random_device{}());
  static std::bernoulli_distribution distribution(EVEN_PROB);
  return distribution(generator);
}

struct bloop_t {
    int32_t aaa;
    std::string bbb;
};

static auto bloopify(int i) -> dave::err::ValueOr_c<bloop_t> {
    if (randomBoolean()) {
        return bloop_t{.aaa = i, .bbb = "everything is fine!" };
    }
    return E_(data_loss, "data was lost");
}

static auto makeTrue() -> dave::err::ValueOr_c<bool> {
    if (randomBoolean()) {
        return true;
    }
    return E_(aborted, "we don't like false");
}

static void demo_valueor() {
    for (auto i : std::ranges::views::iota(0,10)) {
        auto rv0 = bloopify(i);
        if (rv0.Ok()) {
            LOGGER << "bloop_t: " << rv0.v().aaa << ", " << rv0.v().bbb << L_ENDL(info);
        } else {
            LOGGER << rv0.Why() << L_ENDL(error);
        }
        auto rv1 = makeTrue();
        if (rv1.Ok()) {
            LOGGER << "value was: " << (rv1.v() ? "true" : "false") << L_ENDL(info);
        } else {
            LOGGER << rv1.Why() << L_ENDL(error);
        }
    }
}

static void demo_boxes() {
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

class SillyExample_c {
    public:
        SillyExample_c() = default;
        void Silly(uint32_t i) {
            const std::lock_guard<std::mutex> lock(mtx_);
            order_.push_back(i);
        }

        std::string ShowOrder() {
            std::vector<std::string> order_s;
            std::ranges::transform(order_, std::back_inserter(order_s), [](auto v) { return std::to_string(v); });
            return dave::str::join( order_s, ",");
        };

    private:
        std::mutex mtx_;
        std::vector<uint32_t> order_;
};

static SillyExample_c se;

static void demo_tpool() {
    L_(info, "tpool demo starting");
    const uint32_t NUM_THREADS = 5;
    const uint32_t NUM_ITERS = 50;
    dave::async::ThreadPool_c p(NUM_THREADS);

    p.Start();
    for (uint32_t i=0;i<NUM_ITERS;i++) {
        p.Add([i]() { se.Silly(i); });
    }
    p.Stop(true);
    L_(info, "tpool demo complete");
    L_(info, "Completion order was: %s", se.ShowOrder().c_str());
}


static void sub_one([[maybe_unused]] const dave::event::EventID_t &id /*unused*/ , const std::string &evname) {
    L_(info, "sub one got event: %s", evname.c_str());
}
static void sub_two([[maybe_unused]] const dave::event::EventID_t &id /*unused*/ , const std::string &evname) {
    L_(info, "sub two got event: %s", evname.c_str());
}


static void demo_events() {
    dave::event::EventSystem_c evsys({"beep", "boop", "froop", "frop"});
    evsys.Subscribe([](const dave::event::EventID_t, const std::string &evname) {
        L_(info, "lambda got event: %s", evname.c_str());
    });
    evsys.Subscribe(dave::event::Subscriber_t(sub_one));
    evsys.Subscribe(dave::event::Subscriber_t(sub_two));
    evsys.Send("boop");
    evsys.Send("frop");
};


auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int {

    const dave::log::InitList_t initlist = {
        dave::log::InitInfo_t{
            .name = "console",
            .file_ptr =  stdout,
            .level_mask = dave::log::LevelMask_c().AtOrAbove(dave::log::Level_e::debug),
            .style = dave::log::Style_e::tight
        },
        dave::log::InitInfo_t{
            .name = "logfile",
            .file_ptr = fopen("demo_output.json", "ae"),
            .level_mask = dave::log::LevelMask_c().All(),
            .style = dave::log::Style_e::json
        },
    };

    dave::log::Init(initlist);

    auto ws = dave::util::GetTerminalDimensions();
    if (ws.Ok()) {
        L_(notice, "Terminal size: %u by %u", ws.v().width, ws.v().height);
    } else {
        L_(warning, "Could not get terminal size: %s", ws.Why().c_str());
    }
    L_(info, "This is the logger talking!");
    L_(info, "Nothing special here, kinda boring.");
    L_(debug, "but maybe not as boring as this?");
    L_(notice,"This is\na multiline\nstring\nsplit\nup");
    L_(warning, "The number is %d, that is %s", 42, "great!");
    L_(error, "%d is a bad return value", -1);
    L_(notice, dave::str::box("boop", std::vector<std::string>{"thing"}));
    L_(warning, "A string but in hex: %s", dave::hex::bin2hex("A string but in hex").c_str());
    L_(error, "A string but in hex, printed \"canonically\": %s", dave::hex::bin2hex_canonical("A string but in hex").c_str());


    foobs(); 
    demo_valueor();
    demo_boxes();
    demo_tpool();
    demo_events();
    L_(vverbose, "yadda yadda yadda");
    L_(debug, "this is a debug message");
    LOGGER << "Well this is another way to do it" << L_ENDL(debug);
    LOGGER << "Die! Die! Die!" << L_ENDL(fatal);
    L_(info, "and now we're leaving, bye!");
    return 0;
}

