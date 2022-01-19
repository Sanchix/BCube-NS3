#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/random-variable-stream.h"
#include "ns3/on-off-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-l3-protocol.h"

#define PUERTO 9
#define LIBRE 0
#define OCUPADO 1
#define IMPACIENTE 2


using namespace ns3;

class Llamada {
 public:
 	Llamada(NodeContainer nodos, double duracion, double max_t_inicio);
 	void Call(Ptr<Node> nodo_origen);
 	void Hang(Ptr<Node> nodo_origen,Ptr<Node> nodo_destino);

 private:
 	NodeContainer TodosNodos;
 	Ptr<ExponentialRandomVariable> Exp_duracion;
    Ptr<UniformRandomVariable> Uniform_t_inicio;
 	std::vector<int> nodeCalledList;
 	Ptr<UniformRandomVariable> Uniform_equipo_destino;
 	DataRate TasaApp;
 	uint32_t TamPack;
};
