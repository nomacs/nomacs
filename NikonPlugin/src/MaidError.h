#pragma once

#include <stdexcept>

namespace Maid {

class MaidError : public std::logic_error {
public:
	explicit MaidError(const std::string& message = "Unknown MAID error") 
		: std::logic_error(message) {}
};

class InitError : public MaidError {
public:
	explicit InitError(const std::string& message = "Initialization error")
		: MaidError(message) {}
};

class OpenCloseObjectError : public MaidError {
public:
	explicit OpenCloseObjectError(const std::string& message = "Error opening or closing MAID Object")
		 : MaidError(message) {}
};

}