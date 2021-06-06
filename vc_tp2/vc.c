/*
Autores:
-Filipe Gajo
-Ricardo Sampaio
-Cláudio Silva
*/

// MSVC++ = MicroSoft Visual studio C++
/* ex: fopen não é seguro pois é uma versão obsoleta. Foi substituído pela função da microsoft fopen_s (fopen_s = fopen safe)
   fopen_s é uma variação de fopen que contém validação de parâmetros e devolve um código de erro em vez de um apontador
   no caso de algo correr mal no processo de abertura. É mais segura que a variante original pois tem em conta mais cenários de "erro" */
   // Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> // Funções de input/output (exs: printf, fopen)
#include <ctype.h> // Funções para testagem e manipulação de caracteres(exs: isdigit, tolower)
#include <string.h> // Funções para manipulação de arrays de caracteres(strings) (exs: strcmp, strlen)
#include <malloc.h> // Header obsoleto. Substituído por stdlib.h (ex: malloc)
#include "vc.h" // Header com as declarações das funções de Visão por Computador que definimos
#include <math.h> // Funções matemáticas (exs: pow, sqrt)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Função: vc_image_new
 * ----------------------------
 *	 Aloca memória para uma imagem
 *
 *	 width: 	largura
 *	 height: 	altura
 *	 channels:  número de canais
 *	 levels: 	níveis de cor
 */
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		// Liberta memória e return NULL
		return vc_image_free(image);
	}

	// Devolve apontador da imagem criada
	return image;
}

/*
 * Função: vc_image_free
 * ----------------------------
 *	 Liberta memória de uma imagem
 *
 *	 image: endereço de memória da imagem
 */
IVC* vc_image_free(IVC* image)
{
	// Só se liberta a memória se a imagem existir
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Função: unsigned_char_to_bit
 * ----------------------------
 *	 ||| Comprime 1 byte/píxel para 1 bit/píxel (= 1 byte(8bits)/8 píxeis) |||
 * 	 [Só está a ser usada em imagens PBM (0 ou 1 em cada píxel) pois só nessas
 * 	 é que se pode converter em 1 bit/píxel (só 0 ou 1)]
 *
 *   datauchar:     array com os pixeis
 *   databit:       array auxiliar vazio onde serão armazenados os pixeis comprimidos
 *   width:		    comprimento
 *	 height:		largura
 */
long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	// lowestLabel array de píxeis: y = fila || x = coluna
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	// O byte começa a 0 -> 000 000 00
	*p = 0;
	// Para fazer 8 - 1 no princípio (7 shifts esquerda = array[0] pois tamanho = 8)
	countbits = 1;
	counttotalbytes = 0;

	// Percorre todos os píxeis
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// lowestLabela imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem (na função create daqui):
				// 1 = Branco
				// 0 = Preto

				// Passos:
				// 1º: Verificar se o píxel atual é = 0 -> Preto (datauchar[pos] == 0)
				// 2º: Se for 0, faz-se um shift para a 1º posição livre  a contar da esquerda (8 - countbits) de um 1 (datauchar[pos] == 0)
				// 2.5º: Se não for 0, o shift que se faz é de um 0 e não muda nada pois *p = 0 (000 000 00)
				// 3º: A operação binária OR de *p | com o resultado do shift anterior junta o 1 do shift ao *p (junta o novo píxel preto na sua posição)
				* p |= (datauchar[pos] == 0) << (8 - countbits);
				// igual a: *p = *p | (datauchar[pos] == 0) << (8 - countbits); 

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				// Primeiro bit a 0
				*p = 0;
				countbits = 1;
				// a contar o número total de bytes da imagem
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

/*
 * Função: vc_write_image
 * ----------------------------
 *	 Escreve uma imagem da estrutura IVC
 *
 *	 filename:	caminho + nome para guardar a imagem
 *	 image:		endereço de memória da imagem
 */
int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	// "wb" = write binary
	if ((file = fopen(filename, "wb")) != NULL)
	{
		// Só existe preto e branco
		if (image->levels == 1)
		{
			// image->height + 1 para dar mais uma linha no princípio para preencher com o número mágico, largura e altura da imagem
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			// int fprintf(FILE *stream, const char *format, ...)
			// Começa por escrever no princípio do ficheiro/imagem o número mágico, a largura e a altura da imagem
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			// guarda-se byte a byte porque pode não ser múltiplo de 8 (o total bytes)
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes) // verificar se guardou todos os bytes (totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		// Mais de um nível (256)
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			// fwrite devolve o número de elementos escritos
			// Pode-se guardar linha a linha (porque o tamanho total é múltiplo de 8)
			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES NECESSÁRIAS PARA O TRABALHO (TP2)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
 * Função: vc_bgr_to_hsv
 * ----------------------------
 *	 Converte uma imagem bgr para hsv
 *
 *	 src:		estrutura da imagem de origem
 *	 dst:		estrutura da imagem de saida
 */
int vc_bgr_to_hsv(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || (dst->data == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i += channels)
	{
		b = (float)data[i];
		g = (float)data[i + 1];
		r = (float)data[i + 2];

		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0, 255]
		value = rgb_max;
		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0, 1]
			saturation = ((rgb_max - rgb_min) / rgb_max);

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0, 360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else // rgb_max == b
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0, 255]
		dst->data[i] = (unsigned char)(hue / 360.0f * 255.0f);
		dst->data[i + 1] = (unsigned char)(saturation * 255.0f);
		dst->data[i + 2] = (unsigned char)(value);
	}

	return 1;
}

