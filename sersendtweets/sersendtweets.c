/*
 * ===============================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:            sersendweets <puerto> <backlog> <tambuffer>
 * Funcion:        servidor de envio de tweets.
 *                 Espera conexiones de clientes en el puerto <puerto>.
 *		   Una vez realiza una conexion, comprueba que el topico existe
 *		   y llama a la funcion int sendtweet.
 *		   Si sendtweet no ha podido enviar el topico, sersendtweet envia
 *                 un mensaje por la conexion informando al cliente del fracaso en la ejecucion.
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          2-12-09
 * =================================================================================
 */

#include "ttg.h"
#define MAXTOPICO 9

extern int errno;

main(int argc, char **argv)
{
	int			sockfd, connsockfd;
	int			len, n, nleft, encontrado, tam;
	int			backlog, tambuffer, salida;
	char			*buffer, *topico, *ptr, *p,*endptr, error[81];
	struct sockaddr_in	servaddr;
	struct sockaddr_in	clieaddr;
	in_port_t		puerto;
	long			puerto_l, backlog_l, tambuffer_l;

	/*Comprobacion de argumentos de entrada*/
	/* Los argumentos de entrada son:
		- Numero de puerto de escucha
		- backlog
		- tambuffer
	*/
	if (argc != 4)	{
		fprintf(stderr,"Uso:  serstoretweets <puerto> <backlog> <tambuffer>\n");
		exit(1);
	}


        /*Comprobacion de la validez del puerto introducido*/
        /*(si hay algun caracter no numerico, endptr apunta al primero de ellos,
        lo cual implica que si la cadena apuntada por endptr no tiene longitud 0
        es porque se ha introducido un caracter no numerico)*/
        puerto_l = strtol(argv[1],&endptr,10);
        if(strlen(endptr)!=0)   {
                fprintf(stderr,"Numero de puerto incorrecto (el puerto ha de ser numerico)\n");
                exit(1);
        }
        if( (puerto_l <= 0) || (puerto_l > 65535) || (errno==ERANGE) )  {
                fprintf(stderr,"Numero de puerto fuera de rango (1-65535)\n");
                exit(1);
        }
        puerto = (in_port_t) puerto_l;
        fprintf(stdout,"Puerto: %d\n",puerto);


        /*Comprobacion de la validez del backlog introducido*/
        /*(si hay algun caracter no numerico, endptr apunta al primero de ellos,
        lo cual implica que si la cadena apuntada por endptr no tiene longitud 0
        es porque se ha introducido un caracter no numerico)*/
        backlog_l = strtol(argv[2],&endptr,10);
        if(strlen(endptr)!=0)   {
                fprintf(stderr,"Numero de backlog incorrecto (el backlog ha de ser numerico)\n");
                exit(1);
        }
        if( (backlog_l < 0) || (backlog_l> 100) || (errno==ERANGE) )  {
                fprintf(stderr,"Tamano de backlog fuera de rango (1-100)\n");
                exit(1);
        }
        backlog = (int) backlog_l;
        fprintf(stdout,"Backlog: %d\n",backlog);


        /*Comprobacion de la validez del buffer introducido*/
        /*(si hay algun caracter no numerico, endptr apunta al primero de ellos,
        lo cual implica que si la cadena apuntada por endptr no tiene longitud 0
        es porque se ha introducido un caracter no numerico)*/
        tambuffer_l = strtol(argv[3],&endptr,10);
        if(strlen(endptr)!=0)   {
                fprintf(stderr,"Tamano de buffer incorrecto (el tamano ha de ser numerico)\n");
                exit(1);
        }
        if( (tambuffer_l <= 1) || (tambuffer_l > 100) || (errno==ERANGE) )  {
                fprintf(stderr,"Numero de buffer fuera de rango (2-100)\n");
                exit(1);
        }
        tambuffer = (int) tambuffer_l;
        fprintf(stdout,"Buffer: %d\n",tambuffer);

	/*Reserva de memoria para buffer y topico*/
	buffer = malloc(tambuffer);
        if(errno!=0)    {
                fprintf(stderr,"Error de malloc en al reservar memoria\n");
                exit(1);
        }
	topico = malloc(MAXTOPICO); 
        if(errno!=0)    {
                fprintf(stderr,"Error de malloc en al reservar memoria\n");
                exit(1);
        }

	/*Creacion de nuevo socket*/
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		fprintf(stderr,"Error de Socket\n");
		exit(1);
	}


	/*Inicializacion de estructura de direccion*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;		/*Internet*/
	servaddr.sin_port   = htons(puerto);	/*Puerto de "escucha"*/
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Binding...*/
	if( bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)	{
		perror(error);
		fprintf(stderr, "Error de bind()");
		exit(1);
	}


	/*Listen...*/
	if( listen(sockfd, backlog) != 0)	{
		fprintf(stderr, "Error de listen()");
		exit(1);
	}
	fprintf(stdout,"Escuchando peticiones de conexion...\n");


	/*Bucle principal del programa:
		- Acepta peticiones
		- Lee de tambuffer en tambuffer el topico hasta detectar \n
		- LLamada a funcion sendtweet
		- Cierre*/
	while(1)	{
		len = sizeof(clieaddr);
		if( (connsockfd = accept(sockfd, (struct sockaddr *) &clieaddr,  &len)) < 0)	{
			perror(error);
			fprintf(stderr, "Error de accept");
			exit(1);
		}
		printf("SOCKET: %d\n",connsockfd);
		fprintf(stdout,"Conexion recibida de: %s, puerto %d\n",
				inet_ntoa(clieaddr.sin_addr),
				ntohs(clieaddr.sin_port));

		//Calculamos el minimo entre tambuffer y el tope del topico que es de 9 caracteres
		if(tambuffer>9) nleft=9;
		else nleft=tambuffer;

		//Recojo los datos de tambuffer en tambuffer bytes hasta leer /n
		ptr=topico;
		encontrado=0;
		do {
			if ( (n = read(connsockfd, ptr, nleft)) > 0 ) {
				if ( (p = (char*) memchr (ptr, '\n',n)) != NULL)  {
					//encontrado el \n, lo cambio por \0
					*p= '\0';
					encontrado = 1;
				}
				else if(n<nleft){ 
					//seguimos leyendo
					ptr += n;
					nleft -= n;
				} else if (n==nleft) {
					/* Siguientes tambuffer bytes*/
					ptr += n;
				}			
			}	
 		} while ( (n>0) && (nleft>0) && (encontrado==0) );

		if (n < 0) {
			fprintf(stderr,"Error de read()");
			perror(error);
			exit(1);
		}		
		// Ha habido un error en la lectura del topico
 		else if (n == 0 | encontrado==0) {
			ptr = "sersendtweets: Nombre de topico incorrecto\n";
			tam = strlen(ptr);
			if(tambuffer > tam) nleft = tam;
			else nleft = tambuffer;
			while ( (n = write(connsockfd, ptr, nleft)) < tam )	{
				if(n < 0)	{
					fprintf(stderr,"Error de write()\n");
					perror(error);
					exit(1);
				} else if(n < nleft) {
					/* Si no fue posible enviar todo el mensaje 
					* de una vez, se transmite el resto */
					ptr += n;
					nleft -=n;
				} else if(n == nleft) {
					ptr += n;
					tam -=n;
					if(tam<nleft) {
						nleft = tam;			
					}
				}
			}
			fprintf(stderr,"Error en la lectura del topico\n");
 		}
		 
 		else { //Todo ha ido bien

			/*Comprobamos la salida de la funcion de almacenar e informamos al cliente de tambuffer en tambuffer bytes*/
			if((salida=sendtweet(connsockfd, topico, buffer, tambuffer)) != 0) {
				ptr = "sersendtweet: Error en el envio del tweet\n";
				fprintf(stdout, "Ha habido un problema a la hora de enviar el tweet\n");
				tam = strlen(ptr);
				if(tambuffer > tam) nleft = tam;
				else nleft = tambuffer;
				while ( (n = write(connsockfd, ptr, nleft)) < tam )	{
					if(n < 0)	{
						fprintf(stderr,"Error de write()\n");
						perror(error);
						exit(1);
					} else if(n < nleft) {
						/* Si no fue posible enviar todo el mensaje 
						* de una vez, se transmite el resto */
						ptr += n;
						nleft -=n;
					} else if(n == nleft) {
						ptr += n;
						tam -=n;
						if(tam<nleft) {
							nleft = tam;			
						}
					}
				}
			}
		}
		/*Y se cierra la conexion con el cliente*/
		sleep(1);
		close(connsockfd);
		fprintf(stdout,"Conexion cerrada\n\n");

	}
	
}
