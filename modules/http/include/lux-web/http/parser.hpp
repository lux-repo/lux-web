#pragma once

#ifndef _LLHTTP_HPP_
#define _LLHTTP_HPP_
#include "request.hpp"
#include <string>
#include <string_view>
#include <type_traits>

/**
* To use this Class, You should inherit server::http::Parser(CRTP template class).
* Then you may be need to implement the interface as below in your subclass.
*
* int _on_message_begin();
* int _on_url(const char* at, size_t length);
* int _on_status(const char* at, size_t length);
* int _on_header_field(const char* at, size_t length);
* int _on_header_value(const char* at, size_t length);
* int _on_headers_complete();
* int _on_body(const char* at, size_t length);
* int _on_message_complete();
* int _on_chunk_header();
* int _on_chunk_complete();
* int _on_url_complete();
* int _on_status_complete();
* int _on_header_field_complete();
* int _on_header_value_complete();
*/

#define _CB_FUNCTION_DECLARE(name)                                              \
    static int __ ## name(llhttp_t* lparser)                                    \
    {                                                                           \
        auto* parser = (Parser*)lparser->data;                                  \
        auto* self   = (ParserSetting*)parser->setting();                       \
        return self->name(parser);                                              \
    }

#define _DATA_CB_FUNCTION_DECLARE(name)                                         \
    static int __ ## name(llhttp_t* lparser, const char* at, size_t length)     \
    {                                                                           \
        auto* parser = (Parser*)lparser->data;                                  \
        auto* self   = (ParserSetting*)parser->setting();                       \
        return self->name(parser, at, length);                                  \
    }

#define _HAS_MEMBER_FUNCTION(name, ...)                                         \
    template<class T>                                                           \
    class has_ ## name                                                          \
    {                                                                           \
    private:                                                                    \
        template<typename U, int (U::*)(__VA_ARGS__) = &U::_ ## name>           \
        static constexpr bool check(U*) { return true; }                        \
        static constexpr bool check(...) { return false; }                      \
    public:                                                                     \
        static constexpr bool value = check(static_cast<T*>(0));                \
    };                                                                          

