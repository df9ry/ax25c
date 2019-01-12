/*
 *  Project: ax25c - File: ax25c_config.cpp
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

#include "ax25c_config.h"
#include "../runtime.h"
#include "DOMTreeErrorReporter.hpp"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <string>
#include <exception>
#include <iostream>

#define MODULE_NAME "ax25c_config"

using namespace std;
using namespace XMLIO;
using namespace XERCES_CPP_NAMESPACE;

static inline const XMLCh* toX(const char *cs) {
	return XMLString::transcode(cs);
}

static inline char *fmX(const XMLCh* x) {
	string s(XMLString::transcode(x));
	size_t l = s.length();
	char *buf = (char*)malloc(l+1);
	strcpy(buf, s.c_str());
	return buf;
}

static int compare(const void *o1, const void *o2)
{
	assert(o1);
	assert(o2);
	return strcmp((const char *)o1, (const char *)o2);
}

static void readInstance(DOMElement* element, struct instance *inst)
{
	assert(element);
    auto name = element->getAttribute(toX("name"));
    assert(name);
    inst->name = fmX(name);
}

static void readPlugin(DOMElement* element, struct plugin *plug)
{
	struct ::exception ex;

	assert(element);
    auto name = element->getAttribute(toX("name"));
    assert(name);
    plug->name = fmX(name);
    auto file = element->getAttribute(toX("file"));
    assert(file);
    plug->file = fmX(file);
    if (!load_so(plug->file, &plug->module_handle, &ex)) {
    	print_ex(&ex);
    	throw runtime_error(ex.message);
    }
    mapc_init(&plug->instances, compare);
	auto nodeList = element->getElementsByTagName(toX("Instances"));
	assert(nodeList);
	if (nodeList->getLength() > 0) {
		element = static_cast<DOMElement*>(nodeList->item(0));
		assert(element);
		nodeList = element->getElementsByTagName(toX("Instance"));
		for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
			auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
			assert(instanceNode);
			struct instance *inst = (struct instance*)malloc(sizeof(struct instance));
			assert(inst);
			readInstance(instanceNode, inst);
		    assert(inst->name);
		    mapc_insert(&plug->instances, &inst->node, inst->name);
		} // end for //
	}
}

static void readConfig(DOMElement* element, struct configuration *conf)
{
	assert(element);
    auto name = element->getAttribute(toX("name"));
    assert(name);
    conf->name = fmX(name);
    mapc_init(&conf->plugins, compare);
	auto nodeList = element->getElementsByTagName(toX("Plugins"));
	assert(nodeList);
	if (nodeList->getLength() > 0) {
		element = static_cast<DOMElement*>(nodeList->item(0));
		assert(element);
		nodeList = element->getElementsByTagName(toX("Plugin"));
		for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
			auto pluginNode = static_cast<DOMElement*>(nodeList->item(i));
			assert(pluginNode);
			struct plugin *plugin = (struct plugin*)malloc(sizeof(struct plugin));
			assert(plugin);
		    readPlugin(pluginNode, plugin);
		    assert(plugin->name);
		    mapc_insert(&conf->plugins, &plugin->node, plugin->name);
		} // end for //
	}
}

#if 0
static void readInstances(const string& id, DOMElement* element,
		FreeAX25::Runtime::UniquePointerDict<FreeAX25::Runtime::Instance>& instances)
{
	if (element == nullptr) return;
	auto nodeList = element->getElementsByTagName(toX("Instance"));
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
		string name  = fmX(instanceNode->getAttribute(toX("name")));
		FreeAX25::Runtime::env().logDebug(
				"Define instance " + id + "/" + name);
		auto instance = new FreeAX25::Runtime::Instance(name);

	    { // Get client endpoints:
			auto nodeList = instanceNode->getElementsByTagName(toX("ClientEndPoint"));
			for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
				auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
				string name = fmX(instanceNode->getAttribute(toX("name")));
				string url  = fmX(instanceNode->getAttribute(toX("url")));
				FreeAX25::Runtime::env().logDebug(
						"Define client endpoint " + id + "/" + name + " as " + url);
				auto endpoint = new FreeAX25::Runtime::ClientEndPoint(name, url);
				instance->clientEndPoints.insertNew(name, endpoint);
			} // end for //
	    }

	    { // Get server endpoints:
			auto nodeList = instanceNode->getElementsByTagName(toX("ServerEndPoint"));
			for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
				auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
				string name = fmX(instanceNode->getAttribute(toX("name")));
				string url  = fmX(instanceNode->getAttribute(toX("url")));
				FreeAX25::Runtime::env().logDebug(
						"Define server endpoint " + id + "/" + name + " as " + url);
				auto endpoint = new FreeAX25::Runtime::ServerEndPoint(name, url);
				instance->serverEndPoints.insertNew(name, endpoint);
			} // end for //
	    }

	    { // Get settings:
			auto nodeList = instanceNode->getElementsByTagName(toX("Settings"));
			if (nodeList->getLength() > 0)
				readSettings(
						id + "/" + name,
						static_cast<DOMElement*>(nodeList->item(0)),
						instance->settings);
	    }

		instances.insertNew(name, instance);
	} // end for //
}
#endif

extern "C" bool configure(
		int argc, char *argv[],
		struct ::configuration *conf,
		struct ::exception *ex)
{

	// Get the XML file path. This is either specified as an argument
	// argv[1] or can be deduced by argv[0]:
	string xmlpath{};
	if (argc > 1) {
		xmlpath.append(argv[1]);
	} else {
		xmlpath.append(argv[0]);
		unsigned int pos = xmlpath.find_last_of('/');
		if (pos != string::npos) xmlpath.erase(0, pos + 1);
		xmlpath.append(".xml");
	}
	// Load configuration:
	try {
		XMLPlatformUtils::Initialize();

	    XercesDOMParser parser;
	    parser.setValidationScheme(XercesDOMParser::Val_Always);
	    parser.setDoNamespaces(true);
	    parser.setDoSchema(true);
	    parser.setDoXInclude(true);
	    parser.setHandleMultipleImports(true);
	    parser.setValidationSchemaFullChecking(true);
	    parser.setCreateEntityReferenceNodes(false);
	    parser.setIncludeIgnorableWhitespace(false);

	    DOMTreeErrorReporter errReporter;
	    parser.setErrorHandler(&errReporter);

	    parser.parse(xmlpath.c_str());

	    if (errReporter.getSawErrors())
	    	throw std::exception();

	    // Now read configuration from the DOM Tree:
	    XERCES_CPP_NAMESPACE::DOMDocument* doc = parser.getDocument();
	    assert(doc != nullptr);

	    auto element = doc->getDocumentElement();
	    readConfig(element, conf);
	}
	catch (const std::exception& _ex) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = _ex.what();
			ex->param = "";
			ex->module = MODULE_NAME;
			ex->function = "configure";
		}
		return false;
	}
	catch (...) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = "Unknown error";
			ex->param = "";
			ex->module = MODULE_NAME;
			ex->function = "configure";
		}
		return false;
	}
	XMLPlatformUtils::Terminate();
	return true;
}
