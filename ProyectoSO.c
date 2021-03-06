/**
	* Proyecto de Sistemas Operativos 1-2016
	* Supermercados Facyt
	**/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#define N 9
#define true 1
#define false 0
#define Proveedor 0
#define Comprador 1
#define Supervisor 2
#define Guardia 3
#define Cajera 4
#define Reponedor 5
#define Mercadeo 6
#define Tecnico 7
#define Limpieza 8
#define MAX_RUBROS 900
// Pedido de Rubro
typedef struct
{
	float pedido; // Almacena el Pedido
	float vendido; // Descontador de lo vendido
}pedidoR;

// Categor�a, 0 Hortaliza,1 Bebida,2 Charcuter�a,3 Carnicer�a,4 Pescader�a,5 Enlatado,6 Producto de Limpieza,7 Producto de Aseo Personal,8 Perfume.
typedef pedidoR categoria[8]; // Cada campo indica la cantidad en kg/unidad del rubro. 
// Sem�foros
sem_t modulo[8];

// Globales
categoria rubros,list,anaquel;

// Compartidas
volatile int hpedido = false;
volatile int vacio = true;
volatile int camiones = false;
volatile int open = false;
volatile int close = true;
volatile int llenando = false;
// Cabeceras de M�dulos
void * _ThreadProveedor(void *arg);
void * _ThreadComprador(void *arg);
void * _ThreadSupervisor(void *arg);
void * _ThreadGuardia(void *arg);
void * _ThreadCajera(void *arg);
void * _ThreadReponedor(void *arg);
void * _ThreadMercadeo(void *arg);
void * _ThreadTecnico(void *arg);
void * _ThreadLimpieza(void *arg);
// Cabeceras de los procedimientos de los m�dulos
int _ListadoCierre();
void _LlenarCamiones();
void _LlenarEstante();

// Principal
int main()
{
	int i;
	pthread_t h[8];
	for(i=0;i<N;i++)
	{
		list[i].pedido = list[i].vendido = 0;
		rubros[i].pedido = rubros[i].vendido = 0;
		anaquel[i].pedido = anaquel[i].vendido = 0;
	}
	
	pthread_create(&h[0],NULL,_ThreadProveedor,NULL);	
	pthread_create(&h[1],NULL,_ThreadComprador,NULL);	
	pthread_create(&h[2],NULL,_ThreadSupervisor,NULL);	
	pthread_create(&h[3],NULL,_ThreadGuardia,NULL);	
	pthread_create(&h[4],NULL,_ThreadCajera,NULL);	
	pthread_create(&h[5],NULL,_ThreadReponedor,NULL);	
	pthread_create(&h[6],NULL,_ThreadMercadeo,NULL);	
	pthread_create(&h[7],NULL,_ThreadTecnico,NULL);	
	pthread_create(&h[8],NULL,_ThreadLimpieza,NULL);
	for(i=0;i<N;i++)
		pthread_join(h[i],NULL);
	
	return 0;
}

// M�dulo Supervisor
void * _ThreadSupervisor(void *arg)
{
	while(true)
	{
		puts("-- Modulo Supervisor --");
		
		if(!vacio)
			sem_post(&modulo[Reponedor]);
		
		if(!hpedido && _ListadoCierre())
		{
			// Se emite la orden a los compradores
			puts("Procediendo a Omitir orden de pedido");
			hpedido = true;
			sem_post(&modulo[Comprador]);
		}
		else if(!open && close && hpedido && !llenando) // Si est� cerrado y ya se hizo el pedido
		{
			open = true;
			close = false;
			puts("Dando Orden de Abrir a los Guardias");
			sem_post(&modulo[Guardia]);
		}
		getchar();
	}
}
/** Procedimientos del M�dulo Supervisor **/
// -- Revisa el Listado de Cierre y llena la lista de pedidos
int _ListadoCierre()
{
	int i, pedido = false;	
	puts("Revisando Lista de Cierre");
	for(i=0;i<N;i++)
	{
		if(vacio)
		{
			list[i].pedido = MAX_RUBROS;
			if(i+1==N)
			{
				vacio = false;
				pedido = true;
			}
		}
		else if(rubros[i].vendido<(rubros[i].pedido*0.40)) // Si el rubro actual tiene stock menos del 40%
		{
			list[i].pedido = rubros[i].pedido - rubros[i].vendido; // Se pide lo restante
			pedido = true;
		}
	}
	return pedido;
}

