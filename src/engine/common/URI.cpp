#include "URI.h"
#include <string>
#include <algorithm>

URI::URI (const std::string& uri)
{
	const std::string prot_end("://");

	const std::string::const_iterator uriEnd = uri.end();
	std::string::const_iterator queryStart = std::find(uri.begin(), uriEnd, '?');
	std::string::const_iterator protocolStart = uri.begin();
	std::string::const_iterator protocolEnd = std::find(protocolStart, uriEnd, ':');

	if (protocolEnd != uriEnd) {
		std::string prot = &*protocolEnd;
		if (prot.length() > prot_end.length() && prot.substr(0, prot_end.length()) == prot_end) {
			_protocol = std::string(protocolStart, protocolEnd);
			protocolEnd += prot_end.length();
		} else {
			protocolEnd = uri.begin();
		}
	} else {
		protocolEnd = uri.begin();
	}

	std::string::const_iterator hostStart = protocolEnd;
	std::string::const_iterator pathStart = std::find(hostStart, uriEnd, '/');

	std::string::const_iterator hostEnd = std::find(protocolEnd, pathStart != uriEnd ? pathStart : queryStart, ':');

	_host = std::string(hostStart, hostEnd);

	if (hostEnd != uriEnd && *hostEnd == ':') {
		++hostEnd;
		std::string::const_iterator portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
		_port = std::string(hostEnd, portEnd);
	}

	if (pathStart != uriEnd)
		_path = std::string(pathStart, queryStart);

	if (queryStart != uriEnd)
		_query = std::string(queryStart, uri.end());
}

URI::~URI ()
{
}
