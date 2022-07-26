#include "lux-web/http/connect.hpp"
#include <iostream>

namespace lux {
	namespace web {
		namespace http {
			static ParserCallBack static_parse_callback;

			Connect::Connect(boost::asio::ip::tcp::socket&& socket)
				: _socket(std::move(socket)),
				_parser(&static_parse_callback) {}

			Connect::~Connect() = default;

			void Connect::receive_handle(size_t len)
			{
				auto parse_err = _parser.execute(_buffer.data(), len);

				if (parse_err != HPE_OK)
				{
					// @TODO handle parse error
					return;
				}
			}

			void Connect::send_response(const std::shared_ptr<Response>& response)
			{
				boost::asio::async_write(
					_socket,
					boost::asio::buffer(response->content),
					[self{ shared_from_this() }, response](const boost::system::error_code& err, size_t len)
					{
						if (err)
						{
							std::cerr << err.message() << std::endl;
						}
						self->_socket.close();
					}
				);
			}

			int ParserCallBack::_on_url(Parser* parser, const char* at, size_t length)
			{
				parser->request().uri = std::string_view(at, length);
				return 0;
			}

			int ParserCallBack::_on_message_begin(Parser* parser)
			{
				return 0;
			}

			int ParserCallBack::_on_message_complete(Parser* parser)
			{
				parser->request().method = parser->get_method();
				parser->request().version_major = parser->get_http_major();
				parser->request().version_minor = parser->get_http_minor();
				return 0;
			}

			int ParserCallBack::_on_header_field(Parser* parser, const char* at, size_t length)
			{
				parser->request().headers.push_back(
					{ std::string_view(at, length), std::string_view() }
				);
				return 0;
			}

			int ParserCallBack::_on_header_value(Parser* parser, const char* at, size_t length)
			{
				parser->request().headers.back().second = std::string_view(at, length);
				return 0;
			}

			int ParserCallBack::_on_status(Parser* parser, const char* at, size_t length)
			{
				parser->request().status = std::string_view(at, length);
				return 0;
			}

			int ParserCallBack::_on_body(Parser* parser, const char* at, size_t length)
			{
				parser->request().body = std::string_view(at, length);
				return 0;
			}
		}
	}
}