/*
* Função: vc_hsv_segmentation
* ----------------------------
* Faz a segmentação de uma imagem hsv pelos valores dados
*
* src   : estrutura da imagem de origem
* dst : estrutura da imagem de saida
* hmin  : valor minimo para o valor de hue
* hmax  : valor maximo para o valor de hue
* smin  : valor minimo para a saturação
* smax  : valor maximo para a saturação
* vmin  : valor minimo para o value
* vmax  : valor maximo para o value
*/
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int hue, saturation, value;
	long int pos_src, pos_dst;
	int x, y;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || datadst == NULL) return 0;
	if ((width != dst->width) || (height != dst->height)) return 0;
	if ((channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels;
			pos_dst = y * bytesperline_dst + x; // * canais = 1

			hue = (int)((float)data[pos_src] / 255.0f * 360.0f);
			saturation = (int)((float)data[pos_src + 1] / 255.0f * 100.0f);
			value = (int)((float)data[pos_src + 2] / 255.0f * 100.0f);

			if (hue >= hmin && hue <= hmax &&
				saturation >= smin && saturation <= smax &&
				value >= vmin && value <= vmax)
				datadst[pos_dst] = (unsigned char)255;
			else datadst[pos_dst] = (unsigned char)0;
		}
	}

	return 1;
}

/*
* Função: vc_hsv_red_segmentation
* ----------------------------
* Faz a segmentação de uma imagem hsv pelos valores dados
* sendo possivel dar duas gamas para hue
*
* src   : estrutura da imagem de origem
* dst : estrutura da imagem de saida
* hmin1 : valor minimo para o valor de hue para a primeira gama
* hmax1 : valor maximo para o valor de hue para a primeira gama
* hmin2 : valor minimo para o valor de hue para a segunda gama
* hmax2 : valor maximo para o valor de hue para a segunda gama
* smin  : valor minimo para a saturação
* smax  : valor maximo para a saturação
* vmin  : valor minimo para o value
* vmax  : valor maximo para o value
*/
int vc_hsv_red_segmentation(IVC* src, IVC* dst, int hmin1, int hmax1, int hmin2, int hmax2, int smin, int smax, int vmin, int vmax)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int hue, saturation, value;
	long int pos_src, pos_dst;
	int x, y;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || datadst == NULL) return 0;
	if ((width != dst->width) || (height != dst->height)) return 0;
	if ((channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels;
			pos_dst = y * bytesperline_dst + x; // * canais = 1

			hue = (int)((float)data[pos_src] / 255.0f * 360.0f);
			saturation = (int)((float)data[pos_src + 1] / 255.0f * 100.0f);
			value = (int)((float)data[pos_src + 2] / 255.0f * 100.0f);

			if (((hue >= hmin1 && hue <= hmax1) || (hue >= hmin2 && hue <= hmax2)) &&
				saturation >= smin && saturation <= smax &&
				value >= vmin && value <= vmax)
				datadst[pos_dst] = (unsigned char)255;
			else datadst[pos_dst] = (unsigned char)0;

		}
	}
}


