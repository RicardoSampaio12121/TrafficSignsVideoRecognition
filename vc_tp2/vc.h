/*
Author: Filipe Gajo
Author: Ricardo Sampaio
Author: Claudio Silva
*/

// Se VC_DEBUG estiver definido e não comentado, quando acontecerem erros aparecem mensagens de erro.
// Caso contrário não aparecem mensagens de erro.
#define VC_DEBUG

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
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM [imagens não existentes]
IVC* vc_image_new(int width, int height, int channels, int levels); // o levels é o nível máximo e não o número de níveis
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM) [imagens existentes]
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

// FUNÇÕES: CÁLCULO DOS NEGATIVOS DE IMAGENS
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);

// FUNÇÕES: EXTRAI AS COMPONENTES RGB PARA UMA IMAGEM EM TONS DE CINZENTO
int vc_rgb_get_red_gray(IVC* srcdst);
int vc_rgb_get_green_gray(IVC* srcdst);
int vc_rgb_get_blue_gray(IVC* srcdst);

// FUNÇÕESS: EXTRAI AS COMPONENTES BGR PARA UMA IMAGEM EM TONS DE CINZENTO
int vc_bgr_get_blue_gray(IVC* srcdst);

// FUNÇÃO: CONVERTE IMAGEM RGB PARA IMAGEM EM TONS DE CINZENTO
int vc_rgb_to_gray(IVC* src, IVC* dst);

// FUNÇÃO: CONVERTE IMAGEM RGB PARA IMAGEM HSV
int vc_rgb_to_hsv(IVC* src, IVC* dst);

// FUNÇÃO: CONVERTE IMAGEM BGR PARA IMAGEM HSV
int vc_bgr_to_hsv(IVC* src, IVC* dst);

// FUNÇÃO: SELECIONA PARTES DE UMA IMAGEM DE ACORDO COM A COR ESCOLHIDA
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin,
	int smax, int vmin, int vmax);

// FUNÇÃO: CONVERTE IMAGEM EM ESCALA DE CINZENTOS PARA IMAGEM RGB (imagens térmicas)
int vc_scale_gray_to_rgb(IVC* src, IVC* dst);

// FUNÇÃO: REALIZA A BINARIZAÇÃO, POR THRESHOLDING MANUAL, DE UMA IMAGEM EM TONS DE CINZENTO
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);

// FUNÇÃO: REALIZA A BINARIZAÇÃO, POR THRESHOLDING AUTOMÁTICO, DE UMA IMAGEM EM TONS DE CINZENTO
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);

// FUNÇÃO: REALIZA A BINARIZAÇÃO, POR THRESHOLDING AUTOMÁTICO MIDPOINT, DE UMA IMAGEM EM TONS DE CINZENTO
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);

// FUNÇÃO: REALIZA A DILATAÇÃO DE UMA IMAGEM BINÁRIA
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);

// FUNÇÃO: REALIZA A EROSÃO DE UMA IMAGEM BINÁRIA
int vc_binary_erode(IVC* src, IVC* dst, int kernel);

// FUNÇÃO: REALIZA A ABERTURA BINÁRIA
int vc_binary_open(IVC* src, IVC* dst, int kernelErosion, int kernelDilation);

// FUNÇÃO: REALIZA O FECHO BINÁRIA
int vc_binary_close(IVC* src, IVC* dst, int kernelErosion, int kernelDilation);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a,b) (a > b ? a : b) // Macro para calcular o máximo
#define MIN(a,b) (a < b ? a : b) // Macro para calcular o mínimo

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	// x e y são o canto superior esquerdo do blob
	int area;					// Área
	int xc, yc;					// Centro-de-massa (xc = x centro; yc = y centro)
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC; // OVC = Objeto de Visão por Computador

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Recebe imagem binária e devolve imagem em tons de cinzento (etiquetada)
// nlabels = quantos objetos encontrou (apontador para poder alterar nlabels como vem em arg da função, 
// tem de ser apontador para podermos alterar)
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
// Recebe imagem já etiquetada
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);

// FUNÇÃO: EXIBE O HISTOGRAMA DE UMA IMAGEM EM TONS DE CINZENTO
int vc_gray_histogram_show(IVC* src, IVC* dst);

// FUNÇÃO: REALIZA A EQUALIZAÇÃO DE HISTOGRAMA DE UMA IMAGEM EM TONS DE CINZENTO
int vc_gray_histogram_equalization(IVC* src, IVC *dst);

// FUNÇÃO: DESENHA CONTORNOS DA IMAGEM EM TONS DE CINZENTO, UTILIZANDO OS OPERADORES DE PREWITT
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th);

// FUNÇÃO: DESENHA CONTORNOS DA IMAGEM EM TONS DE CINZENTO, UTILIZANDO OS OPERADORES DE SOBEL
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th);

// FUNÇÃO: DESENHA CONTORNOS DA IMAGEM EM TONS DE CINZENTO, UTILIZANDO OS OPERADORES DE LAPLACE (2º DERIVADA)
int vc_gray_edge_laplace(IVC* src, IVC* dst);

// FUNÇÃO: FILTRO DE MÉDIA (PASSA-BAIXO)
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize);

// FUNÇÃO: ORDENA UM ARRAY COM O ALGORITMO DE INSERTION SORT
void vc_insertionSort(int array[], int tamanho);

// FUNÇÃO: FILTRO DE MEDIANA (PASSA-BAIXO)
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernelsize);

// FUNÇÃO: FILTRO GAUSSIANO (PASSA-BAIXO)
int vc_gray_lowpass_gaussian_filter(IVC* src, IVC* dst);

// FUNÇÃO: FILTRO BÁSICO (PASSA-ALTO)
int vc_gray_highpass_filter(IVC* src, IVC* dst);

// FUNÇÃO: APLICAÇÃO DO FILTRO BÁSICO (PASSA-ALTO)
int vc_gray_highpass_filter_enhance(IVC* src, IVC* dst, int gain);

int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    IDENTIFICAÇÃO DE SINAIS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int vc_transit_signal_identifier(IVC* src);