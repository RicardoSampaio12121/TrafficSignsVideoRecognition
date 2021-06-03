/*
Author: Filipe Gajo
Author: Ricardo Sampaio
Author: Claudio Silva
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
#include "vc.h"
#include <math.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar memória para uma imagem
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

// Libertar memória de uma imagem
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
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// RECEBE um ficheiro, um endereço para uma string e um comprimento e DEVOLVE uma string
char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	// Loop infinito (para passar à frente espaços em branco e comentários iniciais)
	for (;;)
	{
		// isspace(char c): verifica se o caracter passado como argumento é um espaço ' ' ou '\t', '\n', '\v', '\f', '\r' 
		//  int getc(FILE* stream): obtém o caracter atual e avança o indicador da posição na stream
		// Passa os espaços em branco/irrelevantes à frente e guarda o último caracter lido 
		while (isspace(c = getc(file)));
		// Só continua se for um comentário
		if (c != '#') break;
		do c = getc(file);
		// Passa o comentário à frente
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	// t passa a apontar para o apontador tok
	t = tok;

	// Se ainda não estamos no fim do ficheiro
	if (c != EOF)
	{
		// Nesta parte, lê-se a informação útil, caracter a caracter
		do
		{
			// ESPECIAL: *t++ is parsed as *(t++)
			// TRADUÇÃO (de *t++ = c;):
			// *t = c;
			// t++;
			*t++ = c;
			c = getc(file);
			// Continua até aparecer um espaço, um comentário, o ficheiro acabar ou o tamanho da string em tok for maior ou igual a len
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		// int ungetc(int char, FILE* stream): na posição atual da stream, substitui o caracter selecionado por char (não no ficheiro) para poder ser lido
		// (não escreve no ficheiro, apenas pode ser lido na próxima leitura da stream, enquanto se estiver a ler)
		// Nesta linha de código, se o c for um '#', o ungetc substitui com ele próprio.
		// Neste caso a função só é usada para retroceder o indicador da posição na stream para o '#' [1 pos para trás] (próximo a ser lido)
		if (c == '#') ungetc(c, file);
		// Isto é feito para não se começar a ler um comentário sem saber que o estamos a fazer (tínhamos passado '#' à frente)
	}

	// *t = 0 é a mesma coisa que *t = '\0';
	// Termina a string
	*t = 0;

	return tok;
}

// ||| Comprime 1 byte/píxel para 1 bit/píxel (= 1 byte(8bits)/8 píxeis) |||
// [Só está a ser usada em imagens PBM (0 ou 1 em cada píxel) pois só nessas é que se pode converter em 1 bit/píxel (só 0 ou 1)]
// datauchar = array com os píxeis (datauchar = data unsigned char)
// databit = array auxiliar vazio onde serão armazenados os píxeis comprimidos (databit = data bit)
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

// ||| Descomprime 1 bit/píxel (= 1 byte(8bits)/8 píxeis) para 1 byte/píxel |||
// databit = array com os píxeis (databit = data bit)
// datauchar = array auxiliar vazio onde serão armazenados os píxeis descomprimidos (datauchar = data unsigned char)
void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

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
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem (na função create daqui):
				// 1 = Branco
				// 0 = Preto

				// Passos:
				// 1º: Se na posição atual de *p (8 - countbits) tiver um píxel/bit a 1 (*p & (1 << (8 - countbits))), guarda um píxel preto em datauchar[pos] (? 0)
				// 2º: Se na posição atual de *p (8 - countbits) tiver um píxel/bit a 0 (*p & (1 << (8 - countbits))), guarda um píxel branco em datauchar[pos] (: 1)
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	// Nível máximo
	int levels = 255;
	int v;

	// Abre o ficheiro (em modo "rb" = read binary)
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efetua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic lowestLabelber!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			// Conversão do tamanho da imagem para o tamanho real da informação (1 byte = 8 píxeis -> image->width / 8)
			// Se não for múltiplo de 8 -> + (image->width % 8) ? 1 : 0)
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			// size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
			/*
			* ptr - onde se guarda o que se lê
			* size - tamanho de cada elemento que se vai ler
			* nmemb - número de elementos que se vai ler, cada um com tamanho size
			* stream - ficheiro que se vai ler
			* ptr - This is the pointer to a block of memory with a minimum size of size*nmemb bytes.
			* RETURN: número de elementos lidos com sucesso
			*/
			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			// Não é preciso [image->width / 8] pois 1 byte = 1píxel
			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

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

