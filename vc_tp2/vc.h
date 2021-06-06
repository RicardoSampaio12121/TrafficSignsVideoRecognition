/*
Autores:
-Filipe Gajo
-Ricardo Sampaio
-Cláudio Silva
*/

// Se VC_DEBUG estiver definido e não comentado, quando acontecerem erros aparecem mensagens de erro.
// Caso contrário não aparecem mensagens de erro.
#define VC_DEBUG

#define MAX(a,b) (a > b ? a : b) // Macro para calcular o máximo
#define MIN(a,b) (a < b ? a : b) // Macro para calcular o mínimo

// enum para representar os vários sinais de trânsito identificados
typedef enum {
	INDEFINIDO, // Valor para inicializações
	VIRAR_E, // Sentido obrigatório (esquerda)
	VIRAR_D,// Sentido obrigatório (direita)
	AUTO_ESTRADA, // Auto-estrada
	AUTOMOVEIS_MOTOCICLOS, // Via reservada a automóveis e motociclos
	SENTIDO_PROIBIDO, // Sentido proibido
	STOP, // Paragem obrigatória em cruzamentos ou entroncamentos
} Sinal;

// enum para representar a cor principal dos sinais de trânsito
typedef enum {
	INDEFINIDA, // Valor para inicializações
	AZUL,
	VERMELHO,
} Cor;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	unsigned char* data;    // aqui são armazenados os dados de cada píxel
	int width, height;      // largura e altura da imagem
	int channels;			// Binário/Cinzentos=1; RGB=3 (a cores)
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;                      // IVC = Imagem de Visão por Computador

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	// x e y são o canto superior esquerdo da caixa delimitadora
	int area;					// Área
	int xc, yc;					// Centro-de-massa (xc = x centro; yc = y centro)
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC; // OVC = Objeto de Visão por Computador

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM [imagens não existentes]
IVC* vc_image_new(int width, int height, int channels, int levels); // o levels é o nível máximo e não o número de níveis
IVC* vc_image_free(IVC* image);

// FUNÇÃO: ESCRITA DE IMAGENS (PBM, PGM E PPM) [imagens existentes]
int vc_write_image(char* filename, IVC* image);

// FUNÇÃO: CONVERTE IMAGEM BGR PARA IMAGEM HSV
int vc_bgr_to_hsv(IVC* src, IVC* dst);

// FUNÇÃO: SELECIONA PARTES DE UMA IMAGEM DE ACORDO COM A COR ESCOLHIDA
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin,
	int smax, int vmin, int vmax);

// FUNÇÃO: SELECIONA PARTES DE UMA IMAGEM DE ACORDO COM A COR ESCOLHIDA (DOIS INTERVALOS DE TONALIDADE)
int vc_hsv_red_segmentation(IVC* src, IVC* dst, int hmin1, int hmax1, int hmin2, int hmax2,
	int smin, int smax, int vmin, int vmax);

// Recebe imagem binária e devolve imagem em tons de cinzento (etiquetada)
// nlabels = quantos objetos encontrou (apontador para poder alterar nlabels como vem em arg da função)
// (tem de ser apontador para podermos alterar)
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);

// FUNÇÃO: CALCULA A ÁREA DE CADA BLOB E IDENTIFICA O MAIOR
// (src = imagem já etiquetada, proveniente de vc_binary_blob_labelling)
int vc_encontrarMaiorBlob(IVC* src, OVC* blobs, int nblobs, int* maiorBlob);

// FUNÇÃO: PARA O MAIOR BLOB, CALCULA O CENTRO DE MASSA, A CAIXA DELIMITADORA E O PERÍMETRO
// (src = imagem já etiquetada, proveniente de vc_binary_blob_labelling)
int vc_maiorBlob_info(IVC* src, OVC* blobs, int nblobs, int maiorBlob);

// FUNÇÃO: MARCA A CAIXA DELIMITADORA E O CENTRO DE MASSA DO MAIOR BLOB NUMA NOVA IMAGEM
// (definido só para imagens a cores)
int vc_marcarMaiorBlob(IVC* src, IVC* dst, OVC* blobs, int nblobs, int maiorBlob);

// FUNÇÃO: IDENTIFICA O SINAL DE TRÂNSITO
Sinal vc_identificarSinal(OVC* blobs, int nblobs, int maiorblob, Cor cor);

// FUNÇÃO: ORDENA UM ARRAY COM O ALGORITMO DE INSERTION SORT
void vc_insertionSort(int array[], int tamanho);

// FUNÇÃO: FILTRO DE MEDIANA (PASSA-BAIXO)
// (elimina ruído "salt-and-pepper")
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernelsize);