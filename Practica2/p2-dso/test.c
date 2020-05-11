
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



	/////// TEST 00

	//se prueba a crear el sistema de ficheros fuera de rango, mayor que el disco
	ret = mkFS(614401);
	if (ret != -1)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "00: TEST mkFS fuera de rango ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "00: TEST mkFS fuera de rango ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	/////// TEST 01

	//se prueba a crear el sistema de ficheros
	ret = mkFS(DEV_SIZE);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "01: TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "01: TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	/////// TEST 02

	//se prueba a montar el sistema de ficheros
	ret = mountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "02: TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "02: TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	/////// TEST 03

	//se prueba a crear el archivo
	ret = createFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "03: TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "03: TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	/////// TEST 04

	//se prueba abrir el archivo
	fd = openFile("/test.txt");
	if (fd < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "04: TEST openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "04: TEST openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 05

	//se prueba a escribir en el archivo
	buffer_write = "hola mundo";
	size = 7; //se prueba a escribir una parte del buffer
	byte_write = writeFile(fd, buffer_write, size);
	if (byte_write != 7)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "05: TEST writeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "05: TEST writeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 06

	//se prueba a leer el archivo justo despues de escribir, tiene que dar error porque no hemos movido el puntero
	size = 10; //va a leer 10 bytes
	byte_read = readFile(fd, buffer, size);

// printf("block read: \n<%s>\n", buffer); DESCOMENTAR PARA VER EL BLOQUE LEIDO

	if (byte_read != 0) //no lee ningun byte
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "06: TEST readFile without lseek", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "06: TEST readFile without lseek ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 07

	//se prueba poner el puntero al inicio para que asi pueda leer
	offset = 0; //el offset solo vale para CUR
	ret = lseekFile( fd, offset, FS_SEEK_BEGIN);
	if(ret != 0){
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "07: TEST lseekFile BEGIN ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
	}
	else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "07: TEST lseekFile BEGIN ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
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
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "08: TEST readFile after lseek BEGIN ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "08: TEST readFile after lseek BEGIN ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 09

	//se prueba poner el puntero en el medio, para ello, ahora que esta en el final se le resta 4
	offset = -4; //el puntero deberia cambiar a estar del 7 al 3
	ret = lseekFile(fd, offset, FS_SEEK_CUR);
	if(ret != 0){
			fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "09: TEST lseekFile CUR ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
			return -1;
	}
	else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "09: TEST lseekFile CUR ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}


	//////// TEST 10

	//ahora se prueba a leer con el puntero en la tercera posicion, que deberia ser <holA mu> en la mayuscula
	memset(buffer, 0, 1000); //se resetea el buffer
	size = 2; //va a leer 2 bytes, que en este caso seran <a >
	byte_read = readFile(fd, buffer, size);

	// printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

	if (byte_read != 2) //lee los 7 bytes que hay
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "10: TEST readFile after lseek CUR ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "10: TEST readFile after lseek CUR ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 11

	//ahora probamos a sobreescribir una parte del codigo
	lseekFile( fd, offset, FS_SEEK_BEGIN); //movemos el puntero al inicio

	buffer_write = "SSS";
	size = 2; //se prueba a escribir una parte del buffer
	byte_write = writeFile(fd, buffer_write, size);

	lseekFile( fd, offset, FS_SEEK_BEGIN); //movemos el puntero al inicio otra vez para leer

	memset(buffer, 0, 1000); //se resetea el buffer
	size = 3; //va a leer los 3 primeros bytes del buffer
	byte_read = readFile(fd, buffer, size);
	resultado_esperado = "SSl";

	//printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

	if (byte_read != 3 || strncmp(buffer, resultado_esperado, 7) != 0) //se comprueba con el resultado esperado
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "11a: TEST writeFile overwrite & multiple read ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "11a: TEST writeFile overwrite & multiple read ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	size = 4; //va a leer los 4 siguientes bytes del buffer
	byte_read = readFile(fd, buffer, size);
	resultado_esperado = "a mu";
	// printf("block read: \n<%s>\n", buffer); //DESCOMENTAR PARA VER EL BLOQUE LEIDO

	if (byte_read != 4 || strncmp(buffer, resultado_esperado, 7) != 0) //se comprueba con el resultado esperado
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "11b: TEST writeFile overwrite & multiple read ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "11b: TEST writeFile overwrite & multiple read ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////// TEST 12

	//para poder probar llenar los 5 bloques, se crea un texto que tiene 6 bloques: 12288 bytes
	buffer_write = "1 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. 2 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. 3 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. 4 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. 5 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. 6 BLOQUE Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor. ";

	//se intenta escribir MAS DE 5 BLOQUES
	size = 11000; //se prueba a escribir una parte del buffer
	byte_write = writeFile(fd, buffer_write, size);
	// printf("byte write: %d\n", byte_write);
	if (byte_write != 10233) //devuelve el numero de bytes que le faltan para llenarse
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "12: TEST writeFile full ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "12: TEST writeFile full ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	//////// TEST 13

	//se prueba a cerrar el archivo
	ret = closeFile(fd);
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "13: TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "13: TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 14

	//se intenta escribir en el archivo cerrado
	size = 4; //se prueba a escribir una parte del buffer
	byte_write = writeFile(fd, buffer_write, size);
	if (byte_write != -1) //da error al estar cerrado
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "14: TEST writeFile after closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "14: TEST writeFile after closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 15

	//probamos a eliminar el archivo
	ret = removeFile("/test.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "15: TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "15: TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	////////////// TESTS CON INTEGRIDAD ///////////////////

	createFile("/test_int.txt"); //se crea un fichero nuevo


	//////// TEST 16

	//se mete integridad al fichero
	ret = includeIntegrity("/test_int.txt");

	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "16: TEST_INTEGRITY includeIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "16: TEST_INTEGRITY includeIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 17

	//se intenta abrir con un openFile sin INTEGRIDAD
	fd = openFile("/test_int.txt");

	if (fd != -2) //es el error de integridad
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "17: TEST_INTEGRITY openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "17: TEST_INTEGRITY openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 18

	//se comprueba la integridad del archivo antes de abrirlo y modificarlo
	ret = checkFile("/test_int.txt");
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "18: TEST_INTEGRITY checkFile before open ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "18: TEST_INTEGRITY checkFile before open ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 19

	//se intenta abrir con openFileIntegrity
	fd = openFileIntegrity("/test_int.txt");

	if (fd < 0) //es el error de integridad
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "19: TEST_INTEGRITY openFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "19: TEST_INTEGRITY openFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 20

	//se intenta comprobar la integridad del archivo una vez abierto, por lo que dara error -2
	ret = checkFile("/test_int.txt");
	if (ret < -2)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "20: TEST_INTEGRITY checkFile after open ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "20: TEST_INTEGRITY checkFile after open ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 21

	//se prueba a escribir un texto que haga que se llene un bloque de datos y se le asigne otro al inodo
	//se llena el buffer con 2229 bytes (mas de un bloque de 2048)
	buffer_write = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. ";

	byte_write = writeFile(fd, buffer_write, 2200);
	if (byte_write != 2200)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "21: TEST_INTEGRITY fillDataBlock ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "21: TEST_INTEGRITY fillDataBlock ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

		//////// TEST 22

	//se prueba a desmontar el sistema de ficheros con un fichero abierto
	ret = unmountFS();
	if (ret != -1)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "22: TEST_INTEGRITY unmountFS before closeFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "22: TEST_INTEGRITY unmountFS before closeFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 23

	//se comrpueba cerrar el fichero para poder asi chequear de nuevo la integridad
	ret= closeFileIntegrity(fd);
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "23: TEST_INTEGRITY closeFileIntegrity ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "23: TEST_INTEGRITY closeFileIntegrity ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 24

	//se comprueba la integridad del archivo despues de modificarlo y actualizarlo
	ret = checkFile("/test_int.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "24: TEST_INTEGRITY checkFile after write ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "24: TEST_INTEGRITY checkFile after write ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	//////// TEST 25

	//probamos a eliminar el archivo con integridad
	ret = removeFile("/test_int.txt");
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "25: TEST_INTEGRITY removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "25: TEST_INTEGRITY removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	///////////////////////////////////////////////////////////////
	////////////// TESTS CON ENLACES SIMBOLICOS ///////////////////
	///////////////////////////////////////////////////////////////

	createFile("/test_zelda.txt"); //se crea un fichero nuevo
	fd = openFile("/test_zelda.txt");

	//////// TEST 26

	// crear un enlace simbolico a un fichero existente
	ret = createLn("/test_zelda.txt", "/enlace1");
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "26: TEST_LINK createLn exist ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "26: TEST_LINK createLn exist ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 27

	// crear un enlace simbolico a un fichero que no existe
	ret = createLn("/no_existe.txt", "/enl");
	if (ret != -1)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "27: TEST_LINK createLn not exist ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "27: TEST_LINK createLn not exist ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 28

	// crear un enlace simbolico a otro enlace
	ret = createLn("/enlace1", "/enlace2");
	if (ret < 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "28: TEST_LINK createLn to a link ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "28: TEST_LINK createLn to a link ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 29

	// nombre de fichero largo
	ret = createLn("/111111111111111111111111111111111111111111", "/111111111111111111111111111111111111111111");
	if (ret == 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "29: TEST_LINK createLn long name ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "29: TEST_LINK createLn long name ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 30

	// se prueba a escribir usando el enlace simbolico
	fd = openFile("/enlace2");
	byte_write = writeFile(fd, "hola mundo", 7);
	if (byte_write != 7)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "30: TEST_LINK writeFile through a link ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "30: TEST_LINK writeFile through a link ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 31

	// se eliminan los enlaces simbolicos
	int link2 = removeLn("/enlace2");
	int link1 = removeLn("/enlace1");
	closeFile(fd);
	if (link1 != 0 && link2 != 0 )
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "31: TEST_LINK remove a link ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "31: TEST_LINK remove a link ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);


	//////// TEST 32

	//se prueba a desmontar el sistema de ficheros
	ret = unmountFS();
	if (ret != 0)
	{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "32: TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "32: TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	//////// FIN TESTS

	return 0;
}