// Gerar negativo da imagem Gray
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Inverte a imagem Gray
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = (unsigned char)(255 - data[pos]);
		}
	}

	return 1;
}

// Gerar negativo da imagem RGB
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Inverte a imagem a cores
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = (unsigned char)(255 - data[pos]);
			data[pos + 1] = (unsigned char)(255 - data[pos + 1]);
			data[pos + 2] = (unsigned char)(255 - data[pos + 2]);
		}
	}

	return 1;
}

// Extrair a componente Red da imagem RGB para Gray
int vc_rgb_get_red_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Extrai a componente Red
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos + 1] = (unsigned char)data[pos]; // Green
			data[pos + 2] = (unsigned char)data[pos]; // Blue
		}
	}

	return 1;
}

// Extrair a componente Green da imagem RGB para Gray
int vc_rgb_get_green_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Extrai a componente Green
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = (unsigned char)data[pos + 1]; // Red
			data[pos + 2] = (unsigned char)data[pos + 1]; // Blue
		}
	}

	return 1;
}

// Extrair a componente Blue da imagem RGB para Gray
int vc_rgb_get_blue_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Extrai a componente Blue
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = (unsigned char)data[pos + 2]; // Red
			data[pos + 1] = (unsigned char)data[pos + 2]; // Green
		}
	}

	return 1;
}

// Extrair a componente Blue da imagem BGR para Gray
int vc_bgr_get_blue_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;
	int bytesperline = width * channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Extrai a componente Blue
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos + 1] = (unsigned char)data[pos]; // Green
			data[pos + 2] = (unsigned char)data[pos]; // Red
		}
	}

	return 1;
}

// Converter de RGB para Gray
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}

	return 1;
}

// BGR para HSV
int vc_bgr_to_hsv(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || dst->data == NULL) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 3 || dst->channels != 3) return 0;

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

// Selecionar partes de uma imagem de acordo com a cor escolhida
int OLD_vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin,
	int smax, int vmin, int vmax)
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
	if (width != dst->width || height != dst->height || channels != dst->channels) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels;
			pos_dst = y * bytesperline_dst + x * dst->channels;

			hue = (int)((float)data[pos_src] / 255.0f * 360.0f);
			saturation = (int)((float)data[pos_src + 1] / 255.0f * 100.0f);
			value = (int)((float)data[pos_src + 2] / 255.0f * 100.0f);

			if (hue >= hmin && hue <= hmax &&
				saturation >= smin && saturation <= smax &&
				value >= vmin && value <= vmax)
			{
				datadst[pos_dst] = (unsigned char)255;
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)255;
			}
			else
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)0;
				datadst[pos_dst + 2] = (unsigned char)0;
			}
		}
	}

	return 1;
}

// Selecionar partes de uma imagem de acordo com a cor escolhida
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin,
	int smax, int vmin, int vmax)
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
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 3 || dst->channels != 1) return 0;

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

// Selecionar partes de uma imagem de acordo com a cor escolhida (dois intervalos de tonalidade)
int vc_hsv_red_segmentation(IVC* src, IVC* dst, int hmin1, int hmax1, int hmin2, int hmax2,
	int smin, int smax, int vmin, int vmax)
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
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 3 || dst->channels != 1) return 0;

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

// Converter imagem em escala de cinzentos para imagem RGB (térmica)
int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	long int pos_src, pos_dst;
	int x, y, brilho, r, g, b;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || datadst == NULL) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 1 || dst->channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x; // * canais = 1
			pos_dst = y * bytesperline_dst + x * dst->channels;;

			brilho = (int)data[pos_src];

			if (brilho < 64)
			{
				r = 0;
				g = brilho * 4;
				b = 255;
			}
			else if (brilho < 128)
			{
				r = 0;
				g = 255;
				b = 255 - (brilho - 64) * 4;
			}
			else if (brilho < 192)
			{
				r = (brilho - 128) * 4;
				g = 255;
				b = 0;
			}
			else
			{
				r = 255;
				g = 255 - (brilho - 192) * 4;
				b = 0;
			}

			datadst[pos_dst] = (unsigned char)r;
			datadst[pos_dst + 1] = (unsigned char)g;
			datadst[pos_dst + 2] = (unsigned char)b;
		}
	}

	return 1;
}

