/* This Source Code Form is subject to the terms of the Mozilla Public
g * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr_ifstream_integrationtest.hpp"

TEST_F(ZDecompressionTest, BxzIfstreamDecompressesZ) {
    this->run_test();
}

TEST_F(BzDecompressionTest, BxzIfstreamDecompressesBz) {
    this->run_test();
}

TEST_F(LzmaDecompressionTest, BxzIfstreamDecompressesLzma) {
    this->run_test();
}

TEST_F(ZstdDecompressionTest, BxzIfstreamDecompressesZstd) {
    this->run_test();
}

