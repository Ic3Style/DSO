
/*
 *  Copyright 2016-2020 Alejandro Calderon Mateos (ARCOS.INF.UC3M.ES)
 *
 *  This file is part of nanofs (nano-filesystem).
 *
 *  nanofs is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  nanofs is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with nanofs.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
 *  Interfaz servidor de bloques (block-read + block-write)
 */

#define DISK "tmpfile.dat"
#define BLOCK_SIZE 1024

int bread ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fread(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}

int bwrite ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r+") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fwrite(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}


/*
 *  (1) Estructura de datos en disco
 */

#define NUM_INODO 10
#define NUM_BLOQUES_DATO 20

typedef struct {
    unsigned int numMagico;	            /* Número mágico del superbloque: 0x12345 */
    unsigned int numBloquesMapaInodos;      /* Número de bloques del mapa inodos */
    unsigned int numBloquesMapaDatos;       /* Número de bloques del mapa  datos */
    unsigned int numInodos; 	            /* Número de inodos en el dispositivo */
    unsigned int primerInodo;	            /* Número bloque del 1º inodo del disp. (inodo raíz) */
    unsigned int numBloquesDatos;           /* Número de bloques de datos en el disp. */
    unsigned int primerBloqueDatos;         /* Número de bloque del 1º bloque de datos */
    unsigned int tamDispositivo;	    /* Tamaño total del disp. (en bytes) */
    char relleno[BLOCK_SIZE-8*sizeof(int)]; /* Campo de relleno (para completar un bloque) */
} TipoSuperbloque;

typedef struct {
    unsigned int tipo;	                  /* T_FICHERO o T_DIRECTORIO */
    char nombre[200];	                  /* Nombre del fichero/ directorio asociado */
    unsigned int inodosContenidos[200];   /* tipo==dir: lista de los inodos del directorio */
    unsigned int size;	                  /* Tamaño actual del fichero en bytes */
    unsigned int bloqueDirecto;	          /* Número del bloque directo */
    unsigned int bloqueIndirecto;	  /* Número del bloque indirecto */
    char relleno[BLOCK_SIZE-204*sizeof(int)-200]; /* Campo relleno para llenar un bloque */
} TipoInodoDisco;
#define PADDING_INODO (BLOCK_SIZE - sizeof(TipoInodoDisco))

typedef char  inodo_map[NUM_INODO] ;          /* 100…0 (usado: i_map[x]=1 | libre: i_map[x]=0) */
typedef char bloque_map[NUM_BLOQUES_DATO] ;   /* 000…0 (usado: b_map[x]=1 | libre: b_map[x]=0) */

#define T_FICHERO    1
#define T_DIRECTORIO 2


/*
 *  (2) Estructura de datos en memoria
 */

// Metadatos leídos desde disco
TipoSuperbloque sbloques[1] ;
           char i_map[BLOCK_SIZE] ;
           char b_map[BLOCK_SIZE] ;
 TipoInodoDisco inodos[NUM_INODO] ;

// Metadatos extra de apoyo (que no van a disco)
struct {
    int posicion ; // posición de lectura/escritura
    int abierto  ; // 0: falso, 1: verdadero
} inodos_x [NUM_INODO] ;

int esta_montado = 0 ; // 0: falso, 1: verdadero


/*
 *
 */

int nanofs_ialloc ( void )
{
    int i;

    // buscar un i-nodo libre
    for (i=0; i<sbloques[0].numInodos; i++)
    {
          if (i_map[i] == 0)
          {
              // inodo ocupado ahora
              i_map[i] = 1;
              // valores por defecto en el i-nodo
              memset(&(inodos[i]),0, sizeof(TipoInodoDisco));
              // devolver identificador de i-nodo
              return i;
          }
    }

    return -1;
}

