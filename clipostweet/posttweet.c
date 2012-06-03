/*
 * ===============================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:            esta función es llamada por cliposttweet (ver cliposttweet.c)
 * Funcion:        lee el tweet introducido por entrada estandar de tambuffer en
 *                 tambuffer bytes y lo transmite por la conexion TCP.
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          26-11-09
 * =================================================================================
 */

#include "ttg.h"

extern int errno;

int
posttweet(int sockfd, char *buffer, int tambuffer)	{
	int n, nleft;
	char * ptr, error[81];

	fprintf(stdout, "\nIntroduce tu tweet (teclea CTRL-D para terminar):\n");
	fprintf(stdout, "----------------------------------------------------------\n");

	/*Bucle principal: Recoje lo escrito por entrada estandar y lo manda de tambuffer en tambuffer bytes*/
	while(1)	{
		if( (fgets(buffer,tambuffer+1,stdin)) == NULL)	{ //Hemos puesto tambuffer+1 porque fgets manda tambuffer-1
			/*fgets devuelve NULL cuando se produce algún error
			 * pero también cuando se lee EOF (Ctrl-D)
			   Por tanto, vamos a ver en qué caso estamos:*/
			if(feof(stdin) == 0) {
				perror(error);
				return -1;
			} else return 0;
		
		}
	
		/*Escritura de Mensaje*/
		ptr = buffer;
		nleft = strlen(buffer);
		while ( (n = write(sockfd, ptr, nleft)) < nleft )	{
			if(n < 0)	{
				fprintf(stderr,"Error de write()\n");
				perror(error);
				return -1;
			} else if(n < nleft) {
				/* Si no fue posible enviar todo el mensaje 
				 * de una vez, se transmite el resto */
				ptr += n;
				nleft -=n;
			}
		}
		
	}

}
