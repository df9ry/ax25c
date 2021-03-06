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

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <string>
#include <algorithm>
#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>

#include <stringc/stringc.h>

#include <stddef.h>
#include <unistd.h>

#include "../runtime/runtime.h"

#include "ax25c_config.h"
#include "configuration.h"
#include "DOMTreeErrorReporter.hpp"

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

static const std::string str_int(int i) {
	stringstream oss;
	oss << i;
	return oss.str();
}

static const std::string str_uint(unsigned int u) {
	stringstream oss;
	oss << u;
	return oss.str();
}

static const std::string str_size(size_t s) {
	stringstream oss;
	oss << s;
	return oss.str();
}

static const std::string str_debug_level(debug_level_t t) {
	switch (t) {
	case DEBUG_LEVEL_NONE:    return "NONE";
	case DEBUG_LEVEL_ERROR:   return "ERROR";
	case DEBUG_LEVEL_WARNING: return "WARNING";
	case DEBUG_LEVEL_INFO:    return "INFO";
	case DEBUG_LEVEL_DEBUG:   return "DEBUG";
	default: return "???" + str_int((int)t);
	}
}

static debug_level_t decodeDebugLevel(const char *s) {
	assert(s);
	if (strcmp(s, "NONE")    == 0) return DEBUG_LEVEL_NONE;
	if (strcmp(s, "ERROR")   == 0) return DEBUG_LEVEL_ERROR;
	if (strcmp(s, "WARNING") == 0) return DEBUG_LEVEL_WARNING;
	if (strcmp(s, "INFO")    == 0) return DEBUG_LEVEL_INFO;
	if (strcmp(s, "DEBUG")   == 0) return DEBUG_LEVEL_DEBUG;
	return (debug_level_t)-1;
}

static inline const XMLCh* toX(const char *cs) {
	return XMLString::transcode(cs);
}

static inline std::string fmX1(const XMLCh* x) {
	std::string s(XMLString::transcode(x));
	return s;
}

static inline char *fmX2(const XMLCh* x) {
	std::string s(fmX1(x));
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
		{ "tick",       UINT_T,  offsetof(struct configuration, tick),        "10" },
		{ "loglevel",   DEBUG_T, offsetof(struct configuration, loglevel),    "-"  },
		{ NULL }
};

static DOMElement *findElement(DOMNodeList *nodeList, const char *name)
{
	if (!(nodeList && (nodeList->getLength() > 0)))
			return NULL;
	for (uint32_t i = 0; i < nodeList->getLength(); ++i) {
		auto instanceNode = static_cast<DOMElement*>(nodeList->item(i));
		std::string _name  = fmX1(instanceNode->getAttribute(toX("name")));
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
	if (sscanf(s, "%i", val) != 1) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "decodeInt",
				"Invalid integer value", s);
		return false;
	}
	return true;
}

static bool getInt(DOMNodeList *nodeList, const char *name,
		int *val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		std::string s(fmX1(element->getTextContent()));
		return decodeInt(s.c_str(), val, ex);
	}
	if (def) {
		return decodeInt(def, val, ex);
	}
	exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getInt",
			"Missing mandatory setting", name);
	return false;
}

static bool decodeUInt(const char *s, unsigned int *val, struct ::exception *ex)
{
	assert(s);
	assert(val);
	if (sscanf(s, "%u", val) != 1) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "decodeUInt",
				"Invalid unsigned value", s);
		return false;
	}
	return true;
}

static bool getUInt(DOMNodeList *nodeList, const char *name,
		unsigned int *val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		std::string s(fmX1(element->getTextContent()));
		return decodeUInt(s.c_str(), val, ex);
	}
	if (def) {
		return decodeUInt(def, val, ex);
	}
	exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getUInt",
			"Missing mandatory setting", name);
	return false;
}

static bool decodeSize(const char *s, size_t *val, struct ::exception *ex)
{
	const char *fmt = (sizeof(int*) == 8) ? "%lu" : "%u";
	assert(s);
	assert(val);
	if (sscanf(s, fmt, val) != 1) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "decodeSize",
				"Invalid size value", s);
		return false;
	}
	return true;
}

static size_t getSize(DOMNodeList *nodeList, const char *name,
		size_t *val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		std::string s(fmX1(element->getTextContent()));
		return decodeSize(s.c_str(), val, ex);
	}
	if (def) {
		return decodeSize(def, val, ex);
	}
	exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getSize",
			"Missing mandatory setting", name);
	return false;
}

static bool getCString(DOMNodeList *nodeList, const char *name,
		const char **val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		*val = fmX2(element->getTextContent());
		return true;
	}
	if (def) {
		*val = strdup(def);
		return true;
	}
	exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getCString",
			"Missing mandatory setting", name);
	return false;
}

static bool getDebugLevel(DOMNodeList *nodeList, const char *name,
		debug_level_t *val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	DOMElement *element = findElement(nodeList, name);
	std::string _val = "";
	if (element) {
		_val = fmX1(element->getTextContent());
	} else if (def) {
		_val = def;
	} else {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getDebugLevel",
				"Missing mandatory setting", name);
		return false;
	}
	if (_val != "-") {
		debug_level_t debug_level = decodeDebugLevel(_val.c_str());
		if (debug_level < 0) {
			exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getDebugLevel",
					"Invalid debug level", _val.c_str());
			return false;
		}
		*val = debug_level;
	}
	return true;
}