/*
* Função: vc_binary_blob_labelling
* ----------------------------
* Faz o etiquetamento de uma imagem segmentada
*
* src      : estrutura da imagem de origem
* dst	   : estrutura da imagem de saida
* nlabels  : número de objetos encontrados na imagem
*/
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b; // a e b são variáveis auxiliares para percorrer tabela das etiquetas
	long int i, size; // i = variável auxiliar para percorrer imagem original, etc.
	// long int posX, posA, posB, posC, posD;
	long int posX;
	int A, B, C, D;
	int labeltable[256] = { 0 }; // { 0 } inicializa todas as posições do array a 0
	int label = 1; // Etiqueta inicial.
	int lowestLabel, tmplabel; // lowestLabel = var auxiliar para etiqueta
	// tmplabel = var auxiliar para percorrer tabela de etiquetas (para ser mais fácil de ler: usa um nome diferente)
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{ // para dar se tivermos píxeis com valores iguais a: 0 ou 1 || 0 ou 255
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	// (porque não há objetos só com 1 píxel)
	for (y = 0; y < height; y++) // Limpa rebordos verticais 
	{
		datadst[y * bytesperline + 0 * channels] = 0; // Limpa rebordo vertical esquerdo
		datadst[y * bytesperline + (width - 1) * channels] = 0; // Limpa rebordo vertical direito
	}
	for (x = 0; x < width; x++) // Limpa rebordos horizontais
	{
		datadst[0 * bytesperline + x * channels] = 0; // Limpa rebordo horizontal superior
		datadst[(height - 1) * bytesperline + x * channels] = 0; // Limpa rebordo horizontal inferior
	}

	// Efetua a etiquetagem
	// (Para cada píxel...)
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Píxel a ser analisado X (Kernel:)
			// Vizinhos A,B,C,D
			// A | B | C 
			// D | X | 

			A = (int)datadst[(y - 1) * bytesperline + (x - 1) * channels]; // A
			B = (int)datadst[(y - 1) * bytesperline + x * channels]; // B
			C = (int)datadst[(y - 1) * bytesperline + (x + 1) * channels]; // C
			D = (int)datadst[y * bytesperline + (x - 1) * channels]; // D
			posX = y * bytesperline + x * channels; // X

			// Se o píxel a ser analisado for branco (encontramos parte de um blob)
			if (datadst[posX] != 0)
			{ // Se vizinhos A,B,C,D = 0
				if ((A == 0) && (B == 0) && (C == 0) && (D == 0))
				{
					datadst[posX] = label; // Encontrou um novo objeto (atribui-lhe nova etiqueta)
					labeltable[label] = label; // Adiciona label à tabela das etiquetas
					//label++; // Limitar aqui ???
					if (label < 255) label++;
				}
				else // píxel X = etiqueta de vizinho A,B,C,D com menor valor (não incluindo 0)
				{
					lowestLabel = 255; // Etiqueta de menor valor (inicialização)

					// Se A está marcado
					if (A != 0) lowestLabel = labeltable[A];
					// Se B está marcado, e é menor que a etiqueta "lowestLabel"
					if ((B != 0) && (labeltable[B] < lowestLabel)) lowestLabel = labeltable[B];
					// Se C está marcado, e é menor que a etiqueta "lowestLabel"
					if ((C != 0) && (labeltable[C] < lowestLabel)) lowestLabel = labeltable[C];
					// Se D está marcado, e é menor que a etiqueta "lowestLabel"
					if ((D != 0) && (labeltable[D] < lowestLabel)) lowestLabel = labeltable[D];

					// Atribui a etiqueta ao píxel
					datadst[posX] = lowestLabel;
					labeltable[lowestLabel] = lowestLabel;

					// Atualiza a tabela de etiquetas
					if (A != 0) // Se o vizinho A está marcado
					{
						if (labeltable[A] != lowestLabel) // Se o vizinho tiver uma etiqueta diferente da do píxel atual (X)
						{
							tmplabel = labeltable[A];
							for (a = 1; a < label; a++) // Percorre os píxeis já etiquetados na tabela das etiquetas
							{ // Se esta etiqueta do vizinho A for encontrada
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = lowestLabel; // Atualiza a etiqueta na tabela para o correto
								}
							}
						}
					}

					if (B != 0) // Se o vizinho B está marcado
					{
						if (labeltable[B] != lowestLabel)
						{
							tmplabel = labeltable[B];
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = lowestLabel;
								}
							}
						}
					}

					if (C != 0) // Se o vizinho C está marcado
					{
						if (labeltable[C] != lowestLabel)
						{
							tmplabel = labeltable[C];
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = lowestLabel;
								}
							}
						}
					}

					if (D != 0) // Se o vizinho D está marcado
					{
						if (labeltable[D] != lowestLabel)
						{
							tmplabel = labeltable[D];
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = lowestLabel;
								}
							}
						}
					}

				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{ // Passa as etiquetas da tabela para os valores nos píxeis
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++) // Para cada etiqueta
	{
		for (b = a + 1; b < label; b++) // Compara com todas as etiquetas à frente
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}

	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{ // Escreve por cima nos primeiros índices. No fim, os índices > que *nlabels são "lixo"
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

/*
* Função: vc_encontrarMaiorBlob
* ----------------------------
* Encontra a blob com a maior área
*
* src       : estrutura da imagem etiquetada
* blobs	    : estrutura das blobs
* nblobs    : número de objetos encontrados na imagem
* maiorBlob : endereço de memória para guardar o index da maior blob na estrutura
*/
int vc_encontrarMaiorBlob(IVC* src, OVC* blobs, int nblobs, int* maiorBlob)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i, blobAreaMax = 0;
	long int pos;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if ((blobs == NULL) || (nblobs <= 0)) return 0;
	if (channels != 1) return 0;

	// Percorre cada blob
	for (i = 0; i < nblobs; i++)
	{
		blobs[i].area = 0;

		// Percorre cada píxel da imagem etiquetada
		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x; // * channels; (channels = 1)

				// Se o píxel pertencer ao blob de que estamos à procura
				if (data[pos] == blobs[i].label)
				{
					// Área (= somatório do nº de píxeis do blob)
					blobs[i].area++;
				}
			}
		}

		// Compara o tamanho do blob atual com o maior até agora
		if (blobs[i].area > blobAreaMax)
		{ // Este blob é agora o maior
			blobAreaMax = blobs[i].area;
			*maiorBlob = i;
		}
	}
	return 1;
}

