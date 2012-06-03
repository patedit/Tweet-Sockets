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
gettweet(int sockfd, char *buffer, int tambuffer)	{

	int leido,n, encontrado, nleft,c;
	long leido_l;
	char *endptr, *p, error[80], *ptr;
	

	/*Lectura de un byte de la conexion referente a la cantidad de tweets del topico
	si recibe un FIN o error devuelve -1*/	
	while((n=read(sockfd,buffer,1)) < 1) {
			fprintf(stdout,"Error en la lectura\n");
			perror(error);
			return -1;
	}

	/*Convertimos a long, comprobamos posibles errores y convertimos a int*/
	leido_l = strtol(buffer,&endptr,10);
        if(strlen(endptr)!=0)   {
		fprintf(stderr,"No he recibido un caracter numerico.\n");
		return -1;
	}
	leido = (int) leido_l;
	if(leido==0) {
		fprintf(stdout, "No hay tweets del topico seleccionado\n");
		return 0;
	}

	/*Informamos y recogemos el tweet que se quiere leer*/
	fprintf(stdout, "Hay %d tweets del topico seleccionado.\n Introduzca el numero del tweet que quiere obtener: ", leido);
	c=getchar();

	/*Convertimos el codigo ASCII a su correspondiente numero*/
	c = c - 48;
	if(c<=0 | c>leido) {
		printf("Debe introducir un numero mayor que 0 y menor o igual que %d\n", leido);
		return -1;
	}
	
	//Enviamos el caracter introducido
	sprintf(buffer, "%d", c);
	while ( (n = write(sockfd, buffer, 1)) < 1 )	{
		if(n < 0)	{
			fprintf(stderr,"Error de write()\n");
			perror(error);
			return -1;
		} 
	}
	/*Extraemos de tambuffer en tambuffer bytes los datos recibidos por la conexion (tweet)*/
	while( (n=read(sockfd, buffer, tambuffer)) != 0)	{
		if(n<1)	{
			printf("Error en lectura de socket!\n");
			perror(error);
			return -1;
		} else	{
			buffer[n]=0;
			printf("%s",buffer);
		}
	}
	/*Todo OK*/
	return 0;

}
