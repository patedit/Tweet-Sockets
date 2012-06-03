/*
 * ===============================================================================
 * Autor:          Sergio Alonso y Rodrigo Mota (ttx23)
 * Compilacion:    make
 * Uso:            esta funcion es llamada por serstoretweets (ver serstoretweets.c)
 * Funcion:        Comprueba la existencia del directorio donde se guardan los topicos y si no existe lo crea.
 * Plataforma:     Compilado y probado en Linux 2.6.26 y 2.6.31
 * Fecha:          26-11-09
 * =================================================================================
 */

#include "ttg.h"

extern int errno;

int storetweets(int sockfd, char *topico, char *buffer, int tambuffer)
{
	int n,descriptor, descriptor2, descriptor3, descriptor4, ntweet_s, nleft;
	long ntweet_l;
	struct stat datos;
	char directorio[80], fichero[80],error[80], texto[20];
	char ntweet, *endptr;


	//Compruebo que exista el directorio. Si lstat devuelve 0 es que existe. Si devuelve otro valor hay que crear el directorio
	sprintf(directorio, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s", topico);
	if (n = lstat(directorio, &datos) != 0) {
		if(n=mkdir(directorio, 0777) != 0) {
			fprintf(stderr, "Error al crear el directorio");
			return -1;
		}
	}	


	//Abro el archivo numtweets y leo el primer byte. Si no existiera, guardo en ntweet un 1
	sprintf(fichero, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s/numtweets", topico);
	if((descriptor = open(fichero,O_RDONLY)) < 0) ntweet_s = 1;	
	else {
		n=read(descriptor,&ntweet,1);
		ntweet_l = strtol(&ntweet, &endptr, 10);
		ntweet_s = (int) ntweet_l;
		ntweet_s += 1;
		close(descriptor);
	}


	//Ahora creamos el fichero con el num de tweet que corresponde
	sprintf(fichero, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s/%d", topico, ntweet_s);
	if((descriptor2 = open(fichero, O_CREAT|O_WRONLY, 0644)) < 0) {
		fprintf(stderr, "Error al crear el archivo");
		return -1;	
	}
	nleft=sprintf(texto,"\nTweet numero %d:\n\n", ntweet_s);
	while ( (n = write(descriptor2, texto, nleft)) < nleft )	{
			if(n < 0)	{
				fprintf(stderr,"Error de write()\n");
				perror(error);
				return -1;
			}
	}


	//Extraemos de tambuffer en tambuffer bytes el tweet enviado por el clinte y lo guardamos en el archivo a continuación
	while( (n=read(sockfd, buffer, tambuffer)) != 0)	{
		if(n==-1)	{
			printf("Error en lectura de socket...Saliendo!\n");
			perror(error);
			return -1;
		} else	{
			buffer[n]=0;
			if((descriptor3 = open(fichero, O_RDWR|O_APPEND, 0644)) < 0) {
				fprintf(stderr, "Error al crear el archivo");
				return -1;	
			}
			nleft = strlen(buffer);
			while ( (n = write(descriptor3, buffer, nleft)) < nleft )	{
				if(n < 0)	{
					fprintf(stderr,"Error de write()\n");
					perror(error);
					return -1;
				} else if(n < nleft) {
					/* Si no fue posible enviar todo el mensaje 
					 * de una vez, se transmite el resto */
					buffer += n;
					nleft -=n;
				}
			}
		}
	}

	/*Guardamos el valor ntweet en el fichero*/
	sprintf(fichero, "/home/labs/ttg/ttgx23/Practicas_Sockets/tweets/%s/numtweets", topico);
	if((descriptor4 = open(fichero, O_CREAT|O_WRONLY, 0644)) < 0) {
		fprintf(stderr, "Error con el fichero numtweets\n");
		return -1;
	}
	sprintf(&ntweet, "%d", ntweet_s);
	if((write(descriptor4, &ntweet, 1)) < 0) {
		fprintf(stderr,"Error de write()\n");
		perror(error);
		return -1;
	}
	/*Cerramos descriptores*/
	close(descriptor2);
	close(descriptor3);
	close(descriptor4);

	return 0;
}