// Realizar a binarização, por thresholding manual, de uma imagem em tons de cinzento
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	long int pos;
	int x, y;
	int bytesperline = src->width * src->channels;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || datadst == NULL) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 1 || dst->channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x; // * canais = 1

			if ((int)data[pos] < threshold) datadst[pos] = (unsigned char)0;
			else datadst[pos] = (unsigned char)255;
		}
	}

	return 1;
}

// Realizar a binarização, por thresholding automático, de uma imagem em tons de cinzento
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	long int pos;
	int x, y, brilhoTotal = 0;
	int bytesperline = src->width * src->channels;
	int threshold;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || datadst == NULL) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels != 1 || dst->channels != 1) return 0;

	// Somar os valores de brilho dos píxeis todos
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x; // * canais = 1

			brilhoTotal += (int)data[pos];
		}
	}

	// Calcular a média de brilho da imagem
	threshold = brilhoTotal / (width * height);

	// Aplicar threshold
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x; // * canais = 1

			if ((int)data[pos] < threshold) datadst[pos] = (unsigned char)0;
			else datadst[pos] = (unsigned char)255;
		}
	}

	return 1;
}

// Converter de Gray para Binário (threshold automático Midpoint)
// (kernel tem que ser ímpar e > 1)
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int max, min;
	long int pos, posk;
	unsigned char threshold;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer todos os píxeis da imagem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			// Posição do píxel que está a ser analisado
			pos = y * bytesperline + x * channels;

			max = 0; // Valor inicial de max é igual ao mínimo
			min = 255; // Valor inicial de min é igual ao máximo

			// Percorrer vizinhança do píxel que está a ser analisado
			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					// Se o píxel vizinho está dentro da imagem
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						// Posição do píxel vizinho que está a ser analisado
						posk = (y + ky) * bytesperline + (x + kx) * channels;

						if (datasrc[posk] > max) max = datasrc[posk];
						if (datasrc[posk] < min) min = datasrc[posk];
					}
				}
			}

			// T = 1/2 * (vmin + vmax)
			threshold = (unsigned char)((float)(max + min) / (float)2);

			if (datasrc[pos] > threshold) datadst[pos] = 255;
			else datadst[pos] = 0;
		}
	}

	return 1;
}

// Realizar a dilatação de uma imagem binária
// (kernel tem que ser ímpar e > 1)
int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2;
	long int pos, posk;
	int whiteFound;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			// Se o píxel que está a ser analisado for preto (pode mudar)
			if ((int)datasrc[pos] == 0)
			{
				// NxM Vizinhos
				for (ky = -offset, whiteFound = 0; ky <= offset && !whiteFound; ky++)
				{
					for (kx = -offset; kx <= offset && !whiteFound; kx++)
					{
						if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
						{
							posk = (y + ky) * bytesperline + (x + kx) * channels;

							// Se pelo menos um vizinho no kernel for branco
							if ((int)datasrc[posk] != 0) whiteFound = 1;
						}
					}
				}
				// Adicionar no centro branco
				if (whiteFound) datadst[pos] = (unsigned char)255;
				else datadst[pos] = (unsigned char)0;
			}
			else datadst[pos] = (unsigned char)255;
		}
	}

	return 1;
}

// Realizar a erosão de uma imagem binária
// (kernel tem que ser ímpar e > 1)
int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2;
	long int pos, posk;
	int blackFound;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			// Se o píxel a ser analisado for branco (pode mudar)
			if ((int)datasrc[pos] != 0)
			{
				// NxM Vizinhos
				for (ky = -offset, blackFound = 0; ky <= offset && !blackFound; ky++)
				{
					for (kx = -offset; kx <= offset && !blackFound; kx++)
					{
						if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
						{
							posk = (y + ky) * bytesperline + (x + kx) * channels;

							// Se pelo menos um vizinho no kernel for preto
							if ((int)datasrc[posk] == 0) blackFound = 1;
						}
					}
				}

				// Adicionar no centro preto
				if (blackFound) datadst[pos] = (unsigned char)0;
				else datadst[pos] = (unsigned char)255;
			}
			else datadst[pos] = (unsigned char)0;
		}
	}

	return 1;
}

