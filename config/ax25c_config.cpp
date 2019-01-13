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
#include "../configuration.h"
#include "DOMTreeErrorReporter.hpp"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <string>
#include <exception>
#include <iostream>

#include <stddef.h>

#define MODULE_NAME "ax25c_config"

using namespace std;
using namespace XMLIO;
using namespace XERCES_CPP_NAMESPACE;

#define BUF_SIZE 38

static char buf[BUF_SIZE];

const char *strbuf(const char *s) {
	if (!s)
		return "NULL";
	size_t l = strlen(s);
	if (l < BUF_SIZE-2) {
		strcpy(buf, s);
	} else {
		strncpy(buf, s, BUF_SIZE-5);
		strcat(buf, "...");
	}
	return buf;
}

static inline const XMLCh* toX(const char *cs) {
	return XMLString::transcode(cs);
}

static inline string fmX1(const XMLCh* x) {
	string s(XMLString::transcode(x));
	return s;
}

static inline char *fmX2(const XMLCh* x) {
	string s(fmX1(x));
	return strdup(s.c_str());
}

#if 0
static int compare(const void *o1, const void *o2)
{
	assert(o1);
	assert(o2);
	return strcmp((const char *)o1, (const char *)o2);
}
#else
#define compare (f_compare)strcmp
#endif

static struct setting_descriptor settings_descriptor[] = {
		{ "tick",     UINT_T,  offsetof(struct configuration, tick),     "10"   },
		{ "loglevel", DEBUG_T, offsetof(struct configuration, loglevel), "NONE" },
		{ NULL }
};

static DOMElement *findElement(DOMNodeList *nodeList, const char *name)
{
	if (!(nodeList && (nodeList->getLength() > 0)))
			return NULL;
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
		string _name  = fmX1(instanceNode->getAttribute(toX("name")));
		if (strcmp(_name.c_str(), name) == 0) {
			return instanceNode;
		}
	} // end for //
	return NULL;
}

static bool decodeInt(const char *s, int *val, struct ::exception *ex)
{
	assert(s);
	assert(val);
	assert(ex);
	if (sscanf(s, "%i", val) != 1) {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "decodeInt";
		ex->message = "Invalid integer value";
		ex->param = strbuf(s);
		return false;
	}
	return true;
}

static bool getInt(DOMNodeList *nodeList, const char *name,
		int *val, const char *def, struct ::exception *ex)
{
	assert(nodeList);
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		string s(fmX1(element->getTextContent()));
		return decodeInt(s.c_str(), val, ex);
	}
	if (def) {
		return decodeInt(def, val, ex);
	}
	ex->erc = EXIT_FAILURE;
	ex->module = MODULE_NAME;
	ex->function = "getInt";
	ex->message = "Missing mandatory setting";
	ex->param = name;
	return false;
}

static bool decodeUInt(const char *s, unsigned int *val, struct ::exception *ex)
{
	assert(s);
	assert(val);
	assert(ex);
	if (sscanf(s, "%u", val) != 1) {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "decodeUInt";
		ex->message = "Invalid unsigned value";
		ex->param = strbuf(s);
		return false;
	}
	return true;
}

static bool getUInt(DOMNodeList *nodeList, const char *name,
		unsigned int *val, const char *def, struct ::exception *ex)
{
	assert(nodeList);
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		string s(fmX1(element->getTextContent()));
		return decodeUInt(s.c_str(), val, ex);
	}
	if (def) {
		return decodeUInt(def, val, ex);
	}
	ex->erc = EXIT_FAILURE;
	ex->module = MODULE_NAME;
	ex->function = "getUInt";
	ex->message = "Missing mandatory setting";
	ex->param = name;
	return false;
}

static bool decodeSize(const char *s, size_t *val, struct ::exception *ex)
{
	assert(s);
	assert(val);
	assert(ex);
	if (sscanf(s, "%lu", val) != 1) {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "decodeSize";
		ex->message = "Invalid size value";
		ex->param = strbuf(s);
		return false;
	}
	return true;
}

static size_t getSize(DOMNodeList *nodeList, const char *name,
		size_t *val, const char *def, struct ::exception *ex)
{
	assert(nodeList);
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		string s(fmX1(element->getTextContent()));
		return decodeSize(s.c_str(), val, ex);
	}
	if (def) {
		return decodeSize(def, val, ex);
	}
	ex->erc = EXIT_FAILURE;
	ex->module = MODULE_NAME;
	ex->function = "getSize";
	ex->message = "Missing mandatory setting";
	ex->param = name;
	return false;
}

static bool getCString(DOMNodeList *nodeList, const char *name,
		const char **val, const char *def, struct ::exception *ex)
{
	assert(nodeList);
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		*val = fmX2(element->getTextContent());
		return true;
	}
	if (def) {
		*val = strdup(def);
		return true;
	}
	ex->erc = EXIT_FAILURE;
	ex->module = MODULE_NAME;
	ex->function = "getCString";
	ex->message = "Missing mandatory setting";
	ex->param = name;
	return false;
}

static bool getDebugLevel(DOMNodeList *nodeList, const char *name,
		debug_level_t *val, const char *def, struct ::exception *ex)
{
	assert(nodeList);
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	string _val = "";
	if (element) {
		_val = fmX1(element->getTextContent());
	} else if (def) {
		_val = def;
	} else {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "getDebugLevel";
		ex->message = "Missing mandatory setting";
		ex->param = name;
		return false;
	}
	if (_val == "NONE") {
		*val = DEBUG_LEVEL_NONE;
	} else if (_val == "ERROR") {
		*val = DEBUG_LEVEL_ERROR;
	} else if (_val == "WARNING") {
		*val = DEBUG_LEVEL_WARNING;
	} else if (_val == "INFO") {
		*val = DEBUG_LEVEL_INFO;
	}else if (_val == "DEBUG") {
		*val = DEBUG_LEVEL_DEBUG;
	} else {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "getDebugLevel";
		ex->message = "Invalid debug level";
		ex->param = strbuf(_val.c_str());
		return false;
	}
	return true;
}