/*
* Função: vc_maiorBlob_info
* ----------------------------
* Calcula o perimetro, centro de massa e caixa delimitadora de uma blob
*
* src       : estrutura da imagem etiquetada
* blobs	    : estrutura das blobs
* nblobs    : número de objetos encontrados na imagem
* maiorBlob : index da maior blob na estrutura
*/
int vc_maiorBlob_info(IVC* src, OVC* blobs, int nblobs, int maiorBlob)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, label;
	long int pos;
	int xmin, xmax, ymin, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 1) return 0;
	if ((blobs == NULL) || (nblobs <= 0) || (maiorBlob >= nblobs)) return 0;

	// Inicialização nos extremos possíveis
	xmin = width - 1;
	ymin = height - 1;
	xmax = 0;
	ymax = 0;

	sumx = 0;
	sumy = 0;

	label = blobs[maiorBlob].label;

	// Percorre cada píxel da imagem etiquetada
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x; // *channels; (channels = 1)

			// Se o píxel pertencer ao blob de que estamos à procura
			if (data[pos] == label)
			{
				// Somatórios que vão ser usados no cálculo do centro de massa
				sumx += x;
				sumy += y;

				// Coordenadas dos vértices da caixa delimitadora
				if (xmin > x) xmin = x;
				if (ymin > y) ymin = y;
				if (xmax < x) xmax = x;
				if (ymax < y) ymax = y;

				// Perímetro (está a usar em "cruz", não conta os da diagonal)
				// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
				if ((data[pos - 1] != label) || (data[pos + 1] != label) || (data[pos - bytesperline] != label) || (data[pos + bytesperline] != label))
				{
					blobs[maiorBlob].perimeter++;
				}
			}
		}
	}

	// Caixa delimitadora
	blobs[maiorBlob].x = xmin;
	blobs[maiorBlob].y = ymin;
	blobs[maiorBlob].width = (xmax - xmin) + 1;
	blobs[maiorBlob].height = (ymax - ymin) + 1;

	// Centro de Massa
	// x médio (média de todos os valores/coordenadas em x)
	blobs[maiorBlob].xc = round((float)sumx / (float)MAX(blobs[maiorBlob].area, 1)); // Usa-se o MAX para nunca dar 0
	// y médio (média de todos os valores/coordenadas em y)
	blobs[maiorBlob].yc = round((float)sumy / (float)MAX(blobs[maiorBlob].area, 1));

	return 1;
}


