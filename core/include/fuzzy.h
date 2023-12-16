#ifndef HAK_FUZZY_H
#define HAK_FUZZY_H

#include "types.h"
#include "process.h"
#include "searcher_parser.h"
#include "searcher.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace hak {
    class memory_fuzzy: memory_searcher {
    protected:
        value_type type = value_type::type_i32;

    public:
        explicit memory_fuzzy(std::shared_ptr<hak::process> proc);

        void set_memory_range(i32 _range) override;

        void load_memory_data();
    };
}

#endif //HAK_FUZZY_H