static bool configurator(void *handle, struct setting_descriptor *descriptor,
		void *context, struct ::exception *ex)
{
	DOMElement *element = (DOMElement*)context;
	assert(handle);
	assert(element);
	assert(ex);
	DOMNodeList *nodeList = element->getElementsByTagName(toX("Settings"));
	if (nodeList) {
		if (nodeList->getLength() > 0) {
			auto node = static_cast<DOMElement*>(nodeList->item(0));
			nodeList = node->getElementsByTagName(toX("Setting"));
		} else {
			nodeList = NULL;
		}
	}

	while (descriptor && descriptor->name) {
		const char *name = descriptor->name;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
		void *ptr = handle + descriptor->offset;
#pragma GCC diagnostic pop
		const char *def = descriptor->value;
		// Set value according to type:
		try {
			switch (descriptor->type) {
			case INT_T:
				if (!getInt(nodeList, name, (int*)ptr, def, ex))
					return false;
				break;
			case UINT_T:
				if (!getUInt(nodeList, name, (unsigned int*)ptr, def, ex))
					return false;
				break;
			case SIZE_T:
				if (!getSize(nodeList, name, (size_t*)ptr, def, ex))
					return false;
				break;
			case CSTR_T:
				if (!getCString(nodeList, name, (const char **)ptr, def, ex))
						return false;
				break;
			case DEBUG_T:
				if (!getDebugLevel(nodeList, name, (debug_level_t*)ptr, def, ex))
						return false;
				break;
			default:
				ex->erc = EXIT_FAILURE;
				ex->module = MODULE_NAME;
				ex->function = "configurator";
				ex->message = "Invalid data type";
				ex->param = "";
				return false;
			} // end switch //
		}
		catch (std::exception& _ex) {
			ex->erc = EXIT_FAILURE;
			ex->module = MODULE_NAME;
			ex->function = "configurator";
			ex->message = _ex.what();
			ex->param = "";
			return false;
		}
		++descriptor;
	} // end while //
	return true;
}

static bool readInstance(struct plugin *plug, DOMElement* element,
		struct instance *inst, struct ::exception *ex)
{
	assert(plug);
	assert(element);
	assert(inst);
	assert(ex);
	// Get instance name:
    auto name = element->getAttribute(toX("name"));
    assert(name);
    inst->name = fmX2(name);
    INFO("CNFIG INST", inst->name);
    // Get instance handle:
    assert(plug->plugin_descriptor);
    assert(plug->plugin_descriptor->get_instance_handle);
    inst->handle = plug->plugin_descriptor->get_instance_handle(
    		inst->name, configurator, element, ex);
    if (!inst->handle)
		return false;
    return true;
}

static bool readPlugin(DOMElement* element, struct plugin *plug,
		struct ::exception *ex)
{
	assert(element);
	assert(ex);
	// Get plugin name:
    auto name = element->getAttribute(toX("name"));
    assert(name);
    plug->name = fmX2(name);
    INFO("CNFIG PLUG", plug->name);
    // Get plugin file:
    auto file = element->getAttribute(toX("file"));
    assert(file);
    plug->file = fmX2(file);
    // Load the shared object:
    if (!load_so(plug->file, &plug->module_handle, ex))
    	return false;
    assert(plug->module_handle);
    // Get the descriptor out of the loaded shared object:
    if (!getsym_so(plug->module_handle, "plugin_descriptor",
    		(void**)&plug->plugin_descriptor, ex))
    	return false;
    assert(plug->plugin_descriptor);
    // Get the plugin handle:
    assert(plug->plugin_descriptor->get_plugin_handle);
	plug->handle = plug->plugin_descriptor->get_plugin_handle(
			plug->name, configurator, element, ex);
	if (!plug->handle)
		return false;
    // Initialize the instance map:
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
			if (!readInstance(plug, instanceNode, inst, ex))
				return false;
		    assert(inst->name);
		    mapc_insert(&plug->instances, &inst->node, inst->name);
		} // end for //
	}
	return true;
}

static bool readConfig(DOMElement* element, struct configuration *conf,
		struct ::exception *ex)
{
	assert(element);
	assert(ex);
	// Get config name:
    auto name = element->getAttribute(toX("name"));
    assert(name);
    conf->name = fmX2(name);
    // Configure the configuration:
    if (!configurator(conf, settings_descriptor, element, ex))
    	return false;
    INFO("CNFIG MAIN", conf->name);
    // Init the plugin map:
    mapc_init(&conf->plugins, compare);
    // Read plugins:
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
		    if (!readPlugin(pluginNode, plugin, ex))
		    	return false;
		    assert(plugin->name);
		    mapc_insert(&conf->plugins, &plugin->node, plugin->name);
		} // end for //
	}
	return true;
}

extern "C" bool configure(int argc, char *argv[], struct configuration *conf,
		struct ::exception *ex)
{
	bool result = true;

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
	    result = readConfig(element, conf, ex);
	}
	catch (const std::exception& _ex) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = _ex.what();
			ex->param = "";
			ex->module = MODULE_NAME;
			ex->function = "configure";
		}
		result = false;
	}
	catch (...) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = "Unknown error";
			ex->param = "";
			ex->module = MODULE_NAME;
			ex->function = "configure";
		}
		result = false;
	}
	XMLPlatformUtils::Terminate();
	return result;
}
