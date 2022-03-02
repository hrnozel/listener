#ifndef LISTENER_HPP
#define LISTENER_HPP

#ifdef _WIN32
    #include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
    #include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <time.h>
#endif

#include <string>
#include <cstdlib>
#include <sstream>

class MulticastListener
{

  private:
    std::string m_group;
    int m_port;
    struct sockaddr_in m_addr;

    int m_fd;
    
    constexpr std::size_t msg_size{256};

  public:

    MulticastListener() = delete;

    MulticastListener(const std::string& group, const int& port) : m_group{group},
      m_port{port}
    {

    };
    ~MulticastListener()
    {
#ifdef _WIN32
      WSACleanup();
#endif
    }

    bool listen(const std::string& group, const int& port)
    {
      m_group = group;
      m_port = port;
      return listen();
    }

    bool listen()
    {
#ifdef _WIN32
      WSADATA wsaData;
      if (WSAStartup(0x0101, &wsaData)) {
        return false;
      }
#endif

      m_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if (m_fd < 0) {
        return false;
      }

      u_int yes = 1;
      if (setsockopt(
        m_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0)
      {
        return false;
      }

      memset(&m_addr, 0, sizeof(m_addr));
      m_addr.sin_family = AF_INET;
      m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      m_addr.sin_port = htons(m_port);

      if (bind(m_fd, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0) {
        return false;
      }

      struct ip_mreq mreq;
      mreq.imr_multiaddr.s_addr = inet_addr(m_group.c_str());
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);
      if (setsockopt(m_fd, IPPROTO_IP,
        IP_ADD_MEMBERSHIP, (char*)&mreq,
        sizeof(mreq)) < 0)
      {
        return false;
      }

      return true;
    }

    [[nodiscard]] std::string getMessage()
    {

      while (true)
      {
        uint8_t msgbuf[msg_size]{};
#if defined(__APPLE__) || defined(linux)
				socklen_t addrlen = sizeof(m_addr);
#else
				int addrlen = sizeof(m_addr);
#endif
        int nbytes = recvfrom(
          m_fd,
          reinterpret_cast<char*>(msgbuf),
          MSGBUFSIZE - 1,
#if defined(__APPLE__) || defined(linux)
          MSG_WAITALL,
#else
          0,
#endif
          (struct sockaddr*)&m_addr,
          &addrlen
        );

        if (nbytes < 0) 
	{
	  return std::string{};
        }

	std::string message{ reinterpret_cast<char*>(msgbuf), nbytes };
        return message;
      }

    }
};
#endif // LISTENER_HPP