/*
* Função: vc_marcarMaiorBlob
* ----------------------------
* Marca o centro de massa e a caixa delimitadora de uma blob
*
* src       : estrutura da imagem original(neste caso da camara)
* dst	    : estrutura da imagem de saida
* blobs		: estrutura das blobs
* nblobs    : número de objetos encontrados na imagem
* maiorBlob : index da maior blob na estrutura
*/
int vc_marcarMaiorBlob(IVC* src, IVC* dst, OVC* blobs, int nblobs, int maiorBlob)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int bytesperline = src->bytesperline;
	long int pos;
	int x, y, blobXmin, blobXmax, blobYmin, blobYmax, tamanhoCentro = 2;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if ((blobs == NULL) || (nblobs <= 0) || (maiorBlob >= nblobs)) return 0;
	if (channels != 3) return 0;

	// Copia dados da imagem original para a nova imagem
	memcpy(datadst, datasrc, bytesperline * height);

	// Marcar o centro de massa
	for (y = blobs[maiorBlob].yc - tamanhoCentro; y <= blobs[maiorBlob].yc + tamanhoCentro; y++)
	{
		for (x = blobs[maiorBlob].xc - tamanhoCentro; x <= blobs[maiorBlob].xc + tamanhoCentro; x++)
		{
			pos = y * bytesperline + x * channels;

			if (y >= 0 && y < height && x >= 0 && x < width)
			{
				datadst[pos] = (unsigned char)0;
				datadst[pos + 1] = (unsigned char)0;
				datadst[pos + 2] = (unsigned char)0;
			}

		}
	}

	// Coordenadas da caixa delimitadora
	blobYmin = blobs[maiorBlob].y;
	blobYmax = blobYmin + blobs[maiorBlob].height - 1;
	blobXmin = blobs[maiorBlob].x;
	blobXmax = blobXmin + blobs[maiorBlob].width - 1;

	// Marcar a caixa delimitadora
	// Limites verticais
	for (y = blobYmin; y <= blobYmax; y++)
	{
		// Limite esquerdo
		pos = y * bytesperline + blobXmin * channels;
		datadst[pos] = (unsigned char)0;
		datadst[pos + 1] = (unsigned char)0;
		datadst[pos + 2] = (unsigned char)0;

		// Limite direito
		pos = y * bytesperline + blobXmax * channels;
		datadst[pos] = (unsigned char)0;
		datadst[pos + 1] = (unsigned char)0;
		datadst[pos + 2] = (unsigned char)0;
	}
	// Limites horizontais
	for (x = blobXmin; x <= blobXmax; x++)
	{
		// Limite superior
		pos = blobYmin * bytesperline + x * channels;
		datadst[pos] = (unsigned char)0;
		datadst[pos + 1] = (unsigned char)0;
		datadst[pos + 2] = (unsigned char)0;

		// Limite inferior
		pos = blobYmax * bytesperline + x * channels;
		datadst[pos] = (unsigned char)0;
		datadst[pos + 1] = (unsigned char)0;
		datadst[pos + 2] = (unsigned char)0;
	}

	return 1;
}

