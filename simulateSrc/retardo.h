#ifndef retardo_h
#define retardo_h
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/core-module.h"
#include "ns3/udp-server.h"
#include "ns3/onoff-application.h"
#include "ns3/node-container.h"
#include "ns3/udp-echo-client.h"
#include "ns3/point-to-point-net-device.h"


using namespace ns3;

class Retardo
{
public:
		   Retardo ();
  void     AddServers (NodeContainer nodos);
  void     TxIni (Ptr<const Packet> paquete);
  void     RxEnd (Ptr<const Packet> paquete);
  void 	   NewCall (Ptr<OnOffApplication> emisor);
  Time     RetardoMedio   ();

private:
  Time     average;
  uint32_t  m_cuenta;
};

#endif