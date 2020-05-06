
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 *
 */


#include <stdio.h>
#include <string.h>
#include "filesystem/filesystem.h"


// Color definitions for asserts
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"

#define N_BLOCKS 25					  // Number of blocks in the device
#define DEV_SIZE N_BLOCKS *BLOCK_SIZE // Device size, in bytes

int main()
{
	int ret; //variable que indica el retorno
	int fd; //descriptor del fichero
	long offset; //bit de desplazamiento
	int size; //tamanho de escritura y lectura
	int byte_write; //numero de bytes escritos
	int byte_read; //numero de bytes leidos

	char buffer[1000]; //buffer para escribir lo leido
	char *resultado_esperado; //para las pruebas de lectura y escritura
	char *buffer_write; //buffer para pruebas de escritura

	/////// TEST 01

	ret = mkFS(DEV_SIZE);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	/////// TEST 02

	ret = mountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	/////// TEST 03

	ret = createFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	/////// TEST 04

	fd = openFile("/test.txt");
	if (fd < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	//////// TEST 05

	buffer_write = "hola mundo";
	size = 7; //se prueba a escribir una parte del buffer
	byte_write = writeFile(fd, buffer_write, size);
	if (byte_write != 7)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	//////// TEST 06

	//tiene que dar error porque no hemos movido el puntero

	size = 10; //va a leer 10 bytes
	byte_read = readFile(fd, buffer, size);

// printf("block read: \n<%s>\n", buffer); DESCOMENTAR PARA VER EL BLOQUE LEIDO

	if (byte_read != 0) //no lee ningun byte
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile without lseek", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile without lseek ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 07

		//se prueba poner el puntero al inicio para que asi pueda leer
		offset = 0; //el offset solo vale para CUR
		ret = lseekFile( fd, offset, FS_SEEK_BEGIN);
		if(ret != 0){
				fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile BEGIN ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
				return -1;
		}
		else{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile BEGIN ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
		}

		//////// TEST 08

		//ahora se prueba a leer con el puntero al principio
		memset(buffer, 0, 1000); //se resetea el buffer
		size = 10; //va a leer 10 bytes, aunque solo hay 7 en el buffer por lo que leera 7
		byte_read = readFile(fd, buffer, size);
		resultado_esperado = "hola mu";
		// printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

		if (byte_read != 7 || strncmp(buffer, resultado_esperado, 7)) //lee los 7 bytes que hay
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile after lseek BEGIN ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile after lseek BEGIN ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 09

		//se prueba poner el puntero en el medio, para ello, ahora que esta en el final se le resta 4
		offset = -4; //el puntero deberia cambiar a estar del 7 al 3
		ret = lseekFile(fd, offset, FS_SEEK_CUR);
		if(ret != 0){
				fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile CUR ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
				return -1;
		}
		else{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile CUR ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
		}

		//////// TEST 10

		//ahora se prueba a leer con el puntero en la tercera posicion, que deberia ser <holA mu> en la mayuscula
		memset(buffer, 0, 1000); //se resetea el buffer
		size = 2; //va a leer 2 bytes, que en este caso seran <a >
		byte_read = readFile(fd, buffer, size);

		// printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

		if (byte_read != 2) //lee los 7 bytes que hay
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile after lseek CUR ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile after lseek CUR ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 11

		//ahora probamos a sobreescribir una parte del codigo
		lseekFile( fd, offset, FS_SEEK_BEGIN); //movemos el puntero al inicio

		buffer_write = "SSS";
		size = 2; //se prueba a escribir una parte del buffer
		byte_write = writeFile(fd, buffer_write, size);

		lseekFile( fd, offset, FS_SEEK_BEGIN); //movemos el puntero al inicio otra vez para leer

		memset(buffer, 0, 1000); //se resetea el buffer
		size = 7; //va a leer los 7 bytes del buffer
		byte_read = readFile(fd, buffer, size);
		resultado_esperado = "SSla mu";

 	  //printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

		if (byte_read != 7 || strncmp(buffer, resultado_esperado, 7) != 0) //se comprueba con el resultado esperado
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile overwrite ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile overwrite ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 12

		//cerramos el archivo
		ret = closeFile(fd);
		if (ret != 0)
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 13

		//se intenta escribir en el archivo cerrado
		size = 4; //se prueba a escribir una parte del buffer
		byte_write = writeFile(fd, buffer_write, size);
		if (byte_write != -1) //da error al estar cerrado
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile after closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile after closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 14

		//probamos a eliminar el archivo
		ret = removeFile("/test.txt");
		if (ret != 0)
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


		////////////// TESTS CON INTEGRIDAD ///////////////////

		createFile("/test_int.txt"); //se crea un fichero nuevo

		//////// TEST 15

		//se mete integridad al fichero
		ret = includeIntegrity("/test_int.txt");

		if (ret < 0)
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST includeIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST includeIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 16

		//se intenta abrir con un openFile sin INTEGRIDAD
		fd = openFile("/test_int.txt");

		if (fd != -2) //es el error de integridad
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 17

		//se intenta abrir con openFileIntegrity
		fd = openFileIntegrity("/test_int.txt");

		if (fd < 0) //es el error de integridad
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 18

		//se prueba a escribir un texto que haga que se llene un bloque de datos y se le asigne otro al inodo
		//se llena el buffer con 2229 bytes (mas de un bloque de 2048)
		buffer_write = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

		byte_write = writeFile(fd, buffer_write, 2200);
		if (byte_write != 2200)
		{
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY fillDataBlock ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
		}
		printf("EL numero de bytes escritos es: %d \n", byte_write);
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST_INTEGRITY fillDataBlock ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	////////

//int cf= checkFile();
// closeFile(fd);

ret = checkFile("/test.txt");

if (ret < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	// return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);



//// TEST

fd = openFileIntegrity("/test.txt");

if (fd < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

ret= closeFileIntegrity(fd);
if (ret < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

///FIN test


//// TEST

fd = openFile("/test.txt");

if (fd < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

ret= closeFileIntegrity(fd);
if (ret < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

///FIN test



//// TEST

fd = openFileIntegrity("/test.txt");

if (fd < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

ret= closeFile(fd);
if (ret < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

///FIN test



//// TEST

fd = openFile("/test.txt");

if (fd < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

//ERROR PORQUE NO TEIENE INTEGRIDAD.

ret= checkFile("/test.txt");
if (ret < 0)
{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	return -1;
}
fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

///FIN test






/////////
	// char buffer[1000];
	// size = 10;
	// byte_read = readFile(fd, buffer, size);
	// printf("block read: \n<%s>\n", buffer);
	//
	// if (byte_read < 0)
	// {
	// 	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	// 	return -1;
	// }
	// fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		////////


offset = 0;
ret = lseekFile( fd, offset, FS_SEEK_END);
if(ret != 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
}
else{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
}

/////////
	 // char buffer2[1000];
	memset(buffer, 0, 1000);
	size = 15;
	byte_read = readFile(fd, buffer, size);
	printf("block read: \n<%s>\n", buffer);

	if (byte_read < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


///////
offset = 2;
ret = lseekFile( fd, offset, FS_SEEK_BEGIN);
if(ret != 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
}
else{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
}

/////////

	memset(buffer, 0, 1000);
	size = 15;
	byte_read = readFile(fd, buffer, size);
	printf("block read: \n<%s>\n", buffer);

	if (byte_read < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

///////
offset = 2;
// retSeek = lseekFile( fd, offset, FS_SEEK_BEGIN);
ret = lseekFile( fd, offset, FS_SEEK_CUR);
if(ret != 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile CUR ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
}
else{
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile CUR", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
}

/////////
	memset(buffer, 0, 1000);
	byte_read = readFile(fd, buffer, 20);
	printf("block read: \n<%s>\n", buffer);

	if (byte_read < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
// ///////
//
// 	ret = closeFile(fd);
// 	if (ret != 0)
// 	{
// 		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
// 		return -1;
// 	}
// 	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
//
// 	////////

	// ret = removeFile("/test.txt");
	// if (ret != 0)
	// {
	// 	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	// 	return -1;
	// }
	// fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = unmountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	return 0;
}
