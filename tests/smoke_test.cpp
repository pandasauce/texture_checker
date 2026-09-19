#include <catch2/catch_test_macros.hpp>

#include "texinfo/texinfo.hpp"

TEST_CASE("probing an unreadable file reports a ProbeError") {
    REQUIRE_THROWS_AS(texinfo::probe("no_such_file.dds"), texinfo::ProbeError);
}
