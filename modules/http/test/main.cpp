#include <iostream>
#include <type_traits>
#include <utility>
#include <fstream>
#include <sstream>
#include <vector>

#include "lux-web/http/server.hpp"


int main(int argc, char* argv[])
{
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address("0.0.0.0"), 10086);
	
	lux::web::http::Server server;

	static const char* ret		= "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nhello world";
	static const char* jpg_ret	= "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\nContent-Length:";

	server.beginWith(
		ep,
		[](const lux::web::http::Request& request, lux::web::http::Response& response)
		{
			std::cout << "url:" << request.uri << std::endl;

			std::stringstream ss;
			ss << jpg_ret;

			std::ifstream file("C:/Users/chenhui/Pictures/newone_cat08.jpg", std::ios::binary | std::ios::ate);
			auto filesize = static_cast<size_t>(file.tellg());
			file.seekg(0, std::ios::beg);
			std::vector<char> buffer(filesize);
			
			if (file.read(buffer.data(), filesize))
			{
				ss << std::to_string(filesize) << "\r\n\r\n";
				ss << std::string(buffer.data(), filesize);

				response.content = ss.str();
			}
		}
	);

	server.run();

	return 0;
}