int nanofs_alloc ( void )
{
    char b[BLOCK_SIZE];
    int i;

    for (i=0; i<sbloques[0].numBloquesDatos; i++)
    {
          if (b_map[i] == 0)
          {
              // bloque ocupado ahora
              b_map[i] = 1;
              // valores por defecto en el bloque
              memset(b, 0, BLOCK_SIZE);
              bwrite(DISK, sbloques[0].primerBloqueDatos + i, b);
              // devolver identificador del bloque
              return i;
          }
    }

    return -1;
}

int nanofs_ifree ( int inodo_id )
{
    // comprobar validez de inodo_id
    if (inodo_id > sbloques[0].numInodos) {
        return -1;
    }

    // liberar i-nodo
    i_map[inodo_id] = 0;

    return -1;
}

int nanofs_free ( int block_id )
{
    // comprobar validez de block_id
    if (block_id > sbloques[0].numBloquesDatos) {
        return -1;
    }

    // liberar bloque
    b_map[block_id] = 0;

    return -1;
}

int nanofs_namei ( char *fname )
{
   int i;

   // buscar i-nodo con nombre <fname>
   for (i=0; i<sbloques[0].numInodos; i++)
   {
         if (! strcmp(inodos[i].nombre, fname)) {
               return i;
         }
   }

   return -1;
}

int nanofs_bmap ( int inodo_id, int offset )
{
    int b[BLOCK_SIZE/4];

    // comprobar validez de inodo_id
    if (inodo_id > sbloques[0].numInodos) {
        return -1;
    }

    // bloque de datos asociado
    if (offset < BLOCK_SIZE) {
        return inodos[inodo_id].bloqueDirecto;
    }

    if (offset < BLOCK_SIZE*BLOCK_SIZE/4)
    {
         bread(DISK,
               sbloques[0].primerBloqueDatos +
               inodos[inodo_id].bloqueIndirecto, b);
         offset = (offset - BLOCK_SIZE) / BLOCK_SIZE;
         return b[offset] ;
    }

    return -1;
}


/*
 *
 */