// Realizar a abertura binária
int vc_binary_open(IVC* src, IVC* dst, int kernelErosion, int kernelDilation)
{
	int ret = 1;
	IVC* imageAux = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_binary_erode(src, imageAux, kernelErosion);
	ret &= vc_binary_dilate(imageAux, dst, kernelDilation);

	vc_image_free(imageAux);

	return ret;
}

// Realizar o fecho binário
int vc_binary_close(IVC* src, IVC* dst, int kernelDilation, int kernelErosion)
{
	int ret = 1;
	IVC* imageAux = vc_image_new(src->width, src->height, src->channels, src->levels);

	ret &= vc_binary_dilate(src, imageAux, kernelDilation);
	ret &= vc_binary_erode(imageAux, dst, kernelErosion);

	vc_image_free(imageAux);

	return ret;
}

// Etiquetagem de blobs
// src		: Imagem binária de entrada (0 ou 255)
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respetivas etiquetas. É necessário libertar posteriormente esta memória.
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
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int lowestLabel, tmplabel; // lowestLabel = var auxiliar para etiqueta
	// tmplabel = var auxiliar para percorrer tabela de etiquetas (para ser mais fácil de ler: usa um nome diferente)
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
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

// Cálculos relacionados com blobs
// NOTA: recebe imagem com os labels (src) a partir da função vc_binary_blob_labelling
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs, int *maiorBlob)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax, blobAreaMax, blobMaior;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;
	if (!blobs) return 0;



	// Inicializações para calcular a blob maior
	blobAreaMax = 0;
	blobMaior = 0;

	// Percorre cada blob
	for (i = 0; i < nblobs; i++)
	{
		blobs[i].area = 0;

		// Percorre cada píxel da imagem etiquetada
		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				// Se o píxel pertencer ao blob de que estamos à procura
				if (data[pos] == blobs[i].label)
				{
					// Área (= somatório do nº de píxeis do blob)
					blobs[i].area++;
				}
			}
		}

		// Compara com o maior até agora
		if (blobs[i].area > blobAreaMax)
		{
			// Este blob é agora o maior
			blobAreaMax = blobs[i].area;
			blobMaior = i;
		}
	}

	*maiorBlob = blobMaior;

	// Inicialização nos extremos possíveis
	xmin = width - 1;
	ymin = height - 1;
	xmax = 0;
	ymax = 0;

	sumx = 0;
	sumy = 0;

	blobs[blobMaior].area = 0;

	// Percorre cada píxel da imagem etiquetada
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Se o píxel pertencer ao blob de que estamos à procura
			if (data[pos] == blobs[blobMaior].label)
			{
				// Área (= somatório do nº de píxeis do blob)
				blobs[blobMaior].area++;

				// Centro de Gravidade
				sumx += x;
				sumy += y;

				// Bounding Box
				if (xmin > x) xmin = x;
				if (ymin > y) ymin = y;
				if (xmax < x) xmax = x;
				if (ymax < y) ymax = y;

				// Perímetro (está a usar em "cruz", não conta os da diagonal
				// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
				if ((data[pos - 1] != blobs[blobMaior].label) || (data[pos + 1] != blobs[blobMaior].label) || (data[pos - bytesperline] != blobs[blobMaior].label) || (data[pos + bytesperline] != blobs[blobMaior].label))
				{
					blobs[blobMaior].perimeter++;
				}
			}
		}
	}

	// Bounding Box
	blobs[blobMaior].x = xmin;
	blobs[blobMaior].y = ymin;
	blobs[blobMaior].width = (xmax - xmin) + 1;
	blobs[blobMaior].height = (ymax - ymin) + 1;

	// Centro de Gravidade
	//blobs[i].xc = (xmax - xmin) / 2;
	//blobs[i].yc = (ymax - ymin) / 2; ** FALTA O round() em xc e yc
	// x médio (média de todos os valores/coordenadas em x)
	blobs[blobMaior].xc = sumx / MAX(blobs[blobMaior].area, 1); // Usa-se o MAX para nunca dar 0
	// y médio (média de todos os valores/coordenadas em y)
	blobs[blobMaior].yc = sumy / MAX(blobs[blobMaior].area, 1);
	

	return 1;
}

