
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

// #include "crc.h"

//Auxiliary functions

int metadata_setDefault (void){

 // inicializar a los valores por defecto del superbloque, mapas e i-nodos
    sbloques[0].numMagico            = 0x12345; // ayuda a comprobar que se haya creado por nuestro mkfs
    sbloques[0].numInodos            = NUM_INODO;
    sbloques[0].numBloquesMapaInodos = 1;
    sbloques[0].numBloquesMapaDatos  = 1;
    sbloques[0].primerInodo          = 3;
    sbloques[0].numBloquesDatos      = NUM_DATA_BLOCK;
    sbloques[0].primerBloqueDatos    = 51; //1+1+1+48
    sbloques[0].tamDispositivo       = 50;	//arbitrario


    //Inicializar todos los inodos a 0
    for (int i=0; i<sbloques[0].numInodos; i++) {
     bitmap_setbit(i_map, i, 0);
   }
   //Inicializar todos los bloques de datos a 0
   for (int i=0; i<sbloques[0].numBloquesDatos; i++) {
     bitmap_setbit(b_map, i, 0);
   }

   //Guarda todo en memoria
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
           bwrite(DISK, 1+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // escribir los bloques para el mapa de bloques de datos
    for (int i=0; i<sbloques[0].numBloquesMapaDatos; i++) {
          bwrite(DISK, 1+i+sbloques[0].numBloquesMapaInodos, ((char *)b_map + i*BLOCK_SIZE));
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
           bread(DISK, 1+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // leer los bloques para el mapa de bloques de datos
    for (int i=0; i<sbloques[0].numBloquesMapaDatos; i++) {
          bread(DISK, 1+i+sbloques[0].numBloquesMapaInodos, ((char *)b_map + i*BLOCK_SIZE));
    }

    // leer los i-nodos a memoria
    for (int i=0; i<(sbloques[0].numInodos*sizeof(TipoInodoDisco)/BLOCK_SIZE); i++) {
          bread(DISK, i+sbloques[0].primerInodo, ((char *)inodos + i*BLOCK_SIZE));
    }

    return 1;
}


int namei ( char *fname, int tipo){
   int i;

   // buscar i-nodo con nombre <fname>
   for (i=0; i<sbloques[0].numInodos; i++){
     if (strcmp(inodos[i].nombre, fname) == 0) {

       if (tipo == T_ENLACE_S) {
         return i;
       }

       else if(inodos[i].tipo == T_ENLACE_S){
         return inodos[i].bloqueDirecto[0];
       }
       return i;
     }
   }
   return -1;
}

int ialloc (void){

  int i;

  // buscar un i-nodo libre
  for (i=0; i<sbloques[0].numInodos; i++)
  {
        if (bitmap_getbit(i_map, i) == 0)
        {
            // inodo ocupado ahora
            bitmap_setbit(i_map, i, 1);

            // valores por defecto en el i-nodo
            memset(&(inodos[i]),0, sizeof(TipoInodoDisco));
            // devolver identificador de i-nodo

            return i;
        }
  } //Error no hay inodos libres

  return -1;
}


int alloc ( void )
{
    char b[BLOCK_SIZE];
    int i;

    for (i=0; i < sbloques[0].numBloquesDatos; i++)
    {
          if (bitmap_getbit(b_map, i) == 0)
          {
              // bloque ocupado ahora
              bitmap_setbit(b_map, i, 1);

              // valores por defecto en el bloque
              memset(b, 0, BLOCK_SIZE);
              bwrite(DISK, sbloques[0].primerBloqueDatos + i, b);
              // devolver identificador del bloque
              //bitmap_print(b_map, NUM_DATA_BLOCK);
              return i;
          }
    }

    return -1;
}

int ifree ( int inodo_id )
{
    // comprobar validez de inodo_id
    if (inodo_id > sbloques[0].numInodos) {
        return -1;
    }

    // liberar i-nodo
    bitmap_setbit(i_map, inodo_id, 0);
    // bitmap_print(i_map, NUM_INODO);

    return 1;
}

int bfree ( int block_id )
{
    // comprobar validez de block_id
    if (block_id > sbloques[0].numBloquesDatos) {
        return -1;
    }

    // liberar bloque
    bitmap_setbit(b_map, block_id, 0);
    //bitmap_print(b_map, NUM_DATA_BLOCK);

    return -1;
}

void sblock_print(void) {

  printf("%d", sbloques[0].numMagico);
  printf("%d", sbloques[0].numInodos);
  printf("%d", sbloques[0].numBloquesMapaInodos);
  printf("%d", sbloques[0].numBloquesMapaDatos);
  printf("%d", sbloques[0].primerInodo);
  printf("%d", sbloques[0].numBloquesDatos);
  printf("%d", sbloques[0].primerBloqueDatos);
  printf("%d", sbloques[0].tamDispositivo);
  printf("\n");
}

void bitmap_print(char *bitmap_, int size) {

  for (int i = 0; i < size; i++){
    printf("%d", bitmap_getbit(bitmap_, i));
  }
  printf("\n");
}

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */

int mkFS(long deviceSize)
{
  if(deviceSize > 614400) //tamanho maximo de disco
    return -1;

  char b[BLOCK_SIZE]; //BUFFER PARA CADA bloque

  metadata_setDefault(); //Inicializa todos los parametros del superbloque, mapas ....
	sbloques[0].tamDispositivo = deviceSize; //Cambia el tamanho del dispositivo
  metadata_writeToDisk(); //Escribe todo al disco

  memset(b, 0, BLOCK_SIZE); //Escribir los bloques de datos a 0. Valores iniciales.

  for (int i=0; i < sbloques[0].numBloquesDatos; i++) { //Escribir cada bloque de datos al disco.
    bwrite(DISK, sbloques[0].primerBloqueDatos + i, b); //Escribe en disco en cada bloque de datos lo que hay en el buffer.
  }
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

  metadata_readFromDisk() ; // Leer los metadatos del sistema de ficheros de disco a memoria

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

  if(strlen(fileName)>32) //tamanho maximo del nombre de un fichero
    return -2;

  int block_id, inodo_id ;
  //Si ya existe el nombre de ese fichero debe retornar error
  if (namei(fileName, T_FICHERO) != -1){
    return -1;
  }

  inodo_id = ialloc() ;
  if (inodo_id < 0) { //no hay i-nodos libres por lo que retorna -2
    return -2;
  }

  block_id = alloc();
  if (block_id < 0) { //no hay bloques libres por lo que retorna -2
    ifree(inodo_id); // libera inodo que se había ocupado
    return -2 ;
  }

  inodos[inodo_id].tipo = T_FICHERO ; //1
  strcpy(inodos[inodo_id].nombre, fileName);
  // printf("inodo nomnbre %s", inodos[inodo_id].nombre );

  inodos[inodo_id].bloqueDirecto[0] = block_id ; //se asigna solo el primero bloque de datos
  inodos[inodo_id].bloqueDirecto[1] = -1;
  inodos[inodo_id].bloqueDirecto[2] = -1;
  inodos[inodo_id].bloqueDirecto[3] = -1;
  inodos[inodo_id].bloqueDirecto[4] = -1;

  inodos[inodo_id].size = 7*sizeof(int)+32+5*sizeof(uint32_t); //el size inicial del fichero es el inodo

  inodos_x[inodo_id].posicion = 0; //Consideramos que lo creamos pero que no se abre.
  inodos_x[inodo_id].abierto = 0;
  inodos_x[inodo_id].integridad = 0;

  return 0;

}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
  int inodo_id;

  if(strlen(fileName)>32)   //Si el nombre del fichero es mayor que 32  error return -2;
    return -2;

  inodo_id = namei(fileName, T_FICHERO);

  if (inodo_id < 0){   //Error si el fichero no existe.
    return -1;
  }

  // HACER FREE a los bloques.
  for(int i=0; i<5; i++){ //liberamos los bloques asociados
    if(inodos[inodo_id].bloqueDirecto[i]>-1)
      bfree(inodos[inodo_id].bloqueDirecto[i]);
  }

  memset(&(inodos[inodo_id]), 0, sizeof(TipoInodoDisco));
  ifree(inodo_id) ;

	return 0;   //success
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
*/

int openFile(char *fileName)
{
  int inodo_id;

  //Si el nombre del fichero es mayor que 32  error return -2;
  if(strlen(fileName)>32)
    return -2;

  inodo_id = namei(fileName, T_FICHERO);

  if (inodo_id < 0){ //Error si el fichero no existe.
    return -1;
  }


  if (inodos_x[inodo_id].integridad == 1){
    //Si tiene integridad debe abrirse con integridad
    return -2;
  }

  inodos_x[inodo_id].posicion = 0;
  inodos_x[inodo_id].abierto  = 1;

	return inodo_id;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
  //Error si el descriptor es negativo o es mayor que el numero de inodos, q va de 0 a 47.
  if (fileDescriptor < 0 || fileDescriptor > sbloques[0].numInodos - 1){
    // printf("Error: El fichero %d tiene un fd no valido\n", fileDescriptor);
    return -1;
  }

  if(inodos_x[fileDescriptor].abierto  == 0){
    // printf("Error: El fichero %d ya no esta abierto\n", fileDescriptor);
    return -1;
  }

  if(inodos_x[fileDescriptor].integridad == 1){
    //no puede cerrarse sin integridad.
    return -1;
  }

  //Actualizar los metadatos del fichero:
  inodos_x[fileDescriptor].posicion = 0;
  inodos_x[fileDescriptor].abierto  = 0;
	return 0;

}


int bmap ( int inodo_id, int offset )
{
    int bloqueLogico;
    // comprobar validez de inodo_id
    if (inodo_id > sbloques[0].numInodos) {
        return -1;
    }

    // bloque de datos asociado
    if (offset < BLOCK_SIZE) {
        bloqueLogico = inodos[inodo_id].bloqueDirecto[0];
    }
    else{
    bloqueLogico = offset / BLOCK_SIZE;
    }

    if(bloqueLogico >= 5){
       // printf("Error: el fichero no puede tener mas de 5 bloques\n");
       return -2;
    }
    return inodos[inodo_id].bloqueDirecto[bloqueLogico];
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
  char array_aux[BLOCK_SIZE*5];
  int b_id ;
  // int nbytes;
  if(inodos_x[fileDescriptor].abierto == 0){
    // printf("Error en el read: El fichero %d no esta abierto\n", fileDescriptor);
    return -1;
  }
  //Si el numero de Bytes a leer es mayor que el tamaño del fichero por leer
  if (inodos_x[fileDescriptor].posicion + numBytes > inodos[fileDescriptor].size-7*sizeof(int)-32-5*sizeof(uint32_t)) {

      numBytes = (inodos[fileDescriptor].size-7*sizeof(int)-32-5*sizeof(uint32_t)) - inodos_x[fileDescriptor].posicion;
  }

  //ERROR. Si ya no queda mas fichero para leer
  if (numBytes < 0) {
    // printf("Error: El size del fichero no puede ser negativo\n");
    return -1;
  }
  if (numBytes == 0)
    return 0;

  int bytesLeft = numBytes;

  b_id = bmap(fileDescriptor, inodos_x[fileDescriptor].posicion);
  int i = 0; //variable de control
  while(b_id != -1 && bytesLeft > 0){
    bread(DISK, sbloques[0].primerBloqueDatos + b_id, array_aux + i);
    bytesLeft -= BLOCK_SIZE;
    b_id = bmap(fileDescriptor, inodos_x[fileDescriptor].posicion + i);
    i += BLOCK_SIZE;
  }

  memmove(buffer, array_aux+(inodos_x[fileDescriptor].posicion%BLOCK_SIZE), numBytes);
  inodos_x[fileDescriptor].posicion += numBytes;

  return numBytes;
}


/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
  char b[BLOCK_SIZE] ; //buffer auxiliar
  int b_id ; //numero de bloque
  int b_aux; //para asignar nuevos bloques
  int b_log_aux; //para calcular la asignacion de nuevos bloques
  int bytesLeft = numBytes; //bytes que faltan por escribir
  int bytesWr = 0; //bytes leidos, valor del return
  int fd = fileDescriptor; //se pone un nombre mas corto
  int size = bytesLeft; //bytes para copiar

  if(inodos_x[fd].abierto == 0){
    // printf("Error en el write: El fichero %d no esta abierto\n", fd);
    return -1;
  }

  if(inodos_x[fd].posicion >=  MAX_FILE_SIZE){
    // printf("Error en write: El puntero de posicion ha llegado al final del archivo\n");
    return 0;
  }

  if (size <= 0) {
      return 0;
  }

  if(numBytes > strlen(buffer)){
    bytesLeft = strlen(buffer);
  }

  int i= 0; //variable de control


  while(bytesLeft > 0){
  size = bytesLeft;
  b_id = bmap(fd, inodos_x[fd].posicion);
  if(b_id == -2){
    // printf("Error en write: El archivo esta lleno\n");
    return bytesWr;
  }
  if(b_id == -1){ //tiene los bloques llenos, hay que asignarle otro bloque
    b_aux = alloc();
    if (b_aux < 0) { //no hay bloques libres por lo que retorna -2
      // printf("Error: No hay mas bloques disponibles para asignar\n");
      return -1;
    }

    b_log_aux = inodos_x[fd].posicion/BLOCK_SIZE;
    inodos[fd].bloqueDirecto[b_log_aux] = b_aux; //se le asigna el bloque nuevo
    b_id = b_aux;
  }

  bread(DISK, sbloques[0].primerBloqueDatos+b_id, b); //lee de disco el bloque de datos
  if ((inodos_x[fd].posicion%BLOCK_SIZE)+size > BLOCK_SIZE) {
      size = BLOCK_SIZE - (inodos_x[fd].posicion%BLOCK_SIZE);
  }
  memmove(b+(inodos_x[fd].posicion%BLOCK_SIZE), buffer+i*BLOCK_SIZE, size); //lo copia del buffer al otro buffer auxiliar
  bwrite(DISK, sbloques[0].primerBloqueDatos+b_id, b); //lo escribe a disco
  inodos_x[fd].posicion += size; //se actualiza en cada loop
  inodos[fd].size += size;
  bytesLeft -= size; //se actualizan los bytes que quedan por escribir
  bytesWr += size;

  }
  // printf("hace el return\n");
  return bytesWr;

}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
  int fd = fileDescriptor;

  if(fd<0 || fd > NUM_INODO){
    // printf("El fichero no esta en un rango valido\n");
    return -1;
  }

  if(inodos_x[fd].abierto == 0){
    // printf("Error en el write: El fichero %d no esta abierto\n", fd);
    return -1;
  }


  if(whence == FS_SEEK_BEGIN){ //puntero al principio
      inodos_x[fd].posicion = 0;
      return 0;
  }
  else if(whence == FS_SEEK_END){ //puntero al final
      inodos_x[fd].posicion = (inodos[fd].size - 7*sizeof(int)-32-5*sizeof(uint32_t)); //resta lo que ocupa la cabecera al tamanho total

      return 0;
  }
  else if(whence == FS_SEEK_CUR){ //puntero current + offset
    if(inodos_x[fd].posicion + offset > inodos[fd].size - 7*sizeof(int)-32-5*sizeof(uint32_t)){ //si se intetna mover el puntero mas del limite
      // printf("El puntero se sale de limites por el final, se actualiza al limite final\n");
      inodos_x[fd].posicion = (inodos[fd].size - 7*sizeof(int)-32-5*sizeof(uint32_t)); //resta lo que ocupa la cabecera al tamanho totañ
      return 0;
    }
    if(inodos_x[fd].posicion + offset < 0){ //si se intenta poner puntero negativo
      // printf("El puntero se sale de limites menor que 0, se actualiza al limite inicial\n");
      inodos_x[fd].posicion = 0;
      return 0;
    }
      inodos_x[fd].posicion = inodos_x[fd].posicion + offset;
      return 0;
  }
	return -1;
}

/*
 * @brief	Checks the integrity of the file.
 * @return	0 if success, -1 if the file is corrupted, -2 in case of error.
 */

int checkFile (char * fileName)
{
    int inodo_id;
    int b_id;

  //Si el nombre del fichero es mayor que 32  error return -2;
  if(strlen(fileName)>32){
    // printf("El nombre del fichero es mayor que el permitido");
    return -2;
  }
    inodo_id = namei(fileName, T_FICHERO);

      //Si el fichero esta abierto error
    if(inodos_x[inodo_id].abierto == 1){
      //printf("El fichero %s esta abierto\n", fileName );
      return -2;
  }

  //Si no tiene integridad
  if (inodos_x[inodo_id].integridad == 0){
    return -2;
  }
  //Tiene integridad

  //Comprobar la integridad de ese fichero:
  int auxPos= inodos_x[inodo_id].posicion;
  inodos_x[inodo_id].posicion = 0;
  //Añadir integridad:
  for(int i = 0; i < FILE_BLOCKS; i++){
    char buffer[2048]; //to write a block
    char * pt_buffer = buffer;
    uint32_t val;
    //Fichero para cada bloque de fichero con datos
    if (inodos[inodo_id].bloqueDirecto[i] != -1){
      //Obtenemos el bloque de cada inodo
      b_id = bmap(inodo_id, inodos_x[inodo_id].posicion + i*BLOCK_SIZE);
      //Escribimos en el for
      bread(DISK, sbloques[0].primerBloqueDatos + b_id, pt_buffer);
      //Se aplica CRC32 y se obtiene la firma
      val = CRC32((const unsigned char *)pt_buffer, 2048);
      if(inodos[inodo_id].firmasIntegridad[i] != val){
        //Esta corrupto
        return -1;
      }
    }
  }
  //Se ha decido dejar la posicon del puntero en el lugar donde estaba antes de hacer checkFile.
  inodos_x[inodo_id].posicion = auxPos;
  return 0;
}



/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */

int includeIntegrity (char * fileName)
{

  int inodo_id;
  int b_id;

  inodo_id = namei(fileName, T_FICHERO);
  //Error si el fichero no existe.
  if (inodo_id < 0){
    // printf("(ERROR: El fichero %s no existe)\n", fileName);
    return -1;
  }

  if (inodos_x[inodo_id].integridad == 1){
    // printf("ERROR: El fichero  %s ya tiene integridad\n",fileName );
    return -2;
  }

  int auxPos= inodos_x[inodo_id].posicion;
  inodos_x[inodo_id].posicion = 0;
  //Añadir integridad:
  for(int i = 0; i < FILE_BLOCKS; i++){
    char buffer[2048]; //to write a block
    char * pt_buffer = buffer;
    uint32_t val;
    //Fichero para cada bloque de fichero con datos
    if (inodos[inodo_id].bloqueDirecto[i] != -1){
      //Obtenemos el bloque de cada inodo
      b_id = bmap(inodo_id, inodos_x[inodo_id].posicion + i*BLOCK_SIZE);
      //Escribimos en el for
      bread(DISK, sbloques[0].primerBloqueDatos + b_id, pt_buffer);
      //Se aplica CRC32 y se obtiene la firma
      val = CRC32((const unsigned char *)pt_buffer, 2048);
      inodos[inodo_id].firmasIntegridad[i] = val;
    }
    else{
    //Si no hay bloques de datos el valor es -1
    inodos[inodo_id].firmasIntegridad[i] = -1;
  }
  }

  // Se ha incluido integridad en ese fichero
  //Se restaura el puntero de posicion
  inodos_x[inodo_id].posicion = auxPos;
  inodos_x[inodo_id].integridad =1;

  return 0;
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName)
{

  int inodo_id;
  //Si el nombre del fichero es mayor que 32  error;
  if(strlen(fileName)>32)
    return -1;

  inodo_id = namei(fileName, T_FICHERO);
  //Error si el fichero no existe.
  if (inodo_id < 0){
    // printf("(ERROR: El fichero %s no existe)\n", fileName);
    return -1;
  }

  //Si nunca se ha calculado la integridad del fichero
  if(inodos_x[inodo_id].integridad == 0){
    // printf("ERROR: El fichero %s no tiene integridad\n", fileName);
  return -3;
  }

  //si la integridad no es correcta
  if (checkFile(fileName) == -1){
  return -2;
  }

  //Si la integridad es correcta
  inodos_x[inodo_id].posicion = 0;
  inodos_x[inodo_id].abierto  = 1;
  return inodo_id;

}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */


 int closeFileIntegrity(int fileDescriptor)
 {

   int fd = fileDescriptor;
   int b_id;
   //Error si el descriptor es negativo o es mayor que el numero de inodos, q va de 0 a 47.
   if (fileDescriptor < 0 || fileDescriptor > sbloques[0].numInodos - 1){
     // printf("Error: El fichero %d tiene un fd no valido\n", fd);
     return -1;
   }

   //Si está cerrrado no puede cerrarse por lo que da error.
   if(inodos_x[fd].abierto  == 0){
     // printf("Error: El fichero %d ya no esta abierto\n", fd);
     return -1;
   }

   //Si nunca se ha calculado la integridad del fichero n
   if(inodos_x[fd].integridad == 0){
     // printf("ERROR: El fichero %d no tiene integridad\n", fd);
   return -1;
   }

   //actualizar el crc.
   int auxPos= inodos_x[fd].posicion;
   inodos_x[fd].posicion = 0;
   for(int i = 0; i < FILE_BLOCKS; i++){
     char buffer[2048]; //to write a block
     char * pt_buffer = buffer;
     uint32_t val;
     //Fichero para cada bloque de fichero con datos
     if (inodos[fd].bloqueDirecto[i] != -1){
       //Obtenemos el bloque de cada inodo
       b_id = bmap(fd, inodos_x[fd].posicion + i*BLOCK_SIZE);
       //Escribimos en el for
       bread(DISK, sbloques[0].primerBloqueDatos + b_id, pt_buffer);
       //Se aplica CRC32 y se obtiene la firma
       val = CRC32((const unsigned char *)pt_buffer, 2048);
       inodos[fd].firmasIntegridad[i] = val;
     }else{
     //Si no hay bloques de datos el valor es -1
     inodos[fd].firmasIntegridad[i] = -1;
   }
    }

   // Se ha incluido integridad en ese fichero
   //Se restuara el puntero de posicon
   inodos_x[fd].posicion = auxPos;
   inodos_x[fd].abierto=0;
    return 0;
    }

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */


int createLn(char *fileName, char *linkName)
{
    if(strlen(fileName) > 32 && strlen(linkName) > 32){
      // printf("El nombre del fichero es mayor que el permitido");
      return -2;
    }

    int inodo_id = ialloc();
    if (inodo_id < 0) { //ERROR. No hay i-nodos libres
      return -2;
    }

    int inodo_name = namei(fileName, T_FICHERO);
    if (inodo_name < 0){      //ERROR. El fichero no existe
      // printf("(ERROR: El fichero %s no existe)\n", fileName);
      return -1;
    }

    inodos[inodo_id].tipo = T_ENLACE_S ;
    strcpy(inodos[inodo_id].nombre, linkName);
    inodos[inodo_id].size = 7*sizeof(int)+32+5*sizeof(uint32_t); //tamanho inicial del fichero
    inodos[inodo_id].bloqueDirecto[0] = inodo_name;
    inodos[inodo_id].bloqueDirecto[1] = -1;
    inodos[inodo_id].bloqueDirecto[2] = -1;
    inodos[inodo_id].bloqueDirecto[3] = -1;
    inodos[inodo_id].bloqueDirecto[4] = -1;
    inodos_x[inodo_id].integridad = 0;
    inodos_x[inodo_id].abierto  = 1;


    return 0;
}


/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName){

  int inodo_id = namei(linkName, T_ENLACE_S);
  if (inodo_id < 0){      //ERROR. El fichero no existe
    //printf("(ERROR: El enlace simbolico %s no existe)\n", linkName);
    return -1;
  }

  for(int i=0; i<5; i++){ //liberamos los bloques asociados
    if(inodos[inodo_id].bloqueDirecto[i]>-1)
      bfree(inodos[inodo_id].bloqueDirecto[i]);
  }

  memset(&(inodos[inodo_id]), 0, sizeof(TipoInodoDisco));
  ifree(inodo_id) ;

  inodos_x[inodo_id].abierto  = 0;
  return 0;
}
