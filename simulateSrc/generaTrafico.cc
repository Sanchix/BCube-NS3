
#include "generaTrafico.h"
NS_LOG_COMPONENT_DEFINE ("Genera_Trafico");

Llamada *generaTrafico (NodeContainer nodos_BCube, bool ControlaTrafico, uint32_t PorcentajeTrafico){
  
  NS_LOG_FUNCTION ("Contenedor con " << nodos_BCube.GetN() << " nodos");

  // Creamos la Aplicacion UdpServer (sumidero) 
  UdpServerHelper H_ServerUdp (PUERTO); 
  ApplicationContainer C_App_Sumidero = H_ServerUdp.Install (nodos_BCube);

  // Creamos las llamadas
  NS_LOG_INFO("Se planifican las Llamadas");
  return new Llamada(nodos_BCube,30.0, 140.0,ControlaTrafico,PorcentajeTrafico);

}
