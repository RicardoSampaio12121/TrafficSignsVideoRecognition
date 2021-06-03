/*
Author: Filipe Gajo
Author: Ricardo Sampaio
Author: Claudio Silva
*/

// Headers "normais"
#include <iostream> // Header para input e output streams
#include <string> // Classe string do C++

// M�dulos do openCV
#include <opencv2\opencv.hpp> // Fun��es principais do OpenCV (Open Source Computer Vision Library)
#include <opencv2\core.hpp> // Estruturas de dados, opera��es sobre arrays, XML e desenho
#include <opencv2\highgui.hpp> // Fun��es de interface com o utilizador
#include <opencv2\videoio.hpp> // Fun��es de leitura/escrita de v�deo

// Avisar que est�o a ser usadas conven��es da linguagem de programa��o C para a linha #include "vc.h"
extern "C" {
#include "vc.h"
}

int main(void)
{
	IVC* imagemCamera, * imagemHSV, * imagemSegmentada, * imagemSemRuido, * imagemHSVcontornos, * imagemLabels, * imagemBoundingBox;
	OVC* blobs;
	int nblobs, maiorBlob;
	Sinal sinal = INDEFINIDO;
	Cor cor = AZUL;

	// Classe cv::VideoCapture: classe para captura de v�deo a partir de c�maras ou para leitura de ficheiros de v�deo e sequ�ncias de imagens
	cv::VideoCapture capture;

	// Estrutura do v�deo
	struct
	{
		int width, height;
		int ntotalframes; // N� total de frames de um v�deo
		int fps; // Frames por segundo
		int nframe; // N� da frame atual
	} video;

	std::string str;
	int key = 0, nCanais = 3;


	// usar c�mara do pc em vez (s� com 0 se der erro, sem ',' e frente)
	// c�mara 0. Se existisse outra c�mara ligada por usb, seria a c�mara 1
	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened()) // se tentarmos abrir um ficheiro que n�o temos
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	// cvv:WINDOW_AUTOSIZE: ajusta o tamanho da janela automaticamente para corresponder ao tamanho da imagem. N�o permite alterar o tamanho da janela manualmente.
	cv::namedWindow("VC - Video", cv::WINDOW_AUTOSIZE);

	// Cria��o das imagens IVC
	imagemCamera = vc_image_new(video.width, video.height, nCanais, 255);
	imagemHSV = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemSegmentada = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemSemRuido = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemLabels = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemBoundingBox = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	//imagemHSVcontornos = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);

	cv::Mat frame;

	// Fecha a captura/leitura de v�deo ao carregar na tecla q
	while (key != 'q')
	{
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		// Quando chegar ao fim duma leitura de um ficheiro de v�deo � aqui que para o ciclo
		if (frame.empty()) break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		/*
		// Exemplo de inser��o de texto na frame ---------------------------------------------------------------------------------

		// .append("sufixo"): extende a string acrescentando caracteres no fim do seu valor atual
		// std::to_string(n�mero): converte um valor num�rico numa string (std::string)
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));

		// putText: escreve texto sobre o v�deo
		// cv::putText(imagem, texto, ponto, fonte, tamanhoFonte, vetorCor, espessuraTexto)
		// contorno a preto (espessura = 2)
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		// texto branco interior (espessura = 1)
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		// -----------------------------------------------------------------------------------------------------------------------
		*/

		// !CUIDADO! cv::Mat est� em BGR ao contr�rio de IVC que est� em RGB !CUIDADO!

		//// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(imagemCamera->data, frame.data, video.width * nCanais * video.height);

		// Segmentar pela cor azul
		//vc_bgr_get_blue_gray(image);

		// Converter BGR para HSV
		vc_bgr_to_hsv(imagemCamera, imagemHSV);

		// Segmentar imagem HSV
		if (cor == AZUL) vc_hsv_segmentation(imagemHSV, imagemSegmentada, 192, 289, 10, 100, 15, 100);
		else if (cor == VERMELHO) vc_hsv_red_segmentation(imagemHSV, imagemSegmentada, 0, 34, 335, 360, 30, 100, 35, 100);

		// Eliminar ru�do "salt-and-pepper"
		vc_gray_lowpass_median_filter(imagemSegmentada, imagemSemRuido, 7);

		// Etiquetar blobs da imagem
		blobs = vc_binary_blob_labelling(imagemSemRuido, imagemLabels, &nblobs);
		//printf("\nNumero de blobs: %d\n\n", nblobs);

		// Calcular bounding box
		vc_binary_blob_info(imagemLabels, blobs, nblobs, &maiorBlob);

		// Marcar bounding box do blob com maior �rea (sinal)
		vc_marcarMaiorBlob(imagemCamera, imagemBoundingBox, blobs, nblobs, maiorBlob);

		

		// N�o detetou o sinal (tentar a outra cor para a pr�xima frame)
		if (blobs != NULL && blobs[maiorBlob].area < 5000)
		{
			if (cor == AZUL) cor = VERMELHO;
			else if (cor == VERMELHO) cor = AZUL;
		}
		// Detetou o sinal
		else
		{
			// Identificar a forma do sinal de tr�nsito

			sinal = vc_identificarSinal(blobs, nblobs, maiorBlob, cor);

			//printf("AREA DO MAIOR BLOB = %d\n\n", blobs[maiorBlob].area);

			// printf("SINAL = %d\n", sinal);
			// PARA TESTAR
			switch (sinal)
			{
			case (INDEFINIDO):
				printf("INDEFINIDO\n\n");
				break;
			case (VIRAR_D):
				printf("VIRAR A DIREITA\n\n");
				break;
			case (VIRAR_E):
				printf("VIRAR A ESQUERDA\n\n");
				break;
			case (AUTOMOVEIS_MOTOCICLOS):
				printf("Via reservada a automoveis e motociclos\n\n");
				break;
			case (AUTO_ESTRADA):
				printf("ENTRADA PARA AUTO-ESTRADA\n\n");
				break;
			case (SENTIDO_PROIBIDO):
				printf("SENTIDO PROIBIDO!\n\n");
				break;
			case (STOP):
				printf("STOP!\n\n");
				break;
			default:
				printf("!!!ERRO!!!\n\n");
				break;
			}
		}

		free(blobs);

		// Exibir contornos
		//vc_gray_edge_laplace(imagemSemRuido, imagemHSVcontornos);

		//// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		//memcpy(frame.data, imagemHSVcontornos->data, video.width * nCanais * video.height);
		memcpy(frame.data, imagemBoundingBox->data, video.width * nCanais * video.height);

		/* Exibe a frame */
		cv::imshow("VC - Video", frame);

		// Espera um milissegundo por uma tecla pressionada pelo utilizador. 
		// Grava a tecla pressionada em key.
		key = cv::waitKey(1);
	}

	//// Liberta a mem�ria das imagens IVC
	vc_image_free(imagemCamera);
	vc_image_free(imagemHSV);
	vc_image_free(imagemSegmentada);
	vc_image_free(imagemSemRuido);
	vc_image_free(imagemLabels);
	//vc_image_free(imagemHSVcontornos);

	/* Fecha a janela */
	cv::destroyWindow("VC - Video");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}