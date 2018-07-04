/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <davix.hpp>
#include <copy/davixcopy.hpp>
#include <unistd.h>
#include <checksums/checksums.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <json/json.h>
#include "gfal_http_plugin.h"

using namespace Davix;

const char* gfal_http_check_classes(plugin_handle plugin_data, const char *url, const char *type, GError** err)
{
	if (type != NULL && (strcmp(type, "dataobject") == 0 || strcmp(type, "container") == 0 )) {
		GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
		DavixError* tmp_err=NULL;
		Context c;


		std::string uri(url);
		uri += "/cdmi_capabilities/";
		uri += type;
		HttpRequest r(c, uri, &tmp_err);
		Davix::RequestParams req_params;// = new RequestParams();
		davix->get_params(&req_params, Davix::Uri(url), false);

		//std::stringstream ss;
		//ss << "Bearer " << "eyJraWQiOiJyc2ExIiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJmZWE1ZTZlMi0wYjlmLTQwZjUtYjE5OC00YmI3YWU0YjIzNGEiLCJpc3MiOiJodHRwczpcL1wvaWFtLmV4dHJlbWUtZGF0YWNsb3VkLmV1XC8iLCJleHAiOjE1MzA2MDc0ODgsImlhdCI6MTUzMDYwMzg4OCwianRpIjoiMTgwNmFkOTktZTQzMi00MzQwLWFjZTctOTk3YzIxMjkzMDYxIn0.gL3pg3_0A7Qwkqg79Mp2lTO3HgSrCQkqHundHJEjS5LZsl52zmCFumXD6qa3EHL0v-BBJhe6rGaTvwCEBrDNTOjZl_uQ-kZj9TGGGzklCZ0qZsKnYarZnpuWYRpAh82MaQMq-Jk3gOGLnlY6_yeoH4by7kE-EVX8Jw4EmN6Xnt4";
		//req_params.addHeader("Authorization", ss.str());
		r.setParameters(req_params);

		if(!tmp_err)
			r.executeRequest(&tmp_err);
		if(tmp_err){
			std::cerr << " error in request of getting available QoS classes: " << tmp_err->getErrMsg() << std::endl;
		}else{
			std::vector<char> body = r.getAnswerContentVec();
			std::string response(body.begin(), body.end());
			json_object *info = json_tokener_parse(response.c_str());

			std::string classes = json_object_get_string(json_object_object_get(info, "children"));

			// Remove all extra chars and create a comma separated string to return
			classes.erase(std::remove(classes.begin(), classes.end(), '['), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), ']'), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), ' '), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), '"'), classes.end());

			//Adding prefix of cdmi capabilitiesURI
			std::string stype(type);
			std::string prefix = "/cdmi_capabilities/" + stype + "/";
			std::istringstream iss(classes);
			std::string classToken;
			classes="";
			while (std::getline(iss, classToken, ','))
			{
				classes += prefix + classToken + ",";
			}
			//remove final ,
			classes.erase(classes.size()-1);
			//std::cout << "QoS classes: "<< classes << std::endl;

			//std::cout << "content "<< response << std::endl;
			return classes.c_str();
		}
	} else {
		gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "type argument should be either dataobject or container");
	}
	return NULL;
}

const char* gfal_http_check_file_qos(plugin_handle plugin_data, const char *fileUrl, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
	DavixError* tmp_err=NULL;
	Context c;

	std::string uri(fileUrl);
	HttpRequest r(c, uri, &tmp_err);
	Davix::RequestParams req_params;
	davix->get_params(&req_params, Davix::Uri(fileUrl), false);
	r.setParameters(req_params);

	if(!tmp_err)
		r.executeRequest(&tmp_err);
	if(tmp_err){
		std::cerr << " error in request of checking file QoS: " << tmp_err->getErrMsg() << std::endl;
	}else{
		std::vector<char> body = r.getAnswerContentVec();
		std::string response(body.begin(), body.end());

		json_object *info = json_tokener_parse(response.c_str());
		std::string qos_class = json_object_get_string(json_object_object_get(info, "capabilitiesURI"));
		qos_class.erase(std::remove(qos_class.begin(), qos_class.end(), '"'), qos_class.end());

		//std::cout << "QoS class of file: "<< qos_class << std::endl;
		//std::cout << "content "<< response << std::endl;
		return qos_class.c_str();
	}
	return NULL;
}

const char* gfal_http_check_qos_available_transitions(plugin_handle plugin_data, const char *qosClassUrl, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
		DavixError* tmp_err=NULL;
		Context c;

		std::string uri(qosClassUrl);
		HttpRequest r(c, uri, &tmp_err);
		Davix::RequestParams req_params;
		davix->get_params(&req_params, Davix::Uri(qosClassUrl), false);
		r.setParameters(req_params);

		if(!tmp_err)
			r.executeRequest(&tmp_err);
		if(tmp_err){
			std::cerr << " error in request of checking file QoS: " << tmp_err->getErrMsg() << std::endl;
		}else{
			std::vector<char> body = r.getAnswerContentVec();
			std::string response(body.begin(), body.end());

			json_object *info = json_tokener_parse(response.c_str());
			json_object *metadata = json_object_object_get(info, "metadata");
			std::string transitions = json_object_get_string(json_object_object_get(metadata, "cdmi_capabilities_allowed"));//cdmi_capabilities_allowed

			// Remove all extra chars and create a comma separated string to return
			transitions.erase(std::remove(transitions.begin(), transitions.end(), '['), transitions.end());
			transitions.erase(std::remove(transitions.begin(), transitions.end(), ']'), transitions.end());
			transitions.erase(std::remove(transitions.begin(), transitions.end(), ' '), transitions.end());
			transitions.erase(std::remove(transitions.begin(), transitions.end(), '"'), transitions.end());
			transitions.erase(std::remove(transitions.begin(), transitions.end(), '\\'), transitions.end());

			//std::cout << "QoS class of file: "<< qos_class << std::endl;
			//std::cout << "content "<< response << std::endl;
			return transitions.c_str();
		}
		return NULL;
}