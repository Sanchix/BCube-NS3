#include "llamada.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Llamada");


Llamada::Llamada(NodeContainer nodos, double duracion, double max_t_inicio, bool ControlaTrafico, uint32_t PorcentajeTrafico){
	
	NS_LOG_FUNCTION("Entramos en el constructor Llamada: ");
	NS_LOG_FUNCTION("La duracion media de las llamadas será: "<< duracion);
	NS_LOG_FUNCTION("El máximo tiempo en el que podrá comenzar una llamada: "<< max_t_inicio);
	
	if (ControlaTrafico){
		NS_LOG_FUNCTION("El PorcentajeTrafico maximo será: "<< PorcentajeTrafico<<"%");		
	}

	this->PorcentajeTrafico = PorcentajeTrafico;
	this->ControlaTrafico = ControlaTrafico;

	TasaApp = DataRate("32kb/s");
	TamPack = 92;

	double tiempoAppOn = 0.350;
	double tiempoAppOff = 0.650;
	Exp_ON = CreateObject<ExponentialRandomVariable> ();
	Exp_OFF = CreateObject<ExponentialRandomVariable> ();
	Exp_ON->SetAttribute ("Mean", DoubleValue (tiempoAppOn));
	Exp_OFF->SetAttribute ("Mean", DoubleValue (tiempoAppOff));

	Exp_0 = CreateObject<ConstantRandomVariable> ();
	Exp_1 = CreateObject<ConstantRandomVariable> ();
	Exp_0->SetAttribute ("Constant", DoubleValue (0.0));
	Exp_1->SetAttribute ("Constant", DoubleValue (1.0));
	
	//NS_LOG_DEBUG("Tam pack: " << TamPack);


	TodosNodos = nodos;
	nodeCalledList = new std::vector<int> (TodosNodos.GetN(),int(LIBRE));
	nNodesInCall = 0;
	
	Uniform_t_inicio = CreateObject<UniformRandomVariable> ();
	Uniform_t_inicio->SetAttribute ("Min", DoubleValue (0.0));
	Uniform_t_inicio->SetAttribute ("Max", DoubleValue (max_t_inicio));

	Exp_duracion = CreateObject<ExponentialRandomVariable> ();
	Exp_duracion->SetAttribute ("Mean", DoubleValue (duracion));

	Uniform_equipo_destino = CreateObject<UniformRandomVariable> ();
	Uniform_equipo_destino->SetAttribute ("Min", DoubleValue (0.0));
	Uniform_equipo_destino->SetAttribute ("Max", DoubleValue (double(TodosNodos.GetN()-1)));

	Time t_inicio;
	for(uint32_t i = 0; i<TodosNodos.GetN(); i++){
		t_inicio = Seconds(int64x64_t(Uniform_t_inicio->GetInteger()));	
        Simulator::Schedule(t_inicio, &Llamada::Call, this, TodosNodos.Get(i));
	}
	
	NS_LOG_DEBUG("Se crea el observador");
	obs_ret.AddServers(TodosNodos);
	
}

