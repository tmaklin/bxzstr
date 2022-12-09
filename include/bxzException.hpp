/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_BXZEXCEPTION_HPP
#define BXZSTR_BXZEXCEPTION_HPP

#include <exception>
#include <string>
#include <iostream>

#if !defined(__EXCEPTIONS) && __EXCEPTIONS == 0
#define bxzThrow new
#else
#define bxzThrow throw
#endif

namespace bxz {

/// Exception class thrown by failed operations.
class bxzException
#if defined(__EXCEPTIONS) && __EXCEPTIONS == 1
    : public std::exception
#endif
{
public:
    bxzException() = default;
    bxzException(const std::string& msg) : _msg(msg) {
	terminate_if_no_exceptions();
    }
    const char * what() const noexcept { return _msg.c_str(); }
protected:
    std::string _msg;

    void terminate_if_no_exceptions() {
#if !defined(__EXCEPTIONS) && __EXCEPTIONS == 0
	// Print msg and abort if an exception is created when the program is compiled with -fno-exceptions.
	std::cerr << "Error: " << _msg << std::endl;
	std::terminate();
#endif
    }
	
}; // class bxzException

}

#endif