// Marcar o centro de massa e a caixa delimitadora de cada blob numa nova imagem (definido só para imagens a cores)
int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = dst->width;
	int height = dst->height;
	int channels = dst->channels;
	int bytesperline = src->bytesperline;
	long int pos;
	int x, y, i, blobXmin, blobXmax, blobYmin, blobYmax;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if ((blobs == NULL) || (nblobs <= 0)) return 0;
	if (channels != 3) return 0;

	// Copia dados da imagem original para a nova imagem
	memcpy(datadst, datasrc, bytesperline * height);

	// Percorre cada blob
	for (i = 0; i < nblobs; i++)
	{
		// Marcar o centro de massa
		pos = blobs[i].yc * bytesperline + blobs[i].xc * channels;
		/*
		datadst[pos] = (unsigned char) 255;
		datadst[pos + 1] = (unsigned char) 255;
		datadst[pos + 2] = (unsigned char) 255;
		*/
		datadst[pos] = (unsigned char)255;
		datadst[pos + 1] = (unsigned char)255;
		datadst[pos + 2] = (unsigned char)255;
		datadst[pos - 1] = (unsigned char)255;
		datadst[pos - 2] = (unsigned char)255;
		datadst[pos - 3] = (unsigned char)255;
		datadst[pos + 3] = (unsigned char)255;
		datadst[pos + 4] = (unsigned char)255;
		datadst[pos + 5] = (unsigned char)255;
		datadst[pos - bytesperline] = (unsigned char)255;
		datadst[pos - bytesperline + 1] = (unsigned char)255;
		datadst[pos - bytesperline + 2] = (unsigned char)255;
		datadst[pos + bytesperline] = (unsigned char)255;
		datadst[pos + bytesperline + 1] = (unsigned char)255;
		datadst[pos + bytesperline + 2] = (unsigned char)255;

		// Coordenadas da caixa delimitadora
		blobYmin = blobs[i].y;
		blobYmax = blobYmin + blobs[i].height - 1;
		blobXmin = blobs[i].x;
		blobXmax = blobXmin + blobs[i].width - 1;

		// Marcar a caixa delimitadora
		// Limites verticais
		for (y = blobYmin; y <= blobYmax; y++)
		{
			// Limite esquerdo
			pos = y * bytesperline + blobXmin * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;

			// Limite direito
			pos = y * bytesperline + blobXmax * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;
		}
		// Limites horizontais
		for (x = blobXmin; x <= blobXmax; x++)
		{
			// Limite superior
			pos = blobYmin * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;

			// Limite inferior
			pos = blobYmax * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;
		}
	}

	return 1;
}

// Marcar a caixa delimitadora do blob com maior área numa nova imagem (definido só para imagens a cores)
int vc_marcarMaiorBlob(IVC* src, IVC* dst, OVC* blobs, int nblobs, int maiorBlob)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = dst->width;
	int height = dst->height;
	int channels = dst->channels;
	int bytesperline = src->bytesperline;
	long int pos;
	int x, y, i, blobXmin, blobXmax, blobYmin, blobYmax, blobAreaMax, blobMaior;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if ((blobs == NULL) || (nblobs <= 0)) return 0;
	if (channels != 3) return 0;

	// Copia dados da imagem original para a nova imagem
	memcpy(datadst, datasrc, bytesperline * height);

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

