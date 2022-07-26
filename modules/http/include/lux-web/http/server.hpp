#pragma once
#if _MSC_VER && !__INTEL_COMPILER
#   include <SDKDDKVer.h>
#endif

#include <boost/asio.hpp>
#include "connect.hpp"

namespace lux {
	namespace web {
		namespace http {
			using BIOContext = boost::asio::io_context;

			class Server
			{
			public:
				using IOContext = boost::asio::io_context;
				using IOContextSPtr = std::shared_ptr<IOContext>;
				using BSignal = boost::asio::signal_set;
				using BSignalUniquePtr = std::unique_ptr<BSignal>;
				using BAcceptor = boost::asio::ip::tcp::acceptor;
				using BacceptorUniquePtr = std::unique_ptr<BAcceptor>;

				template<class Func>
				void beginWith(boost::asio::ip::tcp::endpoint ep, Func&& func)
				{
					_signal_ptr = std::make_unique<BSignal>(_io_context);
					_signal_ptr->add(SIGINT);
					_signal_ptr->add(SIGTERM);
#if defined(SIGQUIT)
					_signal_ptr->add(SIGQUIT);
#endif
					_do_await_stop();

					_acceptor_ptr = std::make_unique<BAcceptor>(_io_context, ep);
					_acceptor_ptr->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
					_acceptor_ptr->listen();
					_accept(std::forward<Func>(func));
				}

				void run()
				{
					_io_context.run();
				}

			private:

				template<class Func>
				void _accept(Func&& func)
				{
					_acceptor_ptr->async_accept(
						[_func{ std::forward<Func>(func) }, this]
					(const boost::system::error_code& err, boost::asio::ip::tcp::socket socket)
						{
							if (!_acceptor_ptr->is_open())
							{
								return;
							}

							if (err)
							{
								std::cerr << err.value() << " " << err.message() << std::endl;
								return;
							}

							std::make_shared<Connect>(std::move(socket))->requestHandle(_func);
							_accept(_func);
						}
					);
				}

				void _do_await_stop()
				{
					_signal_ptr->async_wait(
						[this](boost::system::error_code, int) {_acceptor_ptr->close(); }
					);
				}

				IOContext           _io_context;
				BSignalUniquePtr    _signal_ptr;
				BacceptorUniquePtr  _acceptor_ptr;
			};
		}
	}
}