void Llamada::Call(Ptr<Node> nodo_origen){
	
	NS_LOG_FUNCTION("CALL - El id del nodo llamante: "<< nodo_origen->GetId());
	NS_LOG_DEBUG("El estado del nodo es: "<<nodeCalledList->at(nodo_origen->GetId()));

	uint32_t id_destino = 0;
	int nodosLibres = TodosNodos.GetN()-1-nNodesInCall;	

	// Trazas
	if (nodo_origen->GetNApplications() > 1){
		TimeValue t_parada;
		nodo_origen->GetApplication(nodo_origen->GetNApplications() - 1)->GetAttribute("StopTime", t_parada);
		NS_LOG_DEBUG ("Valor del tiempo de parada del nodo " << nodo_origen->GetId() << ": "<< t_parada.Get());
	}	
	//int i = 0;
	//for (std::vector<int>::iterator it = nodeCalledList->begin(); it != nodeCalledList->end(); ++it){
	//	NS_LOG_DEBUG("El valor del estado del nodo "<<i<<" es "<< *it);
	//	i++;
	//}
	NS_LOG_DEBUG("nNodesInCall: " << nNodesInCall);


	// Si está ocupado -> impaciente
    if(nodeCalledList->at(nodo_origen->GetId()) != LIBRE){
        nodeCalledList->at(nodo_origen->GetId()) = IMPACIENTE;
        // Trazas
		//int i = 0;
		//for (std::vector<int>::iterator it = nodeCalledList->begin(); it != nodeCalledList->end(); ++it){
		//	NS_LOG_DEBUG("El valor del estado del nodo "<<i<<" es "<< *it);
		//	i++;
		//}
    }
	// Si está libre pero no se puede llamar
    else if(nodosLibres <= 0){
			Time t_inicio = Seconds(int64x64_t(Uniform_t_inicio->GetInteger()));
			Simulator::Schedule(t_inicio, &Llamada::Call, this, nodo_origen);
	}
		
	// Si sí se puede llamar
	else{
		
		Uniform_equipo_destino->SetAttribute("Max", DoubleValue((double)(nodosLibres-1)));
		uint32_t id_destino_libre = Uniform_equipo_destino->GetInteger();
	
		// id_destino_libre = 3
		//                |
		// OOXOOOXOOXOOOOOXO
		// index -> 3

		NS_LOG_DEBUG("El id del nodo destino libre: " << id_destino_libre);
		uint32_t index = 0;
		while(index <= id_destino_libre){
			while(nodeCalledList->at(id_destino) != LIBRE || id_destino == nodo_origen->GetId()){
				id_destino++;
			}
			id_destino++;
			index++;
		}
		id_destino--;

		NS_LOG_DEBUG("El id del nodo destino: " << id_destino);

		nodeCalledList->at(nodo_origen->GetId()) = OCUPADO;
		nodeCalledList->at(id_destino) = OCUPADO;
		nNodesInCall += 2;
		
		// CREAR APLICACIONES FUENTES
		Ptr<Node> nodo_destino = TodosNodos.Get(id_destino);
		
		Time t_fin = Seconds(Exp_duracion->GetValue());
		NS_LOG_DEBUG("El tiempo de finalizacíon de la llamada es :" << t_fin);

		Ptr<OnOffApplication> AppOrigen = CreateObject<OnOffApplication>();
		AppOrigen->SetAttribute("OnTime",PointerValue(Exp_ON));
		AppOrigen->SetAttribute("OffTime",PointerValue(Exp_OFF));
		AppOrigen->SetAttribute("PacketSize",UintegerValue(TamPack));
		AppOrigen->SetAttribute("DataRate",DataRateValue(TasaApp));
		AppOrigen->SetAttribute("Remote",AddressValue(InetSocketAddress(nodo_destino->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(), PUERTO)));
		AppOrigen->SetAttribute("StopTime",TimeValue(Simulator::Now()+t_fin));
		nodo_origen->AddApplication(AppOrigen);

		Ptr<OnOffApplication> AppDestino = CreateObject<OnOffApplication>();
		AppDestino->SetAttribute("OnTime",PointerValue(Exp_ON));
		AppDestino->SetAttribute("OffTime",PointerValue(Exp_OFF));
		AppDestino->SetAttribute("PacketSize",UintegerValue(TamPack));
		AppDestino->SetAttribute("DataRate",DataRateValue(TasaApp));
		AppDestino->SetAttribute("Remote",AddressValue(InetSocketAddress(nodo_origen->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0).GetLocal(), PUERTO)));
		AppDestino->SetAttribute("StopTime",TimeValue(Simulator::Now()+t_fin));
		nodo_destino->AddApplication(AppDestino);

		//Se añaden las aplicaciones al observer
		obs_ret.NewCall(AppOrigen->GetObject<OnOffApplication>());
		obs_ret.NewCall(AppDestino->GetObject<OnOffApplication>());
		
		Simulator::Schedule(t_fin, &Llamada::Hang,this, nodo_origen, nodo_destino);
        
    }
	
}


void Llamada::Hang(Ptr<Node> nodo_origen,Ptr<Node> nodo_destino){

	nNodesInCall -= 2;
    uint32_t id_destino = nodo_destino->GetId();
    uint32_t id_origen = nodo_origen->GetId();
    NS_LOG_FUNCTION("HANG - El id del nodo llamante: "<< id_origen);
    NS_LOG_FUNCTION("HANG - El id del nodo llamado: "<< id_destino);
    NS_LOG_FUNCTION("HANG - El porcentaje de trafico actual: "<< 100*nNodesInCall/TodosNodos.GetN());


    // Nueva llamada del origen
    nodeCalledList->at(id_origen) = LIBRE;

    if((ControlaTrafico==true) && (100*nNodesInCall/TodosNodos.GetN() < PorcentajeTrafico)){

    	Time t_inicio = Seconds(int64x64_t(Uniform_t_inicio->GetInteger()));
    	Simulator::Schedule(t_inicio, &Llamada::Call,this, nodo_origen);
    }
    else if(ControlaTrafico==false){

		Time t_inicio = Seconds(int64x64_t(Uniform_t_inicio->GetInteger()));
    	Simulator::Schedule(t_inicio, &Llamada::Call,this, nodo_origen);
	}

    // Nueva llamada del destino (si la quiere)
    if(nodeCalledList->at(id_destino) == IMPACIENTE){
		NS_LOG_INFO("Era impaciente");
        Simulator::Schedule(Seconds(0), &Llamada::Call,this, nodo_destino);
    }
	nodeCalledList->at(id_destino) = LIBRE;
	
}

Retardo Llamada::GetObserver(){
	return obs_ret;
}