// Identificar a forma do maior blob (sinal)
Sinal vc_identificarSinal(OVC* blobs, int nblobs, int maiorblob, Cor cor)
{
	float perimetroCaixa, perimetroBlob, proporcaoPerimetros;
	int centroXCaixa;
	// Inicialização do sinal
	Sinal sinal = INDEFINIDO;

	// Verificação de erros
	if ((blobs == NULL) || (nblobs <= 0)) return INDEFINIDO;
	if (maiorblob >= nblobs) return INDEFINIDO;
	if (cor == INDEFINIDA) return INDEFINIDO;

	// Divisão do perímetro do maior blob pela área da caixa delimitadora
	perimetroCaixa = blobs[maiorblob].width + blobs[maiorblob].height;
	perimetroBlob = blobs[maiorblob].perimeter;
	proporcaoPerimetros = perimetroBlob / perimetroCaixa;

	// Se for um sinal azul
	if (cor == AZUL)
	{
		// (Valores aproximados +/- 0.4)

		// Obrigatório virar à direita/esquerda: proporção de perímetros = 2.5
		if (proporcaoPerimetros >= 2.1f && proporcaoPerimetros < 2.9f)
		{
			// X central da caixa delimitadora
			centroXCaixa = blobs[maiorblob].x + blobs[maiorblob].width / 2;

			// Se o centro de massa está à esquerda do centro da caixa delimitadora... 
			if (blobs[maiorblob].xc < centroXCaixa) sinal = VIRAR_E;
			else sinal = VIRAR_D;
		}

		// Automóveis e motociclos: proporção de perímetros = 3.3
		else if (proporcaoPerimetros >= 2.9f && proporcaoPerimetros <= 3.7f) sinal = AUTOMOVEIS_MOTOCICLOS;

		// Auto-estrada: proporção de perímetros = 4.5
		else if (proporcaoPerimetros >= 4.1f && proporcaoPerimetros <= 4.9f) sinal = AUTO_ESTRADA;
	}

	else if (cor == VERMELHO)
	{
		// Sentido proibido: proporção de perímetros = 2.4
		if (proporcaoPerimetros >= 2.0f && proporcaoPerimetros < 2.8f) sinal = SENTIDO_PROIBIDO;

		// STOP: proporção de perímetros = 3.2
		else if (proporcaoPerimetros >= 2.8f && proporcaoPerimetros < 3.6f) sinal = STOP;
	}

	// Identificar a forma de acordo com o cálculo anterior
	//printf("Razao Perimetros = %f\n\n", proporcaoPerimetros);

	return sinal;
}

// Exibir o histograma de uma imagem em tons de cinzento
int vc_gray_histogram_show(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int n = width * height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, xColumn;
	long int pos;
	int i;
	int ni[256] = { 0 };
	float pdf[256] = { 0 };
	float pdfMax = 0;
	int widthRange, heightRange;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			// Somar o número de ocorrências de cada nível de cinzento
			ni[(int)datasrc[pos]]++;
			datadst[pos] = (unsigned char)0;
		}
	}

	// Calcular pdf para cada nível de brilho
	for (i = 0; i < 256; i++)
	{
		pdf[i] = (float)ni[i] / (float)n;
		// Guardar o máximo atual
		if (pdf[i] > pdfMax) pdfMax = pdf[i];
	}

	// Desenhar histograma
	for (i = 0; i < 256; i++)
	{
		// Converter i para x
		xColumn = (i * width) / 255;

		// Calcular largura da coluna
		widthRange = ((i + 1) * width) / 255;

		// Calcular altura da coluna
		heightRange = pdf[i] * (height / pdfMax);

		// Colorir píxeis das colunas a branco
		for (y = 0; y <= heightRange; y++)
		{
			for (x = xColumn; x <= widthRange; x++)
			{
				pos = (height - 2 - y) * bytesperline + x * channels;

				datadst[pos] = (unsigned char)255;
			}
		}
	}

	return 1;
}

// [Código pouco eficiente]
// Equalização de histograma de uma imagem em tons de cinzento
int vc_gray_histogram_equalization(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int n = width * height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int i;
	int ni[256] = { 0 }; // 1º valor é o que pomos aí, próximos são 0
	float pdf[256] = { 0.0f };
	float cdf[256], cdfMin;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	// Somar o número de ocorrências de cada nível de cinzento
	for (pos = 0; pos < n; pos++) ni[(int)datasrc[pos]]++;

	// Calcular pdf para cada nível de brilho
	for (i = 0; i < 256; i++) pdf[i] = (float)ni[i] / (float)n;

	// Calcular cdf
	cdf[0] = pdf[0];
	cdfMin = cdf[0];
	for (i = 1; i < 256; i++)
	{
		cdf[i] = cdf[i - 1] + pdf[i];
	}

	// Calcular cdf mínimo
	for (i = 0, cdfMin = cdf[0]; i < 256; i++)
	{
		if (cdf[i] > 0.0f)
		{
			cdfMin = cdf[i];
			break;
		}
	}

	// Percorrer píxeis da imagem de saída
	for (pos = 0; pos < n; pos++)
	{
		int nivel_de_brilho = datasrc[pos];
		datadst[pos] = (unsigned char)(((cdf[nivel_de_brilho] - cdfMin) / (1.0f - cdfMin)) * 255.0f);
	}

	return 1;
}

