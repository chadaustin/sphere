#include <stdio.h>
#include "../../third-party/NL.h" // HawkNL
#include "sdl_network.hpp"


struct NSOCKETimp
{
  NLsocket socket;
  bool is_connected;
  bool is_listening;
};


static bool s_NetworkInitialized = false;

////////////////////////////////////////////////////////////////////////////////

bool InitNetworkSystem()
{
  if (nlInit() == NL_FALSE)
    return false;
  nlSelectNetwork(NL_IP);

  s_NetworkInitialized = true;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CloseNetworkSystem()
{
  nlShutdown();
  s_NetworkInitialized = false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GetLocalName(char* name, int size)
{
  if (s_NetworkInitialized) {
    //sprintf(name, "SDL-Sphere");

    // null the stuff... since HawkNL will automatically connect to itself
    // if the name of the address is null... which somehow the smoketest
    // will pass O_o
    name[0] = 0;
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool GetLocalAddress(char* name, int size)
{
  if (s_NetworkInitialized) {

    // make a dummy socket
    address_t local_address;
    NLsocket s = nlOpen(0, NL_RELIABLE);
    nlGetLocalAddr(s, &local_address);

    // format the address for use
    NLbyte buffer[NL_MAX_STRING_LENGTH];
    nlAddrToString(&local_address, buffer);

    name[size - 1] = 0;
    sprintf(name, "%s", buffer);
    return true;
    
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

NSOCKET OpenAddress(const char* name, int port)
{
  // lookup for the address
  address_t address;
  nlGetAddrFromName((NLbyte*)name, &address);
  nlSetAddrPort(&address, port);

  // create a new socket
  NSOCKET s = new NSOCKETimp;
  s->socket = NULL;
  s->is_connected = false;
  s->is_listening = false;

  s->socket = nlOpen(0, NL_RELIABLE);
  if (s->socket == NL_INVALID)
  {
    delete s;
    return NULL;
  }

  // see if the address is accepted by the socket.
  if (nlConnect(s->socket, &address) == NL_FALSE)
  {
    printf("Error! %s\n", nlGetErrorStr(nlGetError()));
    printf("Error! %s\n", nlGetSystemErrorStr(nlGetSystemError()));
    nlClose(s->socket);
    delete s;
    return NULL;
  }

  return s;
}

////////////////////////////////////////////////////////////////////////////////

NSOCKET ListenOnPort(int port)
{
  NSOCKET s = new NSOCKETimp;
  s->socket = NULL;
  s->is_connected = false;
  s->is_listening = false;

  // create a socket
  s->socket = nlOpen(port, NL_RELIABLE);
  if (s->socket == NL_INVALID)
  {
    delete s;
    return NULL;
  }

  // enable listening mode
  if (nlListen(s->socket) == NL_FALSE)
  {
    delete s;
    return NULL;
  }

  s->is_listening = true;
  return s;
}

////////////////////////////////////////////////////////////////////////////////

void CloseSocket(NSOCKET socket)
{
  nlClose(socket->socket);
  delete socket;
}

////////////////////////////////////////////////////////////////////////////////

bool IsConnected(NSOCKET socket)
{
  if (socket->is_connected) {
    return true;
  } else {
    // check to see if we're connected
    if (socket->is_listening) {

      NLsocket connection = nlAcceptConnection(socket->socket);
      if (connection != NL_INVALID) {
        nlClose(socket->socket);
        socket->socket = connection;
        socket->is_connected = true;
        return true;
      }

      return false;

    } else {

      // I know, I know, but HawkNL is limited in determining stuff
      // dammnit ~_~
      socket->is_connected = true;
      return true;

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

int GetPendingReadSize(NSOCKET socket)
{
  if (IsConnected(socket)) {

    // return *any* size, heh heh
    return 256;

  } else {
    return -1;
  }
}

////////////////////////////////////////////////////////////////////////////////

int SocketRead(NSOCKET socket, void* buffer, int size)
{
  if (IsConnected(socket)) {
    return nlRead(socket->socket, buffer, size);
  }

  return -1;
}

////////////////////////////////////////////////////////////////////////////////

void SocketWrite(NSOCKET socket, void* buffer, int size)
{
  if (IsConnected(socket)) {
    nlWrite(socket->socket, buffer, size);
  }
}

////////////////////////////////////////////////////////////////////////////////
