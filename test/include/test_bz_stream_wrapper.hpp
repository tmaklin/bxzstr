/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_TEST_BZ_STREAM_WRAPPER_HPP
#define BXZSTR_TEST_BZ_STREAM_WRAPPER_HPP

#include <string>
#include <fstream>

#include "gtest/gtest.h"
#include "bzlib.h"

#include "bxzstr.hpp"

// Integration tests for bz_stream_wrapper
//
// Decompression
class BzDecompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake gzip data with 10 1s on their own lines
	const unsigned char test[] = { 0x42, 0x5a, 0x68, 0x39, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x35, 0xaa, 0x83, 0x68, 0x00, 0x00,
	                                0x09, 0xc8, 0x00, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x20, 0xa9, 0xa0, 0x82, 0x64, 0xce, 0x2e,
	                                0xe4, 0x8a, 0x70, 0xa1, 0x20, 0x6b, 0x55, 0x06, 0xd0 };

	this->testInFile = "BzDecompressionTest_fake_data.txt.bz2";
	std::ofstream of(testInFile);
	for (uint32_t i = 0; i < 41; ++i) {
	    of << test[i];
	}
	of.close();

	this->nInVals = 10;
	this->expected = std::vector<char>(nInVals, '1');
    }
    void TearDown() override {
    }
    // Test values
    std::string testInFile;
    uint32_t nInVals;
    std::vector<char> expected;
};

// Compression
class BzCompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::bz2 for this test set
	const unsigned char test[] = { 0x42, 0x5a, 0x68, 0x36, 0x31, 0x41, 0x59, 0x26, 0x53, 0x59, 0x95, 0x99, 0x0f, 0x45, 0x00, 0x00,
	                               0x04, 0xc8, 0x00, 0x00, 0x10, 0x20, 0x00, 0x20, 0x00, 0x20, 0xa9, 0xa0, 0x82, 0x65, 0x8e, 0x2e,
				       0xe4, 0x8a, 0x70, 0xa1, 0x21, 0x2b, 0x32, 0x1e, 0x8a, 0x42, 0x5a, 0x68, 0x36, 0x17, 0x72, 0x45,
				       0x38, 0x50, 0x90, 0x00, 0x00, 0x00, 0x00 };

	this->testOutFile = "BzCompressionTest_fake_data.txt.bz2";
	this->nOutVals = 10;
	this->expected = std::vector<char>(55);

	bxz::ofstream out(testOutFile, bxz::bz2);
	for (uint32_t i = 0; i < nOutVals; ++i) {
	    out << 1;
	    if (i < nOutVals - 1)
		out << '\n';
	}
	out.flush();

	for (uint32_t i = 0; i < 55; ++i) {
	    expected[i] = test[i];
	}
    }
    void TearDown() override {
    }
    // Test values
    std::string testOutFile;
    uint32_t nOutVals;
    std::vector<char> expected;
};

#endif
