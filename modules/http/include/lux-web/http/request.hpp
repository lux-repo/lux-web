#pragma once
#include "lux-web/http/llhttp/llhttp.h"
#include <utility>
#include <string_view>
#include <cstdint>
#include <vector>

namespace lux {
	namespace web {
		namespace http {

			using RequestMethod = llhttp_method_t;
			using Header = std::pair<std::string_view, std::string_view>;

			struct Request
			{
				RequestMethod		method;
				std::string_view	uri;
				uint8_t				version_major;
				uint8_t				version_minor;
				std::vector<Header>	headers;
				std::string_view	status;
				std::string_view	body;
			};
		}
	}
}