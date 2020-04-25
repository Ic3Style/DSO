
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	Last revision 01/04/2020
 *
 */
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

#include "filesystem/filesystem.h" // Headers for the core functionality
#include "filesystem/auxiliary.h"  // Headers for auxiliary functions
#include "filesystem/metadata.h"   // Type and structure declaration of the file system
#include "blocks_cache.c"

//Auxiliary functions

int metadata_setDefault (void){

 // inicializar a los valores por defecto del superbloque, mapas e i-nodos
    sbloques[0].numMagico            = 0x12345; // ayuda a comprobar que se haya creado por nuestro mkfs
    sbloques[0].numInodos            = NUM_INODO;
    sbloques[0].numBloquesMapaInodos = 1;
    sbloques[0].numBloquesMapaDatos  = 1;
    sbloques[0].primerInodo          = 1;
    sbloques[0].numBloquesDatos      = NUM_DATA_BLOCK;
    sbloques[0].primerBloqueDatos    = 12; //NO SE
    sbloques[0].tamDispositivo       = 50;	//arbitrario


    //Establecer a 0 todo, y escribir 0s en el disco.
    for (int i=0; i<sbloques[0].numInodos; i++) {
     i_map[i] = 0; // free
   }

   for (int i=0; i<sbloques[0].numBloquesDatos; i++) {
     b_map[i] = 0; // free
   }

   for (int i=0; i<sbloques[0].numInodos; i++) {
     memset(&(inodos[i]), 0, sizeof(TipoInodoDisco) );
   }

return 1;

}


int metadata_writeToDisk ( void )
{
    // escribir bloque 0 de sbloques[0] a disco
    bwrite(DISK, 0, ((char *)&(sbloques[0])) );

    // escribir los bloques para el mapa de i-nodos
    for (int i=0; i<sbloques[0].numBloquesMapaInodos; i++) {
           bwrite(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // escribir los bloques para el mapa de bloques de datos
    for (int i=0; i<sbloques[0].numBloquesMapaDatos; i++) {
          bwrite(DISK, 2+i+sbloques[0].numBloquesMapaInodos, ((char *)b_map + i*BLOCK_SIZE));
    }

    // escribir los i-nodos a disco
    for (int i=0; i<(sbloques[0].numInodos*sizeof(TipoInodoDisco)/BLOCK_SIZE); i++) {
          bwrite(DISK, i+sbloques[0].primerInodo, ((char *)inodos + i*BLOCK_SIZE));
    }

    return 1;
}

int metadata_readFromDisk ( void )
{
    // leer bloque 0 de disco en sbloques[0]
    bread(DISK, 0, ((char *)&(sbloques[0])) );

    // leer los bloques para el mapa de i-nodos
    for (int i=0; i<sbloques[0].numBloquesMapaInodos; i++) {
           bread(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // leer los bloques para el mapa de bloques de datos
    for (int i=0; i<sbloques[0].numBloquesMapaDatos; i++) {
          bread(DISK, 2+i+sbloques[0].numBloquesMapaInodos, ((char *)b_map + i*BLOCK_SIZE));
    }

    // leer los i-nodos a memoria
    for (int i=0; i<(sbloques[0].numInodos*sizeof(TipoInodoDisco)/BLOCK_SIZE); i++) {
          bread(DISK, i+sbloques[0].primerInodo, ((char *)inodos + i*BLOCK_SIZE));
    }

    return 1;
}


/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
//BUFFER PARA CADA bloque
  char b[BLOCK_SIZE];

//Inicializa todos los parametros del superbloque, mapas ....
  metadata_setDefault();
	sbloques[0].tamDispositivo = deviceSize; //NO ES SEGURO
//Escribe todo al disco
  metadata_writeToDisk();

//Escribir los bloques de datos a 0. Valores iniciales.
  memset(b, 0, BLOCK_SIZE);

//Escribir cada bloque de datos al disco.
  for (int i=0; i < sbloques[0].numBloquesDatos; i++) {
    //Escribe en disco en cada bloque de datos lo que hay en el buffer.
    bwrite(DISK, sbloques[0].primerBloqueDatos + i, b);
  }

  // PONER EL CASO DE ERROR Y DEVOLVER -1.
	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
  // Si ya esta montado. ERROR
  if (1 == is_mount) {
      return -1 ;
  }

  // Leer los metadatos del sistema de ficheros de disco a memoria
  metadata_readFromDisk() ;

  // Montar
  is_mount = 1 ; // 0: falso, 1: verdadero

  return 0 ;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
  // Si hay un fichero abierto. ERROR
  for (int i = 0; i<sbloques[0].numInodos; i++) {
    if (inodos_x[i].abierto == 1) {
      return -1;
    }
  }

  // Si ya esta desmontado. ERROR
  if (0 == is_mount) {
      return -1 ;
  }

  // escribir los metadatos del sistema de ficheros de memoria a disco
  metadata_writeToDisk() ;

  // Desmontar
  is_mount = 0 ; // 0: falso, 1: verdadero

  return 0;

}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
}

/*
 * @brief	Checks the integrity of the file.
 * @return	0 if success, -1 if the file is corrupted, -2 in case of error.
 */

int checkFile (char * fileName)
{
    return -2;
}

/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */

int includeIntegrity (char * fileName)
{
    return -2;
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName)
{

    return -2;
}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */
int closeFileIntegrity(int fileDescriptor)
{
    return -1;
}

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */
int createLn(char *fileName, char *linkName)
{
    return -1;
}

/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName)
{
    return -2;
}
