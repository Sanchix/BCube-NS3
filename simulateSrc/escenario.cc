
#include "escenario.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE("Escenario");


double escenario(StageConfig_t *config){
	
	// Eliminar cualquier variable o elemento de red de una simulación anterior.
	double returnValue = 0;
	Simulator::Destroy();
	
	TopologyElements_t topology;
	double nNodosDim = config->nNodos;
	if(config->bCubeLevel>0){
		nNodosDim = ceil(pow(config->nNodos, 1/(double)(config->bCubeLevel+1)));
	}
	NS_LOG_INFO("Generating topology for " << nNodosDim << " nodes in each one of the " << config->bCubeLevel + 1 << " dims (total " << config->nNodos << ")");
	topologiaFisica(config->bCubeLevel, nNodosDim, topology, &(config->puenteConfig));
	NS_LOG_INFO("Topology generated");
	NS_LOG_DEBUG("El numero de equipos es: "<< topology.nodes.GetN());

	asignaDirecciones(topology, config->bCubeLevel+1, nNodosDim);
	Llamada *llamada = generaTrafico(topology.nodes,config->ControlaTrafico,config->PorcentajeTrafico);
	llamada = llamada;

	//AppOnOff Obs_OnOff = llamada->GetObserver();
	

	Simulator::Stop(Seconds(STOPTIME));
	NS_LOG_INFO("RUN");
	Simulator::Run();
	
	Retardo obs = llamada->GetObserver();
	NS_LOG_INFO("El retardo medio de las comunicaciones es: "<< obs.RetardoMedio().ToDouble(ns3::Time::Unit::MS)<<"ms");

	for (int j = 0; j < config->nNodos;j++){
		NS_LOG_DEBUG("El nodo "<<j<< " ha recibido "<< topology.nodes.Get(j)->GetApplication(0)->GetObject<UdpServer>()->GetReceived() <<" paquetes");
	}
	//NS_LOG_DEBUG ("El observador nos da este ultimo tiempo: " << Obs_OnOff.getTUltPaq());
	NS_LOG_INFO("Porcentaje de paquetes pedidos: " << obs.PorcentajePerdidaPaqs());
	if(config->ret == perdidas){
		returnValue = obs.PorcentajePerdidaPaqs();
	}
	else if(config->ret == delay){
		returnValue = obs.RetardoMedio().ToDouble(ns3::Time::Unit::MS);;
	}
	return returnValue;
	
}


void asignaDirecciones(TopologyElements_t &topology, int nDim, int dimSize){
	
	Ipv4NixVectorHelper h_Nix_Routing;
	
	InternetStackHelper h_pila;
	h_pila.SetRoutingHelper(h_Nix_Routing);
	h_pila.SetIpv6StackInstall (false);
	h_pila.Install (topology.nodes);
	NS_LOG_INFO("Internet stack installed");
	
	Ipv4AddressHelper h_direcciones (SUBRED, MASCARA);
	Ipv4InterfaceContainer c_interfaces = h_direcciones.Assign (topology.devices);
	NS_LOG_INFO("Direcciones colocadas");
	
	Ipv4NixVectorRouting IPv4NixVectorRouting = Ipv4NixVectorRouting();
	IPv4NixVectorRouting.SetBCubeParams(nDim, dimSize);

	// Generación de rutas en un fichero para debug
	OutputStreamWrapper Outputwrapper = OutputStreamWrapper("NixVector12.txt",std::ios::out);
	IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(1)->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(),&Outputwrapper,Time::MS);
	//IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(1)->GetObject<Ipv4L3Protocol>()->GetAddress(2, 0).GetLocal(),&Outputwrapper,Time::MS);
	//IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(1)->GetObject<Ipv4L3Protocol>()->GetAddress(3, 0).GetLocal(),&Outputwrapper,Time::MS);
	IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(2)->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(),&Outputwrapper,Time::MS);
	//IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(1),topology.nodes.Get(3)->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(),&Outputwrapper,Time::MS);
	IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(3)->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(),&Outputwrapper,Time::MS);
	//IPv4NixVectorRouting.PrintRoutingPath(topology.nodes.Get(0),topology.nodes.Get(2)->GetObject<Ipv4L3Protocol>()->GetAddress(2, 0).GetLocal(),&Outputwrapper,Time::MS);
	
	// Trazas
	for (uint32_t i = 0; i<topology.nodes.GetN(); i++){
		Ptr<Node> aux = topology.nodes.Get(i);
		NS_LOG_DEBUG("ID: "<<aux->GetId() << "  IP:");
		
		Ptr<Ipv4L3Protocol> L_IP = aux->GetObject<Ipv4L3Protocol> ();
		NS_LOG_DEBUG("El numero de interfaces es: "<< L_IP->GetNInterfaces());
		for (uint32_t j=0; j< L_IP->GetNInterfaces() ;j++){
			NS_LOG_DEBUG( L_IP->GetAddress (j, 0).GetLocal () );
		}
	}
	
}


void topologiaFisica(int bCubeLevel, int dimSize, TopologyElements_t &topology, PuenteConfig_t *puenteConfig){
	
	int nDims = bCubeLevel+1;
	int numEquipos = pow(dimSize,nDims);
	NS_LOG_INFO("NumEquipos = " << numEquipos);
	
	topology.nodes = NodeContainer(numEquipos);

			
	NS_LOG_INFO("BCUBE " << bCubeLevel);
		
	// For every dim
	for(int i = 0; i < nDims; i++){
		NS_LOG_INFO("For dim " << i);
		
		// For every switch
		for(int j = 0; j < pow(dimSize, nDims-1); j++){
			
			std::string cstr = "Coords: ";
			
			// Generate coordinates of base element
			int indexBase = 0;
			for(int k = 0; k < nDims-1; k++){
				int c = (j/(int)pow(dimSize,k))%dimSize;
				indexBase += coordToIndex(dimSize, (k+1+i)%nDims, c);
				cstr += "\t" + std::to_string(c);
			}
			NS_LOG_INFO(cstr + "\tbase: " + std::to_string(indexBase));
			
			// Compose line and create bridge
			NodeContainer line;
			NetDeviceContainer nodosLan;
			for(int k = 0; k < dimSize; k++){
				int index = indexBase+coordToIndex(dimSize, i, k);
				line.Add(topology.nodes.Get(index));
			}
			NS_LOG_INFO(line.GetN());
			PuenteHelper(line, nodosLan, puenteConfig);
			topology.devices.Add(nodosLan);	
			
		}
		
	}
	
}


int coordToIndex(int dimSize, int dim, int coord){
	int index = -1;
	if(coord < dimSize && coord >= 0){
		index = pow(dimSize, dim)*coord;
	}
	return index;
}
