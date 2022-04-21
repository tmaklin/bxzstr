/* This Source Code Form is subject to the terms of the Mozilla Public
g * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "test_z_stream_wrapper.hpp"

#include "zlib.h"

#include "bxzstr.hpp"
#include <iostream>

TEST_F(ZDecompressionTest, BxzIfstreamDecompressesZ) {
    bxz::ifstream in(testInFile);
    std::string line;
    uint32_t i = 0;
    while (std::getline(in, line)) {
	EXPECT_EQ(line[0], expected[i]);
	++i;
    }
    EXPECT_EQ(i, nInVals);
}

TEST_F(ZCompressionTest, BxzOfstreamCompressesZ) {
    std::ifstream in(testOutFile);
    std::ostringstream oss;
    oss << in.rdbuf();
    std::string read_in = oss.str();
    uint32_t n_read_in = read_in.size();
    EXPECT_EQ(n_read_in, expected.size());
    for (uint32_t i = 0; i < n_read_in; ++i) {
	EXPECT_EQ(read_in[i], expected[i]);
    }
}