int nanofs_meta_readFromDisk ( void )
{
    // leer bloque 0 de disco en sbloques[0]
    bread(DISK, 0, &(sbloques[0]) );

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

int nanofs_meta_writeToDisk ( void )
{
    // escribir bloque 0 de sbloques[0] a disco
    bwrite(DISK, 0, &(sbloques[0]) );

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

int nanofs_meta_setDefault ( void )
{
    // inicializar a los valores por defecto del superbloque, mapas e i-nodos
    sbloques[0].numMagico            = 0x12345; // ayuda a comprobar que se haya creado por nuestro mkfs
    sbloques[0].numInodos            = NUM_INODO;
    sbloques[0].numBloquesMapaInodos = 1;
    sbloques[0].numBloquesMapaDatos  = 1;
    sbloques[0].primerInodo          = 1;
    sbloques[0].numBloquesDatos      = NUM_BLOQUES_DATO;
    sbloques[0].primerBloqueDatos    = 12;
    sbloques[0].tamDispositivo       = 20;

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

int nanofs_mount ( void )
{
    if (1 == esta_montado) {
        return -1 ;
    }

    // leer los metadatos del sistema de ficheros de disco a memoria
    nanofs_meta_readFromDisk() ;

    // montar
    esta_montado = 1 ; // 0: falso, 1: verdadero

    return 1 ;
}

int nanofs_umount ( void )
{
    if (0 == esta_montado) {
        return -1 ;
    }

    // escribir los metadatos del sistema de ficheros de memoria a disco
    nanofs_meta_writeToDisk() ;

    // desmontar
    esta_montado = 0 ; // 0: falso, 1: verdadero

    return 1 ;
}

int nanofs_mkfs ( void )
{
    char b[BLOCK_SIZE];

    // establecer los valores por defecto en memoria
    nanofs_meta_setDefault() ;

    // escribir el sistema de ficheros inicial a disco
    nanofs_meta_writeToDisk() ;

    // rellenar los bloques de datos con ceros
    memset(b, 0, BLOCK_SIZE);
    for (int i=0; i < sbloques[0].numBloquesDatos; i++) {
         bwrite(DISK, sbloques[0].primerBloqueDatos + i, b);
    }

    return 1;
}


/*
 *
 */

int nanofs_open ( char *nombre )
{
    int inodo_id ;

    inodo_id = nanofs_namei(nombre) ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    inodos_x[inodo_id].posicion = 0;
    inodos_x[inodo_id].abierto  = 1;

    return inodo_id;
}

int nanofs_close ( int fd )
{
     if (fd < 0) {
         return fd ;
     }

     inodos_x[fd].posicion = 0;
     inodos_x[fd].abierto  = 0;

     return 1;
}

int nanofs_creat ( char *nombre )
{
    int b_id, inodo_id ;

    inodo_id = nanofs_ialloc() ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    b_id = nanofs_alloc();
    if (b_id < 0) {
        nanofs_ifree(inodo_id);
        return b_id ;
    }

    inodos[inodo_id].tipo = T_FICHERO ;
    strcpy(inodos[inodo_id].nombre, nombre);
    inodos[inodo_id].bloqueDirecto = b_id ;
    inodos_x[inodo_id].posicion    = 0;
    inodos_x[inodo_id].abierto     = 1;

    return inodo_id ;
}

int nanofs_unlink ( char * nombre )
{
     int inodo_id ;

     inodo_id = nanofs_namei(nombre) ;
     if (inodo_id < 0) {
         return inodo_id ;
     }

     nanofs_free(inodos[inodo_id].bloqueDirecto);
     memset(&(inodos[inodo_id]), 0, sizeof(TipoInodoDisco));
     nanofs_ifree(inodo_id) ;

    return 1;
}

int nanofs_read ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;
     int b_id ;

     if (inodos_x[fd].posicion+size > inodos[fd].size) {
         size = inodos[fd].size - inodos_x[fd].posicion;
     }
     if (size <= 0) {
         return 0;
     }

     b_id = nanofs_bmap(fd, inodos_x[fd].posicion);
     bread(DISK, sbloques[0].primerBloqueDatos+b_id, b);
     memmove(buffer, b+inodos_x[fd].posicion, size);
     inodos_x[fd].posicion += size;

     return size;
}

int nanofs_write ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;
     int b_id ;

     if (inodos_x[fd].posicion+size > BLOCK_SIZE) {
         size = BLOCK_SIZE - inodos_x[fd].posicion;
     }
     if (size <= 0) {
         return 0;
     }

     b_id = nanofs_bmap(fd, inodos_x[fd].posicion);
     bread(DISK, sbloques[0].primerBloqueDatos+b_id, b);
     memmove(b+inodos_x[fd].posicion, buffer, size);
     bwrite(DISK, sbloques[0].primerBloqueDatos+b_id, b);
     inodos_x[fd].posicion += size;
       inodos[fd].size     += size;

     return size;
}


/*
 *
 */

int main()
{
   int   ret = 1 ;
   int   fd  = 1 ;
   char *str1 = "hola mundo..." ;
   char  str2[20] ;

   //
   // mkfs-mount
   //
   if (ret != -1)
   {
       printf(" * nanofs_mkfs() -> ") ;
       ret = nanofs_mkfs() ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
       printf("%d\n", ret) ;
   }

   //
   // creat-write-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_creat('test1.txt') -> ") ;
       ret = fd = nanofs_creat("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_write(%d,'%s',%ld) -> ", ret, str1, strlen(str1)) ;
       ret = nanofs_write(fd, str1, strlen(str1)) ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // open-read-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_open('test1.txt') -> ") ;
       ret = fd = nanofs_open("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       memset(str2, 0, 20) ;
       printf(" * nanofs_read(%d,'%s',%d) -> ", ret, str2, 13) ;
       ret = nanofs_read(fd, str2, 13) ;
       printf("%d (%s)\n", ret, str2) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // unlink-umount
   //
   if (ret != -1)
   {
       printf(" * nanofs_unlink('test1.txt') -> ") ;
       ret = nanofs_unlink("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_umount() -> ") ;
       ret = nanofs_umount() ;
       printf("%d\n", ret) ;
   }

   return 0 ;
}
