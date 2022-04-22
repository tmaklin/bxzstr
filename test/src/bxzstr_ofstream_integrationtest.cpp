/* This Source Code Form is subject to the terms of the Mozilla Public
g * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "bxzstr_ofstream_integrationtest.hpp"

TEST_F(ZCompressionTest, BxzOfstreamCompressesZ) {
    this->run_test();
}

TEST_F(BzCompressionTest, BxzIfstreamCompressesBz) {
    this->run_test();
}

TEST_F(LzmaCompressionTest, BxzIfstreamCompressesLzma) {
    this->run_test();
}

TEST_F(ZstdCompressionTest, BxzIfstreamCompressesZstd) {
    this->run_test();
}

