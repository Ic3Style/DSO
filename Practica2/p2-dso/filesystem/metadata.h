
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	Last revision 01/04/2020
 *
 */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

#define BLOCK_SIZE 2048 //Size of block in FS.
#define DISK "disk.dat"


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
    char nombre[32];	                  /* Nombre del fichero/ directorio asociado */
    unsigned int inodosContenidos[48];   /* tipo==dir: lista de los inodos del directorio */
    unsigned int size;	                  /* Tamaño actual del fichero en bytes */
    unsigned int bloqueDirecto;	          /* Número del bloque directo */
    unsigned int bloqueIndirecto;	  /* Número del bloque indirecto */
    //char relleno[BLOCK_SIZE-48*sizeof(int)-32]; /* Campo relleno para llenar un bloque */
} TipoInodoDisco;
#define PADDING_INODO (BLOCK_SIZE - sizeof(TipoInodoDisco))


//Tipo de Inodo.
#define T_FICHERO    1
#define T_DIRECTORIO 2


//Cambiar
#define NUM_INODO 48
#define NUM_DATA_BLOCK 240



// Metadatos leídos desde disco
TipoSuperbloque sbloques[1] ; //Aarray de tipo superbloque de tamaño 1.
           char i_map[NUM_INODO] ;
           char b_map[NUM_DATA_BLOCK] ;
 TipoInodoDisco inodos[NUM_INODO] ;



// Metadatos extra de apoyo (que no van a disco)
struct {
    int posicion ; // posición de lectura/escritura
    int abierto  ; // 0: falso, 1: verdadero
} inodos_x [NUM_INODO] ;

int is_mount = 0; // global para decir si esta montado 