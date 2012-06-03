/*
 * ===============================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:		   Esta funcion es llamada por serstoretweets (ver serstoretweets.c).
 * Funcion:	   Abre numtweets del topico pedido y manda el primer byte leido por la conexion
		   Si no existiera el archivo manda un 0 y devuelve un 0 a serstoretweets
		   A continuacion lee un byte de la conexion (el tweet que el cliente quiere leer).
		   Manda dicho tweet por la conexion de tambuffer en tambuffer bytes
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          2-12-09
 * =================================================================================
 */

#include "ttg.h"

extern int errno;

int sendtweet(int sockfd, char *topico, char *buffer, int tambuffer)
{
	int n, descriptor, descriptor2, leido, nleft,a,num;
	long leido_l;
	char fichero[80], *endptr, error[80];



	//Abro el archivo numtweets y leo el primer byte. Lo mando por la conexion
	sprintf(fichero, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s/numtweets", topico);
	if((descriptor = open(fichero,O_RDONLY)) < 0) num = 0;
	else read(descriptor,&num,1);
	
	while ( (n = write(sockfd, &num, 1)) < 1 )	{
		fprintf(stderr,"Error de write()\n");
		perror(error);
		return -1;
	}	
	if(descriptor < 0) return 0;
	
	//Lectura de un byte de la conexion referente al topico que quiere leer. Error si recibe FIN o error.
	while ((n=read(sockfd,buffer,1)) < 1) {
			fprintf(stdout,"Error en la lectura\n");
			return -1;
	}
	buffer[n]=0;
	leido_l = strtol(buffer,&endptr,10);
        if(strlen(endptr)!=0 | leido_l==0)   {
		fprintf(stderr,"El cliente no ha introducido un caracter numerico distinto de cero.\n");
		return -1;
	}
	leido = (int) leido_l;

	//Abrimos el fichero seleccionado
	errno=0;
	sprintf(fichero, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s/%d", topico, leido);
	descriptor2 = open(fichero,O_RDONLY); //modo lectura
	if(errno==ENOENT | descriptor2<0) {
		fprintf(stdout, "Error al abrir el archivo\n");
		perror(error);
		return -1;
	} 

	//Extraemos de tambuffer en tambuffer bytes los datos del fichero seleccionado y los mandamos por la conexion
	while( (n=read(descriptor2, buffer, tambuffer)) != 0)	{
		if(n<0)	{
			printf("Error en lectura\n");
			perror(error);
			return -1;
		} else	{
			nleft = n;
			while ( (n = write(sockfd, buffer, nleft)) < nleft )	{
				if(n < 0)	{
					fprintf(stderr,"Error de write()\n");
					perror(error);
					return -1;
				} else if(n < tambuffer) {
					/* Si no fue posible enviar todo el mensaje 
					 * de una vez, se transmite el resto */
					buffer += n;
					nleft -=n;
				}
			}	
		}
	}

	if(close(descriptor) < 0 | close(descriptor2) < 0) {
		fprintf(stdout, "Error al cerrar los descriptores\n");
		perror(error);
		return -1;
	}
	return 0;
}
