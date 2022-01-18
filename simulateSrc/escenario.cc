
#include "ns3/command-line.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/udp-echo-client.h"
#include "ns3/application-container.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/uinteger.h"
#include "ns3/error-model.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/csma-helper.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/queue-disc.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/queue.h"
#include "ns3/csma-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/fifo-queue-disc.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-nix-vector-routing.h"

#include "escenario.h"
#include "puente_helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Escenario");

double escenario(StageConfig_t *config){
	
	// Eliminar cualquier variable o elemento de red de una simulación anterior.
	Simulator::Destroy();
	
	NodeContainer todosNodos = topologiaFisica(config->bCubeLevel, config->nNodosDim);
	NS_LOG_INFO("Topology generated");

	Ptr<Node> aux;
	Ptr<Ipv4L3Protocol> L_IP;

	for (uint32_t i = 0; i<todosNodos.GetN()-1;i++){
		aux = todosNodos.Get(i);
		NS_LOG_INFO("ID: "<<aux->GetId() << "  IP:");
		
		L_IP = aux->GetObject<Ipv4L3Protocol> ();
		for (uint32_t j=0; j< L_IP->GetNInterfaces()-1 ;j++){
			NS_LOG_INFO( L_IP->GetAddress (i, 0).GetLocal () );
		}
 
	}
	
	return 1;
	
}

NodeContainer topologiaFisica(int nDims, int dimSize){
	
	int numEquipos = pow(dimSize,nDims+1);
	NS_LOG_INFO("NumEquipos = " << numEquipos);
	
	NodeContainer todosNodos(numEquipos);
	NetDeviceContainer c_dispositivos;
	
	Ipv4NixVectorHelper h_Nix_Routing;
	
	InternetStackHelper h_pila;
	h_pila.SetRoutingHelper(h_Nix_Routing);
	h_pila.SetIpv6StackInstall (false);
	h_pila.Install (todosNodos);

	PuenteConfig_t puenteConfig;
	puenteConfig.regimenBinario = DataRate(0);
		
	if (nDims==0){
		
		NS_LOG_INFO("BCUBE 0");
		NetDeviceContainer nodosLan;
		PuenteHelper(todosNodos, nodosLan, &puenteConfig);
		c_dispositivos.Add(nodosLan);
		
	}
	
	else if (nDims==1){
		
		NS_LOG_INFO("BCUBE 1");
		
		// Para cada elemendo de fila/columna
		for (int i = 0; i < dimSize; i++){
			
			NS_LOG_DEBUG("Row/column " << i);
			
			NodeContainer row;
			NetDeviceContainer nodosLan;
			
			NS_LOG_DEBUG("Connecting row " << i);
			for (int j = 0; j < dimSize; j++){
				row.Add(todosNodos.Get(dimSize*i+j));
			}
			NS_LOG_DEBUG("Connected row " << i);
			
			PuenteHelper(row, nodosLan, &puenteConfig);
			c_dispositivos.Add(nodosLan);
				
		
			NodeContainer column;
			NetDeviceContainer nodosLan2;
			
			NS_LOG_DEBUG("Connecting column " << i);
			for (int j = 0; j < dimSize; j++){
				NS_LOG_DEBUG("j="<<j<<", nodeID="<<dimSize*j+i);
				column.Add(todosNodos.Get(dimSize*j+i));
			}
			NS_LOG_DEBUG("Connected column " << i);
						
			PuenteHelper(column, nodosLan2, &puenteConfig);
			c_dispositivos.Add(nodosLan2);
			
		}
		
	}

	else if (nDims==2){
		
		NS_LOG_INFO("BCUBE 2");
		
		// Por cada elemento de cara
		for (int i = 0; i<dimSize*dimSize; i++){
			
			//Primero conectamos horizontalmente
			NodeContainer row; 
			NetDeviceContainer nodosLan;
			for (int j=0; j<dimSize; j++){
				row.Add(todosNodos.Get(dimSize*i+j));
			}
			PuenteHelper(row, nodosLan, &puenteConfig);
			c_dispositivos.Add(nodosLan);	
			
			//Conectamos verticalmente
			NodeContainer column; 
			NetDeviceContainer nodosLan2;
			for (int j=0; j<dimSize; j++){
				column.Add(todosNodos.Get(i%dimSize+(i/dimSize)*(dimSize*dimSize)+j*dimSize);
				//column.Add(todosNodos.Get(dimSize*j+i));
			}
			PuenteHelper(column, nodosLan2, &puenteConfig);
			c_dispositivos.Add(nodosLan2);
    	   
			//Conectamos profundamente
			NodeContainer spear; 
			NetDeviceContainer nodosLan3;
			for (int j=0; j<dimSize; j++){
				spear.Add(todosNodos.Get(dimSize*dimSize*j+i));
			}
			PuenteHelper(spear, nodosLan3, &puenteConfig);
			c_dispositivos.Add(nodosLan);
		}

		Ipv4AddressHelper h_direcciones (SUBRED, MASCARA);
		Ipv4InterfaceContainer c_interfaces = h_direcciones.Assign (c_dispositivos);
		Ipv4NixVectorRouting();
	}
	
	/*else if(nDims == 3){
		
		NS_LOG_INFO("BCUBE " << nDims);
		
		// For every one of the nDim-1 fronteer hyperplanes
		for(int i = 0; i < nDims; i++){
			
			// For every element in each hyperplane
			for(int j = 0; j < pow(dimSize, nDims-1); j++){
			
				NS_LOG_INFO("Index = " << j);
				NS_LOG_INFO("Coords = " << j*();
				
			}
			
		}
	}*/
		
	return todosNodos;
	
}