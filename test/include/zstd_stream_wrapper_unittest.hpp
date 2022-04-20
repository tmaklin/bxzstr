/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_ZSTD_STREAM_WRAPPER_UNITTEST_HPP
#define BXZSTR_ZSTD_STREAM_WRAPPER_UNITTEST_HPP

#include <string>

#include "gtest/gtest.h"
#include "zstd.h"

#include "bxzstr.hpp"

// Test zstdException
class ZstdExceptionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->msgConstructorValue = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->msgConstructorExpected = "urpdcjgztzcowdpiucfrhxczlgbbopeg";
	this->errcodeConstructorValue = -1;
	this->errcodeConstructorExpected = "zstd error: [18446744073709551615]: Error (generic)";
    }
    void TearDown() override {
    }
    // Test values
    std::string msgConstructorValue;
    size_t errcodeConstructorValue;
    // Expecteds
    std::string msgConstructorExpected;
    std::string errcodeConstructorExpected;
};

// Test zstd_stream_wrapper
class ZstdStreamWrapperTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->testTrue = true;
	this->testFalse = false;
    }
    void TearDown() override {
    }
    // Test values
    bool testTrue;
    bool testFalse;
};

// Test decompress
class DecompressTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<const unsigned char*>("abcd");
	this->testOut = reinterpret_cast<const unsigned char*>("efgh");
	wrapper = new bxz::detail::zstd_stream_wrapper();
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(4);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(4);
    }
    void TearDown() override {
    }
    // Test values
    const unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::zstd_stream_wrapper* wrapper;
};

// Test compress
class CompressTest : public ::testing::Test {
  protected:
    void SetUp() override {
	this->testIn = reinterpret_cast<const unsigned char*>("abcd");
	this->testOut = reinterpret_cast<const unsigned char*>("efgh");
	wrapper = new bxz::detail::zstd_stream_wrapper(false);
	wrapper->set_next_in(&testIn[0]);
	wrapper->set_avail_in(4);
	wrapper->set_next_out(&testOut[0]);
	wrapper->set_avail_out(4);
    }
    void TearDown() override {
    }
    // Test values
    const unsigned char* testIn;
    const unsigned char* testOut;
    bxz::detail::zstd_stream_wrapper* wrapper;
};

#endif
