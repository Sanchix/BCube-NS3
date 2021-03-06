#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/random-variable-stream.h"
#include "ns3/on-off-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/default-deleter.h"
#include "retardo.h"

#define PUERTO 9
#define LIBRE 0
#define OCUPADO 1
#define IMPACIENTE 2

using namespace ns3;

class Llamada {
 public:
 	Llamada(NodeContainer nodos, double duracion, double max_t_inicio, bool ControlaTrafico, uint32_t PorcentajeTrafico);
 	void Call(Ptr<Node> nodo_origen);
 	void Hang(Ptr<Node> nodo_origen,Ptr<Node> nodo_destino);
	Retardo GetObserver();

 private:
  NodeContainer TodosNodos;
  Ptr<ExponentialRandomVariable> Exp_duracion;
  Ptr<UniformRandomVariable> Uniform_t_inicio;
  Ptr<ExponentialRandomVariable> Exp_ON;
  Ptr<ExponentialRandomVariable> Exp_OFF;
  Ptr<ConstantRandomVariable> Exp_0;
  Ptr<ConstantRandomVariable> Exp_1;
  std::vector<int> *nodeCalledList;
  int nNodesInCall;
  Ptr<UniformRandomVariable> Uniform_equipo_destino;
  DataRate TasaApp;
  uint32_t TamPack;
  Retardo obs_ret;
  uint32_t PorcentajeTrafico;
  bool ControlaTrafico;
};     