/*
* Função: vc_identificarSinal
* ----------------------------
* Identifica o sinal de transito da imagem
*
* blobs		: estrutura das blobs
* nblobs    : número de objetos encontrados na imagem
* maiorBlob : index da maior blob na estrutura
* cor		: enum com a cor do sinal(azul ou vermelho)
*/
Sinal vc_identificarSinal(OVC* blobs, int nblobs, int maiorblob, Cor cor)
{
	float perimetroCaixa, perimetroBlob, proporcaoPerimetros;
	int centroXCaixa;
	// Inicialização do sinal
	Sinal sinal = INDEFINIDO;

	// Verificação de erros
	if ((blobs == NULL) || (nblobs <= 0) || (maiorblob >= nblobs)) return INDEFINIDO;
	if (cor == INDEFINIDA) return INDEFINIDO;

	// Divisão do perímetro do maior blob pela perímetro da caixa delimitadora
	perimetroCaixa = blobs[maiorblob].width + blobs[maiorblob].height; // *2 para ser mesmo perímetro
	perimetroBlob = blobs[maiorblob].perimeter;
	proporcaoPerimetros = perimetroBlob / perimetroCaixa;

	// Se for um sinal azul
	if (cor == AZUL)
	{
		// (Valores aproximados +/- 0.2)

		// Obrigatório virar à direita/esquerda: proporção de perímetros min: 2.4 max: 2.7
		if (proporcaoPerimetros >= 2.2f && proporcaoPerimetros <= 2.9f)
		{
			// X central da caixa delimitadora
			centroXCaixa = blobs[maiorblob].x + blobs[maiorblob].width / 2;

			// Se o centro de massa está à esquerda do centro da caixa delimitadora... 
			if (blobs[maiorblob].xc < centroXCaixa) sinal = VIRAR_E;
			else sinal = VIRAR_D;
		}

		// Automóveis e motociclos: proporção de perímetros min: 3.2 max: 3.4
		else if (proporcaoPerimetros >= 3.0f && proporcaoPerimetros <= 3.6f) sinal = AUTOMOVEIS_MOTOCICLOS;

		// Auto-estrada: proporção de perímetros min: 4.5 max: 4.6
		else if (proporcaoPerimetros >= 4.3f && proporcaoPerimetros <= 4.8f) sinal = AUTO_ESTRADA;
	}

	else if (cor == VERMELHO)
	{
		// Sentido proibido: proporção de perímetros min: 2.4 max: 2.4
		if (proporcaoPerimetros >= 2.2f && proporcaoPerimetros < 2.6f) sinal = SENTIDO_PROIBIDO;

		// STOP: proporção de perímetros min: 3.1 max: 3.2
		else if (proporcaoPerimetros >= 2.9f && proporcaoPerimetros < 3.4f) sinal = STOP;
	}

	// Identificar a forma de acordo com o cálculo anterior
	//printf("Razao Perimetros = %f\n\n", proporcaoPerimetros);

	return sinal;
}

/*
* Função: vc_insertionSort
* ----------------------------
* Algoritmo de organização de arrays
*
* array[] : array a ordenar
* tamanho : tamanho do array
*/
void vc_insertionSort(int array[], int tamanho)
{
	int i, j, atual;

	// Percorrer elementos não ordenados
	for (i = 1; i < tamanho; i++)
	{
		// Elemento a ser comparado com os anteriores
		atual = array[i];

		j = i - 1;
		// Percorrer elementos ordenados com índice menor que o atual e valor maior que o atual
		while (j >= 0 && array[j] > atual)
		{
			// Move o elemento um índice para a frente
			array[j + 1] = array[j];
			// Continua a percorrer de forma descendente
			j--;
		}
		// Finalmente, coloca-se o elemento atual na posição correta na parte ordenada do array
		array[j + 1] = atual;
	}
}

/*
* Função: vc_gray_lowpass_median_filter
* -------------------------------------
* Algoritmo de organização de arrays
*
* src		 : estrutura da imagem de entrada
* dst		 : estrutura da imagem de saida
* kernelsize : tamanho do kernel
*/
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernelsize)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky, vizinhosCount = 0, centro;
	int offset = (kernelsize - 1) / 2, tamanhoVizinhos = pow(kernelsize, 2);
	long int pos, posk;
	int* vizinhos = (int*)malloc(sizeof(int) * tamanhoVizinhos);

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;
	if ((kernelsize <= 1) || (kernelsize % 2 == 0)) return 0; // Kernel tem que ser > 1 e ímpar

	// Percorrer píxeis da imagem original
	for (y = 1; y < height; y++)
	{
		for (x = 1; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			// Percorrer Vizinhos (kernel)
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						// Adicionar ao array de vizinhos
						vizinhos[vizinhosCount] = (int)data[posk];
						vizinhosCount++;
					}
				}
			}

			// Ordenar vizinhos de acordo com o seu valor
			vc_insertionSort(vizinhos, vizinhosCount);

			// Dar o valor central (mediana)
			centro = tamanhoVizinhos / 2;

			datadst[pos] = (unsigned char)vizinhos[centro];

			vizinhosCount = 0;
		}
	}

	free(vizinhos);

	return 1;
}