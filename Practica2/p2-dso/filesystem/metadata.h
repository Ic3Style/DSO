
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

static inline void bitmap_print(char *bitmap_, int size) {

  for (int i = 0; i < size; i++){
    printf("%d", bitmap_getbit(bitmap_, i));
  }
  printf("\n");
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
    unsigned int tipo;                  /* T_FICHERO o T_DIRECTORIO */

    char nombre[32];	                  /* Nombre del fichero/ directorio asociado */
    unsigned int size;	                  /* Tamaño actual del fichero en bytes */
    int bloqueDirecto[5];	          /* Número de los bloques de datos del i-nodo, maximo 5 */
    uint32_t firmasIntegridad[5];    /*arra y de firmas de integridad para cada bloque del fichero.*/

    char relleno[BLOCK_SIZE-7*sizeof(int)-32-5*sizeof(uint32_t)]; /* Campo relleno para llenar un bloque */
} TipoInodoDisco;
#define PADDING_INODO (BLOCK_SIZE - sizeof(TipoInodoDisco))


//Tipo de Inodo.
#define T_FICHERO    1
#define T_ENLACE_S   2



//INODOS Y BLOQUES
#define NUM_INODO 48
#define NUM_DATA_BLOCK 240
#define FILE_BLOCKS 5 // un fichero tiene 5 bloques como máximo



// Metadatos leídos desde disco
TipoSuperbloque sbloques[1] ; //Aarray de tipo superbloque de tamaño 1.
char i_map[NUM_INODO] ;
char b_map[NUM_DATA_BLOCK] ;

TipoInodoDisco inodos[NUM_INODO] ;



// Metadatos extra de apoyo (que no van a disco)
struct {
    int posicion ; // posición de lectura/escritura
    int abierto  ; // 0: falso, 1: verdadero
    int integridad ; // 0: no tiene. 1: si tiene
} inodos_x [NUM_INODO] ;

int is_mount = 0; // global para decir si esta montado

static inline void sblock_print() {

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