#define _FUNCTION_INTERFACE_DECLARE(name)                                       \
    _HAS_MEMBER_FUNCTION(name, Parser*)                                         \
    int name(Parser* p){ return static_cast<SubClass*>(this)->_ ## name(p); }   \
    _CB_FUNCTION_DECLARE(name)

#define _DATA_FUNCTION_INTERFACE_DECLARE(name)                                  \
    _HAS_MEMBER_FUNCTION(name, Parser*, const char*, size_t)                    \
    int name(Parser* p, const char* at, size_t length){                         \
        return static_cast<SubClass*>(this)->_ ## name(p, at, length);          \
    }                                                                           \
    _DATA_CB_FUNCTION_DECLARE(name)

namespace lux {
	namespace web {
		namespace http {
			class Parser;

			template<class SubClass>
			class ParserSetting
			{
			public:
				ParserSetting()
				{
					llhttp_settings_init(&_low_layer_setting);
					__bind_low_layer_setting();
				}

				/* Possible return values 0, -1, `HPE_PAUSED` */
				_FUNCTION_INTERFACE_DECLARE(on_message_begin)

					/* Possible return values 0, -1, HPE_USER */
					_DATA_FUNCTION_INTERFACE_DECLARE(on_url)
					_DATA_FUNCTION_INTERFACE_DECLARE(on_status)
					_DATA_FUNCTION_INTERFACE_DECLARE(on_header_field)
					_DATA_FUNCTION_INTERFACE_DECLARE(on_header_value)

					/*
					* Possible return values:
					* 0  - Proceed normally
					* 1  - Assume that request/response has no body, and proceed to parsing the
					*      next message
					* 2  - Assume absence of body (as above) and make `llhttp_execute()` return
					*      `HPE_PAUSED_UPGRADE`
					* -1 - Error
					* `HPE_PAUSED`
					*/
					_FUNCTION_INTERFACE_DECLARE(on_headers_complete)

					/* Possible return values 0, -1, HPE_USER */
					_DATA_FUNCTION_INTERFACE_DECLARE(on_body)

					/* Possible return values 0, -1, `HPE_PAUSED` */
					_FUNCTION_INTERFACE_DECLARE(on_message_complete)

					/*
					* When on_chunk_header is called, the current chunk length is stored
					* in parser->content_length.
					* Possible return values 0, -1, `HPE_PAUSED`
					*/
					_FUNCTION_INTERFACE_DECLARE(on_chunk_header)
					_FUNCTION_INTERFACE_DECLARE(on_chunk_complete)

					/* Information-only callbacks, return value is ignored */
					_FUNCTION_INTERFACE_DECLARE(on_url_complete)
					_FUNCTION_INTERFACE_DECLARE(on_status_complete)
					_FUNCTION_INTERFACE_DECLARE(on_header_field_complete)
					_FUNCTION_INTERFACE_DECLARE(on_header_value_complete)

					llhttp_settings_t& low_layer_setting() {
					return _low_layer_setting;
				}

			private:

#define HAS_IMPLEMENT_FUNCTION(name) \
            has_ ## name<SubClass>  :: value

#define _BIND_CB(name) \
            if constexpr(HAS_IMPLEMENT_FUNCTION(name)) _low_layer_setting.name = __ ## name

				void __bind_low_layer_setting()
				{
					_BIND_CB(on_message_begin);
					_BIND_CB(on_url);
					_BIND_CB(on_status);
					_BIND_CB(on_header_field);
					_BIND_CB(on_header_value);
					_BIND_CB(on_headers_complete);
					_BIND_CB(on_body);
					_BIND_CB(on_message_complete);
					_BIND_CB(on_chunk_header);
					_BIND_CB(on_chunk_complete);
					_BIND_CB(on_url_complete);
					_BIND_CB(on_status_complete);
					_BIND_CB(on_header_field_complete);
					_BIND_CB(on_header_value_complete);
				}

				llhttp_settings_t  _low_layer_setting;
			};

			class Parser
			{
			public:
				template<class SettingType>
				Parser(ParserSetting<SettingType>* setting)
				{
					llhttp_init(
						&_low_layer_parser,
						HTTP_BOTH,
						&setting->low_layer_setting()
					);
					_setting = static_cast<void*>(setting);
					_low_layer_parser.data = this;
				}

				uint8_t get_type()
				{
					return _low_layer_parser.type;
				}

				uint8_t get_http_major()
				{
					return _low_layer_parser.http_major;
				}

				uint8_t get_http_minor()
				{
					return _low_layer_parser.http_minor;
				}

				RequestMethod get_method()
				{
					return static_cast<RequestMethod>(_low_layer_parser.method);
				}

				const char* method_string(RequestMethod method)
				{
					return llhttp_method_name(static_cast<llhttp_method_t>(method));
				}

				int get_status_code()
				{
					return _low_layer_parser.status_code;
				}

				uint8_t get_upgrade()
				{
					return _low_layer_parser.upgrade;
				}

				/*
				 * Reset an already initialized parser back to the start state, preserving the
				 * existing parser type, callback settings, user data, and lenient flags.
				 */
				void reset()
				{
					_request.headers.clear();
					return llhttp_reset(&_low_layer_parser);
				}

				/* Parse full or partial request/response, invoking user callbacks along the
				 * way.
				 *
				 * If any of `llhttp_data_cb` returns errno not equal to `HPE_OK` - the parsing
				 * interrupts, and such errno is returned from `llhttp_execute()`. If
				 * `HPE_PAUSED` was used as a errno, the execution can be resumed with
				 * `llhttp_resume()` call.
				 *
				 * In a special case of CONNECT/Upgrade request/response `HPE_PAUSED_UPGRADE`
				 * is returned after fully parsing the request/response. If the user wishes to
				 * continue parsing, they need to invoke `llhttp_resume_after_upgrade()`.
				 *
				 * NOTE: if this function ever returns a non-pause type error, it will continue
				 * to return the same error upon each successive call up until `llhttp_init()`
				 * is called.
				 */
				llhttp_errno_t execute(const char* data, size_t len) noexcept
				{
					return llhttp_execute(&_low_layer_parser, data, len);
				}
				llhttp_errno_t execute(const std::string& data) noexcept
				{
					return llhttp_execute(&_low_layer_parser, data.c_str(), data.length());
				}
				llhttp_errno_t execute(std::string_view view) noexcept
				{
					return llhttp_execute(&_low_layer_parser, view.data(), view.length());
				}

				llhttp_errno_t finish()
				{
					return llhttp_finish(&_low_layer_parser);
				}

				/* Returns `1` if the incoming message is parsed until the last byte, and has
				 * to be completed by calling `llhttp_finish()` on EOF
				 */
				int message_needs_eof()
				{
					return llhttp_message_needs_eof(&_low_layer_parser);
				}

				/* Returns `1` if there might be any other messages following the last that was
				 * successfully parsed.
				 */
				int should_keep_alive()
				{
					return llhttp_should_keep_alive(&_low_layer_parser);
				}

				/* Make further calls of `llhttp_execute()` return `HPE_PAUSED` and set
				 * appropriate error reason.
				 *
				 * Important: do not call this from user callbacks! User callbacks must return
				 * `HPE_PAUSED` if pausing is required.
				 */
				void pause()
				{
					llhttp_pause(&_low_layer_parser);
				}

				/* Might be called to resume the execution after the pause in user's callback.
				 * See `llhttp_execute()` above for details.
				 *
				 * Call this only if `llhttp_execute()` returns `HPE_PAUSED`.
				 */
				void resume()
				{
					llhttp_resume(&_low_layer_parser);
				}

				/* Might be called to resume the execution after the pause in user's callback.
				 * See `llhttp_execute()` above for details.
				 *
				 * Call this only if `llhttp_execute()` returns `HPE_PAUSED_UPGRADE`
				 */
				void resume_after_upgrade()
				{
					llhttp_resume_after_upgrade(&_low_layer_parser);
				}

				/* Returns the latest return error */
				llhttp_errno_t get_errno()
				{
					return llhttp_get_errno(&_low_layer_parser);
				}

				/* Returns the verbal explanation of the latest returned error.
				 *
				 * Note: User callback should set error reason when returning the error. See
				 * `llhttp_set_error_reason()` for details.
				 */
				const char* get_error_reason()
				{
					return llhttp_get_error_reason(&_low_layer_parser);
				}

				/* Assign verbal description to the returned error. Must be called in user
				 * callbacks right before returning the errno.
				 *
				 * Note: `HPE_USER` error code might be useful in user callbacks.
				 */
				void set_error_reason(const char* reason)
				{
					return llhttp_set_error_reason(&_low_layer_parser, reason);
				}

				/* Returns the pointer to the last parsed byte before the returned error. The
				 * pointer is relative to the `data` argument of `llhttp_execute()`.
				 *
				 * Note: this method might be useful for counting the number of parsed bytes.
				 */
				const char* get_error_pos()
				{
					return llhttp_get_error_pos(&_low_layer_parser);
				}

				/* Returns textual name of error code */
				static const char* errno_name(llhttp_errno_t err)
				{
					return llhttp_errno_name(err);
				}

				/* Returns textual name of HTTP method */
				static const char* method_name(llhttp_method_t method)
				{
					return llhttp_method_name(method);
				}

				/* Enables/disables lenient header value parsing (disabled by default).
				 *
				 * Lenient parsing disables header value token checks, extending llhttp's
				 * protocol support to highly non-compliant clients/server. No
				 * `HPE_INVALID_HEADER_TOKEN` will be raised for incorrect header values when
				 * lenient parsing is "on".
				 *
				 * **(USE AT YOUR OWN RISK)**
				 */
				void set_lenient_headers(int enabled)
				{
					return llhttp_set_lenient_headers(&_low_layer_parser, enabled);
				}

				/* Enables/disables lenient handling of conflicting `Transfer-Encoding` and
				 * `Content-Length` headers (disabled by default).
				 *
				 * Normally `llhttp` would error when `Transfer-Encoding` is present in
				 * conjunction with `Content-Length`. This error is important to prevent HTTP
				 * request smuggling, but may be less desirable for small number of cases
				 * involving legacy servers.
				 *
				 * **(USE AT YOUR OWN RISK)**
				 */
				void set_lenient_chunked_length(int enabled)
				{
					return llhttp_set_lenient_chunked_length(&_low_layer_parser, enabled);
				}

				/* Enables/disables lenient handling of `Connection: close` and HTTP/1.0
				 * requests responses.
				 *
				 * Normally `llhttp` would error on (in strict mode) or discard (in loose mode)
				 * the HTTP request/response after the request/response with `Connection: close`
				 * and `Content-Length`. This is important to prevent cache poisoning attacks,
				 * but might interact badly with outdated and insecure clients. With this flag
				 * the extra request/response will be parsed normally.
				 *
				 * **(USE AT YOUR OWN RISK)**
				 */
				void set_lenient_keep_alive(int enabled)
				{
					return llhttp_set_lenient_keep_alive(&_low_layer_parser, enabled);
				}

				void* setting()
				{
					return _setting;
				}

				Request& request()
				{
					return _request;
				}

				bool parse_done()
				{
					return _low_layer_parser.finish != HTTP_FINISH_UNSAFE;
				}

			protected:
				llhttp_t            _low_layer_parser;
				Request		        _request;
				void* _setting;
			};
		}
	}
}

#endif
