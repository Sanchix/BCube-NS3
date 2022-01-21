#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/point-to-point-net-device.h"


using namespace ns3;

class Retardo
{
public:
           Retardo      (Ptr<UdpEchoClient>    emisor,Ptr<UdpEchoServer> receptor);
  void     TxIni (Ptr<const Packet> paquete);
  void     RxEnd (Ptr<const Packet> paquete);
  Time     RetardoMedio   ();

private:
  Time     average;
  uint32_t  m_cuenta;
};
