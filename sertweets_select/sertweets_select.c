/*
 * ===============================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:            serstoretweets <puerto> <backlog> <tambuffer>
 * Funcion:        servidor de almacenamiento de tweets.
 *                 Espera conexiones de clientes en el puerto <puerto>.
 *		   Una vez realizada la conexión llama a la funcion int storetweet.
 *		   Envía un mensaje por la conexión creada para alertar al cliente
 *                 del exito o fracaso del almacenamiento del tweet.
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          26-11-09
 * =================================================================================
 */

#include "ttg.h"
#define MAXTOPICO 9

extern int errno;

/*Cuando el 'hijo' acabe, manda una señal al kernel y el padre debe ejecutar wait para que el proceso 'hijo' no se quede como un proceso zoombie*/
void tratamiento (int signalid)	{

	pid_t pid;
	int stat;

	pid=wait(&stat);
	return;
}

main(int argc, char **argv)
{
	fd_set			conjunto_descriptores;
	int			sockfd1, sockfd2, maxfd, connsockfd;
	int			len, n, nleft, encontrado, pid, tam;
	int			backlog, tambuffer, salida;
	char			*buffer, *topico, *ptr, *p,*endptr, error[81];
	struct sockaddr_in	servaddr1, servaddr2;
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

	/*Reserva de memoria*/
	buffer = malloc(tambuffer);
        if(errno==ENOMEM)    {
                fprintf(stderr,"Error de malloc en al reservar memoria\n");
                exit(1);
        }
	topico = malloc(9); 
        if(errno==ENOMEM)    {
                fprintf(stderr,"Error de malloc en al reservar memoria\n");
                exit(1);
        }

	/*Creacion de nuevo socket1 --> serstoretweets*/
	if ( (sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		fprintf(stderr,"Error de Socket para serstoretweets\n");
		exit(1);
	}
	/*Creacion de nuevo socket2 --> sersendtweets*/
	if ( (sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		fprintf(stderr,"Error de Socket para sersendtweets\n");
		exit(1);
	}

	/*Inicializacion de estructura de direccion para serstoretweets*/
	bzero(&servaddr1, sizeof(servaddr1));
	servaddr1.sin_family = AF_INET;		/*Internet*/
	servaddr1.sin_port   = htons(puerto);	/*Puerto de "escucha"*/
	servaddr1.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Inicializacion de estructura de direccion para sersendtweets*/
	bzero(&servaddr2, sizeof(servaddr2));
	servaddr2.sin_family = AF_INET;		/*Internet*/
	servaddr2.sin_port   = htons(puerto+1);	/*Puerto de "escucha" para puerto+1*/
	servaddr2.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Binding... para serstoretweets*/
	if( bind(sockfd1, (struct sockaddr *) &servaddr1, sizeof(servaddr1)) != 0)	{
		perror(error);
		fprintf(stderr, "Error de bind() para serstoretweets");
		exit(1);
	}

	/*Binding... para sersendtweets*/
	if( bind(sockfd2, (struct sockaddr *) &servaddr2, sizeof(servaddr2)) != 0)	{
		perror(error);
		fprintf(stderr, "Error de bind() para sersendtweets");
		exit(1);
	}

	/*Listen... para serstoretweets*/
	if( listen(sockfd1, backlog) != 0)	{
		fprintf(stderr, "Error de listen() para serstoretweets");
		exit(1);
	}

	/*Listen... para sersendtweets*/
	if( listen(sockfd2, backlog) != 0)	{
		fprintf(stderr, "Error de listen() para sersendtweets");
		exit(1);
	}

	fprintf(stdout,"Escuchando peticiones de conexion...\n");



	/*Signal...*/
	if ((signal(SIGCHLD, tratamiento)) == SIG_ERR)	{
		perror("Error de signal()");
		exit(1);
	}

	/*Bucle principal del programa*/
	while(1)	{

		/*FD_SET para 'observar'. Antes un FD_ZERO para vaciar valores*/
		FD_ZERO(&conjunto_descriptores);
		FD_SET(sockfd1, &conjunto_descriptores);
		FD_SET(sockfd2, &conjunto_descriptores);

		/*Calculamos el numero de descriptor mas alto*/
		if(sockfd1>sockfd2) maxfd=sockfd1+1;
		else maxfd=sockfd2+1;

		/*select(). El socket se quedara a la escucha a la espera de conexiones*/
		do {
			errno = 0;
			select(maxfd, &conjunto_descriptores, NULL, NULL, NULL); 
		} while(errno == EINTR);
		if(errno != 0) {
			perror(error);
			exit(1);
		}
		/*Ahora hay que comprobar de que evento se trata*/
		if(FD_ISSET(sockfd1, &conjunto_descriptores)) {
			len = sizeof(clieaddr);
			/*Hay que tener cuidado con el valor EINTR de errno. No hay ningun error*/
			do {
				errno = 0;
				connsockfd = accept(sockfd1, (struct sockaddr *) &clieaddr,  &len);
			} while(errno == EINTR);
			if( errno != 0)	{
				fprintf(stderr, "Error de accept\n");
				exit(1);
			}
			/*Desdoblamos en proceso 'hijo' y proceso 'padre'*/
			pid=fork();
			if(pid==-1) {
				fprintf(stderr, "Error en la creacion del proceso\n");
				exit(1);
			} else {
				if(pid != 0) { //Proceso padre, devuelve el PID del 'hijo'
					close(connsockfd);						
				} else { //Proceso hijo
					close(sockfd1);
					printf("SOCKET: %d\n",connsockfd);
					fprintf(stdout,"Conexion recibida de: %s, puerto %d\n",
							inet_ntoa(clieaddr.sin_addr),
							ntohs(clieaddr.sin_port));


					//Calculamos el minimo entre tambuffer y el tope del topico que es de 9 caracteres
					if(tambuffer>MAXTOPICO) nleft=MAXTOPICO;
					else nleft=tambuffer;

					//Recojo los datos de tambuffer en tambuffer bytes hasta que encuentre el \n
					ptr=topico;
					encontrado = 0;
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
					} else if (n == 0 | encontrado==0) {
						ptr = "serstoretweets: Nombre de topico incorrecto\n";
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
			 		else { 
						salida=storetweets(connsockfd, topico, buffer, tambuffer);
						/*Comprobamos la salida de la funcion de almacenar e informamos al cliente */
						if (salida!=0) { 
							ptr = "serstoretweets: Error en el almacenamiento del tweet\n";
							fprintf(stdout, "Ha habido un problema a la hora de almacenar el tweet enviado\n");
						}
						else {
							ptr = "serstoretweets: tweet almacenado con exito";
							fprintf(stdout, "La direccion %s ha escrito un tweet en el topico '%s'\n", inet_ntoa(clieaddr.sin_addr), topico);
	
						}
	
						/*Informamos al cliente mandando lo que hay en buffer por la conexion de tambuffer en tambuffer bytes*/
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
		
					/*Cerramos el cliente y terminamos el proceso hijo*/
					sleep(1);
					close(connsockfd);
					fprintf(stdout,"storetweet: Conexion terminada con %s, puerto %d\n", inet_ntoa(clieaddr.sin_addr),ntohs(clieaddr.sin_port));
					exit(0);
				
				}
			}
		}
		if(FD_ISSET(sockfd2, &conjunto_descriptores)) {
			len = sizeof(clieaddr);
			/*Hay que tener cuidado con el valor EINTR de errno. No hay ningun error*/
			do {
				errno = 0;
				connsockfd = accept(sockfd2, (struct sockaddr *) &clieaddr,  &len);
			} while(errno == EINTR);
			if( errno != 0)	{
				fprintf(stderr, "Error de accept\n");
				exit(1);
			}
			/*Desdoblamos en proceso 'hijo' y proceso 'padre'*/
			pid=fork();
			if(pid==-1) {
				fprintf(stderr, "Error en la creacion del proceso\n");
				exit(1);
			} else {
				if(pid != 0) { //Proceso padre, devuelve el PID del 'hijo'
					close(connsockfd);						
				} else { //Proceso hijo
					close(sockfd2);	
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


					/*Cerramos el cliente y terminamos el proceso hijo*/
					sleep(1);
					close(connsockfd);
					fprintf(stdout,"sendtweet: Conexion terminada con %s, puerto %d\n", inet_ntoa(clieaddr.sin_addr),ntohs(clieaddr.sin_port));
					exit(0);
				}
			}		
		}

	}	
}