// M�dulo Proveedor
void * _ThreadProveedor(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Proveedor]);
		puts("-- Modulo Proveedor --");
		if(hpedido)
		{
			puts("Pedido Recibido\nLlenando Camiones");
			_LlenarCamiones();
			puts("Camiones Llenos\nEnviando Camiones");
			camiones = true;
			sem_post(&modulo[Comprador]);
		}
		puts("-----------------------\n");
	}
}

/** Procedimientos del m�dulo Proveedor **/
void _LlenarCamiones()
{
	int i;
	for(i=0;i<N;i++)
	{
		rubros[i].pedido = list[i].pedido;
		rubros[i].vendido = list[i].pedido;
	}
}

// M�dulo Comprador
void * _ThreadComprador(void *arg)
{
	int i;
	while(true)
	{
		sem_wait(&modulo[Comprador]);
		puts("-- Modulo Comprador --");
		if(hpedido && !camiones)
		{
			// Hacer pedido al Proveedor
			puts("Haciendo Pedido");
			sem_post(&modulo[Proveedor]);
		}
		else if(camiones)
		{
			puts("Camiones Recibidos\nDescargando Mercancia");
			for(i=0;i<N;i++)
				anaquel[i].pedido = rubros[i].pedido;
			camiones = false;
		}
		puts("-----------------------\n");
	}
}

// M�dulo Guardia
void * _ThreadGuardia(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Guardia]);
		puts("-- Modulo Guardia --");
		if(open && !close)
			puts("Abriendo el Supermercado\nSupermercado Abierto");
		else if(!open && close)
			puts("Cerrando el Supermercado\nSupermercado Cerrado");
		puts("-----------------------\n");
	}
}

// M�dulo Cajera
void * _ThreadCajera(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Cajera]);
		puts("Cajera");
	}
}

// M�dulo Reponedor
void * _ThreadReponedor(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Reponedor]);
		puts("-- M�dulo Reponedor --");
		llenando = true;
		_LlenarEstante();
		puts("-----------------------\n");
		llenando = false;
	}
}

void _LlenarEstante()
{
	int restante,i=0; 
	puts("Revisando Anaqueles\n");
	while(i<N && anaquel[i].vendido < 20 && anaquel[i].pedido > 0)
	{
		printf("Anaquel %d\n",i);
		printf("\tLlenando Estante %d\n",i);
		restante = (20-anaquel[i].vendido);
		if(anaquel[i].pedido >= restante)
		{
			anaquel[i].vendido = anaquel[i].vendido + restante;
			anaquel[i].pedido = anaquel[i].pedido - restante;
			printf("\t-- Se colocaron %d productos\n",restante);
			if(anaquel[i].vendido < 10) // Si el estante tiene menos de la mitad de su capacidad
				printf("\tTiene esp�cio para %.1f productos mas\n",20-anaquel[i].vendido);
		}
		else
			printf("\t**No hay productos para llenar el estante**\n");
		i++;
	}
}

// M�dulo Mercadeo
void * _ThreadMercadeo(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Mercadeo]);
	}
}

// M�dulo Tecnico
void * _ThreadTecnico(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Tecnico]);
	}
}

// M�dulo Limpieza
void * _ThreadLimpieza(void *arg)
{
	while(true)
	{
		sem_wait(&modulo[Limpieza]);
	}
}#   p r o y e c t o S 0 2 0 1 6  
 