static bool getString(DOMNodeList *nodeList, const char *name,
		struct ::string *val, const char *def, struct ::exception *ex)
{
	assert(name);
	assert(val);
	assert(ex);
	DOMElement *element = findElement(nodeList, name);
	if (element) {
		string_set_c(val, fmX2(element->getTextContent()));
		return true;
	}
	if (def) {
		string_set_c(val, def);
		return true;
	}
	exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "getCString",
			"Missing mandatory setting", name);
	return false;
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
				DBG_DEBUG(name, str_int(*(int*)ptr).c_str());
				break;
			case UINT_T:
				if (!getUInt(nodeList, name, (unsigned int*)ptr, def, ex))
					return false;
				DBG_DEBUG(name, str_uint(*(unsigned int*)ptr).c_str());
				break;
			case NSIZE_T:
				if (!getSize(nodeList, name, (size_t*)ptr, def, ex))
					return false;
				DBG_DEBUG(name, str_size(*(size_t*)ptr).c_str());
				break;
			case CSTR_T:
				if (!getCString(nodeList, name, (const char **)ptr, def, ex))
						return false;
				assert(ptr);
				DBG_DEBUG(name, *(const char**)ptr);
				break;
			case DEBUG_T:
				if (!getDebugLevel(nodeList, name, (debug_level_t*)ptr, def, ex))
						return false;
				DBG_DEBUG(name, str_debug_level(*(debug_level_t*)ptr).c_str());
				break;
			case STR_T:
				if (!getString(nodeList, name, (struct ::string*)ptr, def, ex))
					return false;
				DBG_DEBUG(name, ::string_c((struct ::string*)ptr));
				break;
			default:
				exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configurator",
						"Invalid data type", "");
				return false;
			} // end switch //
		}
		catch (std::exception& _ex) {
			exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configurator",
					_ex.what(), "");
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
    DBG_INFO("CNFIG INST", inst->name);
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
    DBG_INFO("CNFIG PLUG", plug->name);
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
    DBG_INFO("CNFIG MAIN", conf->name);
    if (!configurator(conf, settings_descriptor, element, ex))
    	return false;
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

#ifdef __MINGW32__
/*
 * Case Insensitive Implementation of endsWith()
 * It checks if the string 'mainStr' ends with given string 'toMatch'
 */
static bool endsWithCaseInsensitive(std::string mainStr, std::string toMatch)
{
	auto it = toMatch.begin();
	return mainStr.size() >= toMatch.size() &&
			std::all_of(std::next(mainStr.begin(),mainStr.size() - toMatch.size()), mainStr.end(), [&it](const char & c){
				return ::tolower(c) == ::tolower(*(it++))  ;
	} );
}
#endif

extern "C" bool configure(int argc, char *argv[], struct configuration *conf,
		struct ::exception *ex)
{
	bool result = true;

	// Get config parameters:
	while ((argc > 1) && (strncmp(argv[1], "--", 2) == 0)) {
		if (strncmp(argv[1], "--loglevel:", 11) == 0) {
			const char *s = &argv[1][11];
			configuration.loglevel = decodeDebugLevel(s);
			DBG_INFO("Set loglevel", s);
			if (configuration.loglevel < 0) {
				exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
						"Invalid debug level", "");
				return false;
			}
		} else if (strncmp(argv[1], "--pid:", 6) == 0) {
			const char *s = &argv[1][6];
			DBG_INFO("Set pid", s);
			int pid = getpid();
			try {
				ofstream stream;
				stream.open(s);
				stream << pid;
				stream.close();
			}
			catch(std::exception &_ex) {
				exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
						_ex.what(), "");
				return false;
			}
			catch(...) {
				exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
						"IO_Error", "");
				return false;
			}
		} else if (strncmp(argv[1], "--esc:", 6) == 0) {
			escapeChar = argv[1][6];
		} else if (strncmp(argv[1], "--noleads", 9) == 0) {
			writeLeads = false;
		} else {
			exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
					"Invalid argument", argv[1]);
			return false;
		}
		DBG_DEBUG("argv[1]=", argv[1]);
		--argc;
		memmove(&argv[1], &argv[2], argc * sizeof(char*));
	} // end while //

	// Get the XML file path. This is either specified as an argument
	// argv[1] or can be deduced by argv[0]:
	std::string xmlpath{};
	if (argc > 1) {
		xmlpath.append(argv[1]);
	} else {
		xmlpath.append(argv[0]);
		unsigned int pos = xmlpath.find_last_of('/');
		if (pos != std::string::npos) xmlpath.erase(0, pos + 1);
#ifdef __MINGW32__
		if (endsWithCaseInsensitive(xmlpath, ".exe"))
			xmlpath.erase(xmlpath.length()-4);
#endif
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
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
				_ex.what(), argv[1]);
		result = false;
	}
	catch (...) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "configure",
				"Unknown error", argv[1]);
		result = false;
	}
	XMLPlatformUtils::Terminate();
	return result;
}