// Desenhar contornos da imagem em tons de cinzento, utilizando os operadores de Prewitt
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float gradienteX, gradienteY, A, B, C, D, E, F, G, H, I, magn;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++) // não devia ser height - 1?
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// Calcular gradiente em x
			// Mx = 
			// -1 0 1
			// -1 0 1
			// -1 0 1
			gradienteX = -A - D - G + C + F + I;
			// Calcular gradiente em y
			// My = 
			// -1 -1 -1
			//  0  0  0
			//  1  1  1
			gradienteY = -A - B - C + G + H + I;

			// Calcular a magnitude do vetor
			magn = sqrtf(powf(gradienteX, 2.0f) + powf(gradienteY, 2.0f));

			if (magn > th) datadst[pos] = (unsigned char)255;
			else (unsigned char)0;
		}
	}

	return 1;
}

// Desenhar contornos da imagem em tons de cinzento, utilizando os operadores de Sobel
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float gradienteX, gradienteY, A, B, C, D, E, F, G, H, I, magn;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// Calcular gradiente em x
			// Mx = 
			// -1 0 1
			// -2 0 2
			// -1 0 1
			gradienteX = -A - 2 * D - G + C + 2 * F + I;
			// Calcular gradiente em y
			// My = 
			// -1 -2 -1
			//  0  0  0
			//  1  2  1
			gradienteY = -A - 2 * B - C + G + 2 * H + I;

			// Calcular a magnitude do vetor
			magn = sqrtf(powf(gradienteX, 2.0f) + powf(gradienteY, 2.0f));

			if (magn > th) datadst[pos] = (unsigned char)255;
			else (unsigned char)0;
		}
	}

	return 1;
}

// Desenhar contornos da imagem em tons de cinzento, utilizando os operadores de Laplace (2º derivada)
int OLD_vc_gray_edge_laplace(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float gradiente, A, B, C, D, E, F, G, H, I;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// Calcular gradiente (valor final do píxel)
			/*
			* 2º Derivada
			* Laplacian Operator
			* -1 -1 -1
			* -1  8 -1
			* -1 -1 -1
			*/
			gradiente = -A - B - C - D + 8 * E - F - G - H - I;

			datadst[pos] = (unsigned char)gradiente;
		}
	}

	return 1;
}

// Desenhar contornos da imagem em tons de cinzento, utilizando os operadores de Laplace (2º derivada)
int vc_gray_edge_laplace(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float gradiente, A, B, C, D, E, F, G, H, I;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 3) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// Calcular gradiente (valor final do píxel)
			/*
			* 2º Derivada
			* Laplacian Operator
			* -1 -1 -1
			* -1  8 -1
			* -1 -1 -1
			*/
			gradiente = -A - B - C - D + 8 * E - F - G - H - I;

			datadst[pos] = (unsigned char)gradiente;
			datadst[pos + 1] = (unsigned char)gradiente;
			datadst[pos + 2] = (unsigned char)gradiente;
		}
	}

	return 1;
}

// Filtro de média (passa-baixo) [só está feito para kernels ímpares]
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky, numeroVizinhos;
	int offset = (kernelsize - 1) / 2;
	long int pos, posk;
	float soma = 0, media;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;
	if ((kernelsize <= 1) || (kernelsize % 2 == 0)) return 0; // Kernel tem que ser > 1 e ímpar

	// Percorrer píxeis da imagem original
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x;

			soma = 0;
			numeroVizinhos = 0;
			// Percorrer Vizinhos (kernel)
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						numeroVizinhos++;
						posk = (y + ky) * bytesperline + (x + kx);

						// Somar valores dos vizinhos
						soma += (float)data[posk];
					}
				}
			}

			// Fazer a média dos valores do kernel
			media = soma / (float)numeroVizinhos;

			// Atribuir a média ao píxel central
			datadst[pos] = (unsigned char)roundf(media);
		}
	}

	return 1;
}

// Ordenar um array com o algoritmo de insertion sort
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

