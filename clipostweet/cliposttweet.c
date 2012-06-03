/*
 * =========================================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:            cliposttweet <direccionIP> <puerto> <tambuffer> <topico>
 * Funcion:        cliente de envio de tweets.
 *                 Realiza conexiones a un servidor <direccionIP> por el puerto <puerto>
 *		   Envía el <topico> por la conexión TCP y llama a la función int posttweet
 *                 Hace un cierre parcial a la espera de datos desde el otro extremo.
 *                 Extrae lo recibido por la conexión TCP y lo muestra por pantalla
 *                 informando al usuario del éxito o fracaso del envio y almacenamiento del tweet.
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          26-11-09
 * =========================================================================================
 */


#include "ttg.h"

extern int errno;

int
main(int argc, char **argv)	{
	int			sockfd, n, nleft, tambuffer, longtopico, salida;
	char			*buffer, * ptr, *endptr, topico[10], error[81];
	struct sockaddr_in	servaddr;	
	long			puerto_l, tambuffer_l;
	in_port_t		puerto;

	//Comprobamos que se han introducido todos los datos
	if (argc != 5)	{
		fprintf(stderr,"Uso: cliposttweet <direccionIP> <puerto> <buffer> <topico>\n");
		exit(1);
	}

        /*Comprobacion de la validez del puerto introducido
	(usando strtol, si hay algun caracter no numerico, endptr apunta al primero de ellos,
	lo cual implica que si la cadena apuntada por endptr no tiene longitud 0
        es porque se ha introducido un caracter no numerico)*/
	puerto_l = strtol(argv[2],&endptr,10);
	if(strlen(endptr)!=0)   {
		fprintf(stderr,"Numero de puerto incorrecto (el puerto ha de ser numerico)\n");
                exit(1);
        }
        if( (puerto_l <= 0) || (puerto_l > 65535) || (errno==ERANGE) )  {
		fprintf(stderr,"Numero de puerto fuera de rango (1-65535)\n");
		exit(1);
	}
	//Convertimos long a tipo in_port_t
        puerto = (in_port_t) puerto_l;
        fprintf(stdout,"Puerto: %d\n",puerto);



	/*Comprobacion de la validez del buffer introducido*/
	/*(si hay algun caracter no numerico, endptr apunta al primero de ellos,
	 *lo cual implica que si la cadena apuntada por endptr no tiene longitud 0
	 *es porque se ha introducido un caracter no numerico)*/
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


	/* Comprobación del tamaño del topico. Menor de 8 caracteres*/
	strcpy(topico,argv[4]);
	longtopico = strlen(topico);
	if(longtopico > 8 || longtopico < 1)	{
		fprintf(stderr, "Tamano de caracteres del topico incorrecto (1-8)\n ");
		exit(1);
 	}
	printf("****************\n");
	printf("*Topico: %s", topico);
	printf("\n****************\n");

	/*Reserva de memoria*/
	buffer = malloc(tambuffer);
        if(errno!=0)    {
                fprintf(stderr,"Error de malloc en al reservar memoria\n");
                exit(1);
        }



	/*Creacion de nuevo socket*/
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		fprintf(stderr,"Error de Socket, %d\n", errno);
		perror(error);
		exit(1);
	}

	/*Inicializacion de estructura de direccion*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;		
	servaddr.sin_port   = htons(puerto);	
	/*Conversion dotted-decimal a 32 bits*/
	if ( (servaddr.sin_addr.s_addr = inet_addr(argv[1])) == -1)	{
		fprintf(stderr,"Error de inet_addr() para %s\n",argv[1]);
		perror(error);
		exit(1);
	}

	/*Establecimiento de conexion*/
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)	{
		fprintf(stderr,"Error de connect()\n");
		perror(error);
		exit(1);
	}


		/*Preparamos los datos para que se puedan enviar sin problemas*/

	topico[longtopico] = '\n'; //Anadimos un \n, sustituyendo el \0 que hay
	topico[longtopico+1] = '\0'; //Anadimos el \0 para que el strlen lea hasta ahi
	ptr = topico;
	longtopico = strlen(topico);

	/*Bucle principal del programa:
		1. Comprueba si la longitud del topico es mayor que el tambuffer
		2. Si es mayor, envia de tambuffer en tambuffer bytes el topico
		3. Si es menor envia el topico de una vez*/

	if (longtopico > tambuffer) nleft=tambuffer;
	else nleft=longtopico;
	while ( (n = write(sockfd, ptr, nleft)) < longtopico )	{
		if(n < 0)	{
			fprintf(stderr,"Error de write()\n");
			perror(error);
			exit(1);
		} else if (n<nleft) {
			/* Si no fue posible enviar todo el mensaje 
			 * de una vez, se transmite el resto */
			ptr += n;
			nleft -=n;	
		} else if (n==nleft) {
			/* Siguientes tambuffer bytes*/
			ptr += n;
			longtopico -=n;
			if (longtopico<nleft) {
				nleft = longtopico; //En caso de que queden menos bytes por mandar que tambuffer

			} 
		}
	}

	/*Llamamos a la funcion posttweet para que envie el mensaje*/
	if((salida = posttweet(sockfd, buffer, tambuffer)) != 0) {
		fprintf(stdout, "----------------------------------------------------------\n");
		fprintf(stderr, "Error en el envio :(\n");
		exit(1);
	}
	else {
		fprintf(stdout, "----------------------------------------------------------\n");
		fprintf(stdout, "Tweet enviado correctamente ;) A la espera de ser almacenado...\n");
	}

	/*Cierre parcial de la conexion*/
	if(shutdown(sockfd,1) < 0) {
		perror(error);
		exit(1);
	}

	/*Leo lo que me mande el servidor hasta detectar un FIN*/
	while((n=read(sockfd, buffer, tambuffer)) != 0)	{
		if(n<1)	{
			perror(error);
			printf("Error en lectura de socket.\n");
			exit(1);
		} else	{
			buffer[n]=0;
			fprintf(stdout, "%s",buffer);
		}
	}	
	fprintf(stdout, "\n\n");
	exit(0);

}

