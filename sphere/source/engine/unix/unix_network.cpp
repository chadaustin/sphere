#include "unix_network.h"
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

const int MAX_LINE = 1024;

bool GetLocalName (char* name, int size) {
  if ((gethostname(name, size) < 0) && (errno != ENAMETOOLONG))
    return false;
  return true;
}

bool GetLocalAddress (char* name, int size) {
  struct hostent* host;
  char hostname [256]; /* more than we really need */
  char* address;

  if(!GetLocalName(hostname, 256))
    return false;
  host = gethostbyname2(hostname, AF_INET);
  if (!host)
    return false;
  address = inet_ntoa(*(struct in_addr*)host->h_addr);
  if (strlen(address) > size) { /* name is too small to copy address into */
    free(host);
    return false;
  }
  strcpy(name, address);
  return true;
}

NSOCKET OpenAddress (const char* name, int port) {
  struct hostent* host;
  struct sockaddr_in server;
  NSOCKET sock = new NSOCKETimp;
  int err;

  sock->socket = socket(PF_INET, SOCK_STREAM, 0);
  if (sock->socket < 0) {
    delete sock;
    return NULL;
  }
  sock->is_connected = false;
  sock->is_listening = false;
  host = gethostbyname(name);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr = *(struct in_addr*)host->h_addr;
  fcntl(sock->socket, F_SETFL, fcntl(sock->socket, F_GETFL, 0) | O_NONBLOCK);
  err = connect(sock->socket, (struct sockaddr*)&server, sizeof(server));
  if ((err < 0) && (errno != EINPROGRESS)) {
    delete sock;
    free(host);
    return NULL;
  }
  sock->is_connected = true;
  return sock;
}

NSOCKET ListenOnPort (int port) {
  struct sockaddr_in address;
  NSOCKET sock = new NSOCKETimp;

  sock->socket = socket(PF_INET, SOCK_STREAM, 0);
  if (sock->socket < 0) {
    delete sock;
    return NULL;
  }
  sock->is_connected = false;
  sock->is_listening = false;
  address.sin_family = AF_INET;
  address.sin_port = port;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock->socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
    delete sock;
    return NULL;
  }
  fcntl(sock->socket, F_SETFL, fcntl(sock->socket, F_GETFL, 0) | O_NONBLOCK);
  if (listen(sock->socket, 1) < 0) {
    delete sock;
    return NULL;
  }
  sock->is_listening = true;
  return sock;
}

void CloseSocket (NSOCKET socket) {
  close(socket->socket);
  delete socket;
}

bool IsConnected (NSOCKET socket) {
  struct sockaddr_in client;
  size_t size;
  int connection;

  if (socket->is_connected)
    return true;
  if (socket->is_listening) {
    connection = accept(socket->socket, (struct sockaddr*)&client, &size);
    if (connection >= 0) {
      close(socket->socket);
      socket->socket = connection;
      socket->is_connected = true;
      return true;
    }
    return false;
  } else {
    if (GetPendingReadSize(socket) < 0)
      return false;
    return true;
  }
  return false;
}

int GetPendingReadSize (NSOCKET socket) {
  char buffer[MAX_LINE];
  int num_bytes;

  if (!IsConnected(socket))
    return -1;
  /* We don't have ioctl's FIONREAD in POSIX or GNU, so instead we'll peek at the data.  Note that this function probably will perform a little bit differently than the win32 version, but won't affect program correctness. */
  num_bytes = recv(socket->socket, buffer, 1024, MSG_PEEK);
  if (num_bytes == -1) {
    if (errno == EWOULDBLOCK)
      return 0;
    else
      return -1;
  }
  return num_bytes;
}

int SocketRead (NSOCKET socket, void* buffer, int size) {
  int num_read = 0;
  int total_read = 0;
  char* read_ptr = (char*)buffer;

  if (!IsConnected(socket))
    return -1;
  while (((num_read = read(socket->socket, read_ptr,
	 size - total_read)) != 0) && (total_read < size)) {
    if (num_read == -1) {
	   if (errno == EWOULDBLOCK)
		  return 0;
		else
		  return -1;
	 }
	 total_read += num_read;
	 read_ptr += num_read;
  }
  return total_read;
}

void SocketWrite (NSOCKET socket, void* buffer, int size) {
  if (IsConnected(socket))
    write(socket->socket, buffer, size);
}
