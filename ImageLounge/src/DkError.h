/*******************************************************************************************************
 DkError.h
 Created on:	05.02.2010
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

#include <exception>
#include <string>
#include <DkUtils.h>


namespace nmc {

/**
 * The base exception class for the error management.
 **/
class DkException : public std::exception {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkException(const std::string & msg = "");
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkException(const std::string & msg, int line, const std::string & file);
	/**
	 * Default destructor.
	**/
	virtual ~DkException() throw();
	/**
	 * Returns the error message.
	 * @return The error message.
	**/
	virtual const char *what() const throw () ;
	/**
	 * Returns the error message.
	 * @return The error message.
	**/
	virtual const std::string Msg() const;
	/**
	 * Returns the line number of the error.
	 * @return The line number of the error.
	**/
	int Line() const;
	/**
	 * Returns the filename of the error.
	 * @return The filename of the error occurrence.
	**/
	const std::string& File() const;

protected:
	std::string exceptionName;				/**< Name of the exception **/
	std::string errMsg;						/**< Error message **/
	std::string errFile;					/**< Error filename **/
	int errLine;							/**< line number **/

};

/**
 * Exception class for illegal arguments.
 **/
class DkIllegalArgumentException : public DkException {
	
public:
	//DkIllegalArgumentException();
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkIllegalArgumentException(const std::string & msg = "") : DkException(msg) {exceptionName = "DkIllegalArgumentException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkIllegalArgumentException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkIllegalArgumentException";};
};

/**
 * Exception class if cast errors occur.
 **/
class DkCastException : public DkException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkCastException(const std::string & msg = "")  : DkException(msg) {exceptionName = "DkCastException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkCastException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkCastException";};
};

/**
 * Exception class for OpenCV Mats.
 **/
class DkMatException : public DkException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkMatException(const std::string & msg = "")  : DkException(msg) {exceptionName = "DkMatException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkMatException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkMatException";};
};
/**
 * Exception class for files.
 **/
class DkFileException : public DkException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkFileException(const std::string & msg = "")  : DkException(msg) {exceptionName = "DkFileException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkFileException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkFileException";};
};
/**
 * Exception class if specified files are not found.
 **/
class DkFileNotFoundException : public DkFileException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkFileNotFoundException(const std::string & msg = "")  : DkFileException(msg) {exceptionName = "DkFileNotFoundException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkFileNotFoundException(const std::string & msg, int line, const std::string & file) : DkFileException(msg, line, file) {exceptionName = "DkFileNotFoundException";};
};
/**
 * Exception class if specified files are empty.
 **/
class DkFileEmptyException : public DkFileNotFoundException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkFileEmptyException(const std::string & msg = "")  : DkFileNotFoundException(msg) {exceptionName = "DkFileEmptyException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkFileEmptyException(const std::string & msg, int line, const std::string & file) : DkFileNotFoundException(msg, line, file) {exceptionName = "DkFileEmptyException";};
};
/**
 * Exception class for XML.
 **/
class DkXMLException : public DkException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkXMLException(const std::string & msg = "")  : DkException(msg) {exceptionName = "DkXMLException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkXMLException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkXMLException";};
};

/**
 * Exception class if indicess are out of bounds.
 **/
class DkIndexOutOfBoundsException : public DkException {

public:
	/**
	 * Default constructor.
	 * @param msg The error message
	**/
	DkIndexOutOfBoundsException(const std::string & msg = "")  : DkException(msg) {exceptionName = "DkIndexOutOfBoundsException";};
	/**
	 * Default constructor.
	 * @param msg The error message.
	 * @param line The line # of the error.
	 * @param file The filename of the error occurrence.
	**/
	DkIndexOutOfBoundsException(const std::string & msg, int line, const std::string & file) : DkException(msg, line, file) {exceptionName = "DkIndexOutOfBoundsException";};
};
};