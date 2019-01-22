/*
 *  Project: ax25c - File: DomTreeErrorReporter.h
 *  Copyright (C) 2019 - Tania Hagn - tania@df9ry.de
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_DOMTREEERRORREPORTER_HPP_
#define CONFIG_DOMTREEERRORREPORTER_HPP_

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

namespace XMLIO {

class DOMTreeErrorReporter: public XERCES_CPP_NAMESPACE::ErrorHandler {
public:
	DOMTreeErrorReporter() {}
	~DOMTreeErrorReporter() {}

    // -----------------------------------------------------------------------
    //  Implementation of the error handler interface
    // -----------------------------------------------------------------------
    void warning(const XERCES_CPP_NAMESPACE::SAXParseException& toCatch);
    void error(const XERCES_CPP_NAMESPACE::SAXParseException& toCatch);
    void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& toCatch);
    void resetErrors();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawErrors() const;

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSawErrors
    //      This is set if we get any errors, and is queryable via a getter
    //      method. Its used by the main code to suppress output if there are
    //      errors.
    // -----------------------------------------------------------------------
    bool    fSawErrors{false};

};

inline bool DOMTreeErrorReporter::getSawErrors() const
{
    return fSawErrors;
}

} /* namespace XMLIO */

#endif /* CONFIG_DOMTREEERRORREPORTER_HPP_ */
