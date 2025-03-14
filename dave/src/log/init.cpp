#include <memory>
#include "dave/inc/log/init.h"
#include "dave/inc/log/logger.hpp"
#include "dave/inc/log/subscriber.h"

namespace dave::log {

void Init(const InitList_t &initlist) {
    for (const auto &iv : initlist) {
        Logger_c::Get().AddSubscriber(
            iv.name,
            std::make_unique<Subscriber_FilePtr_c>(
                iv.file_ptr,
                iv.level_mask,
                iv.style
            )
        );
    }
}

}  // namespace dave::log