// Filtro de mediana (passa-baixo) [só está feito para kernels ímpares]
int OLD_vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernelsize)
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
	if (channels != 3) return 0;
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
			datadst[pos + 1] = (unsigned char)vizinhos[centro];
			datadst[pos + 2] = (unsigned char)vizinhos[centro];

			vizinhosCount = 0;
		}
	}

	free(vizinhos);

	return 1;
}

// Filtro de mediana (passa-baixo) [só está feito para kernels ímpares]
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

// Filtro Gaussiano (passa-baixo)
int vc_gray_lowpass_gaussian_filter(IVC* src, IVC* dst)
{
	int kernelsize = 5;
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky, indiceMascara = 0;
	int offset = (kernelsize - 1) / 2;
	long int pos, posk;
	float soma = 0, media;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Atribuir valores da máscara
	int mascara[25] = { 1,4,7,4,1,
					   4,16,26,16,4,
					   7,26,41,26,7,
					   4,16,26,16,4,
					   1,4,7,4,1 };

	// Percorrer píxeis da imagem original
	for (y = 2; y < height; y++)
	{
		for (x = 2; x < width; x++)
		{
			pos = y * bytesperline + x;

			soma = 0;
			indiceMascara = 0;
			// Percorrer Vizinhos (kernel)
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width))
					{
						posk = (y + ky) * bytesperline + (x + kx);

						// Somar valores dos vizinhos com máscara
						soma += (float)data[posk] * (float)mascara[indiceMascara];
						indiceMascara++;
					}
				}
			}

			// Fazer a "média" dos valores do kernel
			media = soma / 273.0f;

			// Atribuir a "média" ao píxel central
			datadst[pos] = (unsigned char)roundf(media);
		}
	}

	return 1;
}

// Máscaras de Filtros Passa-alto Básicos (filtros Laplacianos)
/* (1/6)
*  0 -1  0
* -1  4 -1
*  0 -1  0
*/
/* (1/9)
* -1 -1 -1
* -1  8 -1
* -1 -1 -1
*/
/* (1/16)
* -1 -2 -1
* -2 12 -2
* -1 -2 -1
*/

// Filtro básico (passa-alto)
int vc_gray_highpass_filter(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float soma, media, A, B, C, D, E, F, G, H, I;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// Máscara (1/9)
			/*
			* -1 -1 -1
			* -1  8 -1
			* -1 -1 -1
			*/

			soma = -A - B - C
				- D + 8 * E - F
				- G - H - I;
			media = fabs(soma) / 9;

			datadst[pos] = (unsigned char)roundf(media);
		}
	}

	return 1;
}

// Aplicação do filtro básico (passa-alto)
int vc_gray_highpass_filter_enhance(IVC* src, IVC* dst, int gain)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	float soma, media, A, B, C, D, E, F, G, H, I, valorFinal;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorrer píxeis da imagem original
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			pos = y * bytesperline + x * channels;

			// Obter valores nas posições da máscara
			// A | B | C 
			// D | E | F
			// G | H | I
			A = (float)data[(y - 1) * bytesperline + (x - 1) * channels];
			B = (float)data[(y - 1) * bytesperline + x * channels];
			C = (float)data[(y - 1) * bytesperline + (x + 1) * channels];
			D = (float)data[y * bytesperline + (x - 1) * channels];
			E = (float)data[y * bytesperline + x * channels];
			F = (float)data[y * bytesperline + (x + 1) * channels];
			G = (float)data[(y + 1) * bytesperline + (x - 1) * channels];
			H = (float)data[(y + 1) * bytesperline + x * channels];
			I = (float)data[(y + 1) * bytesperline + (x + 1) * channels];

			// // Máscara (1/16)
			/*
			* -1 -2 -1
			* -2 12 -2
			* -1 -2 -1
			*/

			soma = -A - 2 * B - C
				- 2 * D + 12 * E - 2 * F
				- G - 2 * H - I;
			media = soma / 16;

			// Aplicar o resultado do filtro
			valorFinal = E + media * (float)gain;
			// Tendo em conta os limites de valores que um píxel pode tomar, isto é, o intervalo [0, 255]
			if (valorFinal < 0) valorFinal = 0.0f;
			else if (valorFinal > 255) valorFinal = 255.0f;

			datadst[pos] = (unsigned char)roundf(valorFinal);
		}
	}

	return 1;
}