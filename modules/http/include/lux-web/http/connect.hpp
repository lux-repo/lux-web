#pragma once

#if _MSC_VER && !__INTEL_COMPILER
#   include <SDKDDKVer.h>
#endif

#include <string_view>
#include <array>
#include <memory>
#include <atomic>
#include <type_traits>
#include <boost/asio.hpp>

#include "parser.hpp"

namespace lux {
	namespace web {
		namespace http {
			struct Response
			{
				std::string content;
			};

			class ParserCallBack : public ParserSetting<ParserCallBack>
			{
				friend class ParserSetting<ParserCallBack>;
			private:
				int _on_url(Parser*, const char* at, size_t length);

				int _on_message_begin(Parser*);

				int _on_message_complete(Parser*);

				int _on_header_field(Parser*, const char* at, size_t length);

				int _on_header_value(Parser*, const char* at, size_t length);

				int _on_status(Parser*, const char* at, size_t length);

				int _on_body(Parser*, const char* at, size_t length);
			};

			class Connect :
				public std::enable_shared_from_this<Connect>
			{
			public:
				explicit Connect(boost::asio::ip::tcp::socket&& socket);

				~Connect();

				template<class Method>
				void requestHandle(Method&& method)
				{
					using MethodType = std::function<void(const Request&, Response&)>;
					using ErrorCode = boost::system::error_code;

					static_assert(std::is_convertible_v<Method, MethodType>, "Error Type of Method");

					_socket.async_read_some(
						boost::asio::buffer(_buffer),
						[_method{ std::forward<Method>(method) }, self{ shared_from_this() }](const ErrorCode& err, size_t len)
						{
							if (err)
							{
								self->_socket.close();
								return;
							}

							self->receive_handle(len);

							if (self->_parser.parse_done())
							{
								auto response_ptr = std::make_shared<Response>();
								_method(self->_parser.request(), *response_ptr);
								self->_parser.reset();
								self->send_response(std::move(response_ptr));
							}
							else
							{
								self->requestHandle(std::forward<Method>(_method));
							}
						}
					);
				}

			protected:
				void receive_handle(size_t len);

				void send_response(const std::shared_ptr<Response>&);

			private:
				boost::asio::ip::tcp::socket	_socket;
				Parser							_parser;
				std::array<char, 8192>			_buffer;
			};
		}
	}
}
