// SEDNL - Copyright (c) 2013 Jeremy S. Cochoy
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would
//        be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//        be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//        distribution.

#include "SEDNL/TCPServer.hpp"
#include "SEDNL/SocketHelp.hpp"
#include "SEDNL/Exception.hpp"
#include "SEDNL/SocketAddress.hpp"
#include "SEDNL/EventListener.hpp"

#include <cstring>
#include <memory>

namespace SedNL
{

TCPServer::TCPServer() noexcept
    :m_listener(nullptr)
{}

TCPServer::TCPServer(const SocketAddress& socket_address)
    :m_listener(nullptr)
{
    connect(socket_address);
}

void TCPServer::connect(const SocketAddress& socket_address)
{
    if (!socket_address.is_server_valid())
        throw NetworkException(NetworkExceptionT::InvalidSocketAddress);

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Both IPV6 and IPV4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // We want to bind on

    struct addrinfo* addrs = nullptr;

    //Create a lambda deleter
    auto deleter = [](struct addrinfo* ptr)
        { if (ptr != nullptr) freeaddrinfo(ptr); };

    //Will store addresses to allow RAII.
    std::unique_ptr<struct addrinfo, void(*)(struct addrinfo*)>
        resources_keeper(nullptr, deleter);

    retrieve_addresses(socket_address.m_name, socket_address.m_port,
                       hints, addrs,
                       resources_keeper, deleter);

    //Socket FileDescriptor
    FileDescriptor fd;
    struct addrinfo *addr = nullptr;
    for (addr = addrs; addr != nullptr; addr = addr->ai_next)
    {
        fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (fd == -1)
            continue;

        int errcode = bind(fd, addr->ai_addr, addr->ai_addrlen);

        //We binded on this socket
        if (errcode == 0)
            break;

        //Failed, let's try again
        close (fd);
    }

    if (addr == nullptr)
        throw NetworkException(NetworkExceptionT::BindFailed);

    if (!set_non_blocking(fd))
        throw NetworkException(NetworkExceptionT::CantSetNonblocking);

    if (listen(fd, MAX_CONNECTIONS) < 0)
        throw NetworkException(NetworkExceptionT::ListenFailed);

    m_connected = true;
    m_fd = fd;
}

void TCPServer::disconnect() noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_connected)
        {
            //Tell the listener that the connection will be closed
            if (m_listener)
            {
                m_listener->tell_disconnected(this);
                try {
                    m_listener->m_fd_lock.lock();
                } catch(...) {}
            }

            close(m_fd);
            m_fd = -1;
            if (m_listener)
                try {
                    m_listener->m_fd_lock.unlock();
                } catch(...) {}

            m_connected = false;
        }
    }
    catch(std::exception &e)
    {
#ifndef SEDNL_NOWARN
        std::cerr << "Error: "
                  << "std::mutex::lock failed in SedNL::TCPServer::disconnect"
                  << std::endl;
        std::cerr << e.what() << std::endl;
#endif /* SEDNL_NOWARN */
        //Let's try without thread safety :/
        close(m_fd);
        m_fd = -1;
        m_connected = false;
    }
}

} //namespace SedNL
