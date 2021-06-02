///*
//Author: Filipe Gajo
//Author: Ricardo Sampaio
//Author: Claudio Silva
//*/
//
//// Headers "normais"
//#include <iostream> // Header para input e output streams
//#include <string> // Classe string do C++
//
//// M�dulos do openCV
//#include <opencv2\opencv.hpp> // Fun��es principais do OpenCV (Open Source Computer Vision Library)
//#include <opencv2\core.hpp> // Estruturas de dados, opera��es sobre arrays, XML e desenho
//#include <opencv2\highgui.hpp> // Fun��es de interface com o utilizador
//#include <opencv2\videoio.hpp> // Fun��es de leitura/escrita de v�deo
//
//// Avisar que est�o a ser usadas conven��es da linguagem de programa��o C para a linha #include "vc.h"
//extern "C" {
//#include "vc.h"
//}
//
//int main(void)
//{
//	IVC* imagemCamera, * imagemHSV, * imagemHSVsegmentada, * imagemHSVsemRuido, * imagemHSVcontornos, *imagemEtiquetada, *imagemMarcada, *imagemEqualizada;
//	int nlabels;
//	// Classe cv::VideoCapture: classe para captura de v�deo a partir de c�maras ou para leitura de ficheiros de v�deo e sequ�ncias de imagens
//	cv::VideoCapture capture;
//
//	// Estrutura do v�deo
//	struct
//	{
//		int width, height;
//		int ntotalframes; // N� total de frames de um v�deo
//		int fps; // Frames por segundo
//		int nframe; // N� da frame atual
//	} video;
//
//	std::string str;
//	int key = 0, nCanais = 3;
//
//	// usar c�mara do pc em vez (s� com 0 se der erro, sem ',' e frente)
//	// c�mara 0. Se existisse outra c�mara ligada por usb, seria a c�mara 1
//	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
//	capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);
//
//	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
//	if (!capture.isOpened()) // se tentarmos abrir um ficheiro que n�o temos
//	{
//		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
//		return 1;
//	}
//
//	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
//	/* Frame rate do v�deo */
//	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
//	/* Resolu��o do v�deo */
//	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
//	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
//
//	/* Cria uma janela para exibir o v�deo */
//	// cvv:WINDOW_AUTOSIZE: ajusta o tamanho da janela automaticamente para corresponder ao tamanho da imagem. N�o permite alterar o tamanho da janela manualmente.
//	cv::namedWindow("VC - Video", cv::WINDOW_AUTOSIZE);
//
//	// Cria��o das imagens IVC
//	imagemCamera = vc_image_new(video.width, video.height, nCanais, 255);
//	imagemHSV = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVsegmentada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVsemRuido = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVcontornos = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemEtiquetada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemMarcada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemEqualizada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//
//
//
//
//	// Cria��o de imagens OVC
//	OVC* blobs;
//
//	cv::Mat frame;
//
//	// Fecha a captura/leitura de v�deo ao carregar na tecla q
//	while (key != 'q')
//	{
//		/* Leitura de uma frame do v�deo */
//		capture.read(frame);
//
//		/* Verifica se conseguiu ler a frame */
//		// Quando chegar ao fim duma leitura de um ficheiro de v�deo � aqui que para o ciclo
//		if (frame.empty()) break;
//
//		/* N�mero da frame a processar */
//		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);
//
//		
//		
//		
//		// !CUIDADO! cv::Mat est� em BGR ao contr�rio de IVC que est� em RGB !CUIDADO!
//
//		
//
//		//// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
//		memcpy(imagemCamera->data, frame.data, video.width * nCanais * video.height);
//
//		// Segmentar pela cor azul
//		//vc_bgr_get_blue_gray(image);
//
//		// Converter BGR para HSV
//		vc_bgr_to_hsv(imagemCamera, imagemHSV);
//		
//
//		// Segmentar imagem HSV por cores azuis
//		vc_hsv_segmentation(imagemHSV, imagemHSVsegmentada, 192, 289, 20, 100, 15, 100);
//
//
//		// Eliminar ru�do "salt-and-pepper"
//		vc_gray_lowpass_median_filter(imagemHSVsegmentada, imagemHSVsemRuido, 3);
//
//		// Exibir contornos
//		vc_gray_edge_laplace(imagemHSVsemRuido, imagemHSVcontornos);
//
//
//		//Marca��o de blobs
//		blobs = vc_binary_blob_labelling(imagemHSVsegmentada, imagemEtiquetada, &nlabels);
//
//		vc_binary_blob_info(imagemEtiquetada, blobs, nlabels);
//
//		printf("Labels: %d\n", nlabels);
//
//		//vc_mark_blobs(imagemCamera, imagemMarcada, blobs, nlabels);
//		//---------------------
//
//
//		//vc_transit_signal_identifier(imagemHSVcontornos);
//
//		memcpy(frame.data, imagemHSVcontornos->data, video.width* nCanais* video.height);
//
//
//		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
//		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
//		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
//		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
//		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
//		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
//		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
//		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
//		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
//		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
//		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
//		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
//
//
//		//// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
//
//		/* Exibe a frame */
//		cv::imshow("VC - Video", frame);
//
//		// Espera um milissegundo por uma tecla pressionada pelo utilizador. 
//		// Grava a tecla pressionada em key.
//		key = cv::waitKey(1);
//	}
//
//	//// Liberta a mem�ria das imagens IVC
//	vc_image_free(imagemCamera);
//	vc_image_free(imagemHSV);
//	vc_image_free(imagemHSVsegmentada);
//	vc_image_free(imagemHSVsemRuido);
//	vc_image_free(imagemHSVcontornos);
//
//	/* Fecha a janela */
//	cv::destroyWindow("VC - Video");
//
//	/* Fecha o ficheiro de v�deo */
//	capture.release();
//
//	return 0;
//}



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
	IVC* imagemCamera, * imagemHSV, * imagemHSVsegmentada, * imagemHSVsemRuido, * imagemHSVcontornos, *imagemEtiquetada, *imagemMarcada;
	OVC* blobs;
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
	int nlabels;

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
	imagemHSVsegmentada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemHSVsemRuido = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemHSVcontornos = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemEtiquetada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemMarcada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);


	int counter = 0;
	cv::Mat frame;

	// Fecha a captura/leitura de v�deo ao carregar na tecla q
	while (key != 'q')
	{
		counter++;
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

		// Segmentar imagem HSV por cores azuis
		vc_hsv_segmentation(imagemHSV, imagemHSVsegmentada, 192, 289, 10, 100, 15, 100);

		// Eliminar ru�do "salt-and-pepper"
		vc_gray_lowpass_median_filter(imagemHSVsegmentada, imagemHSVsemRuido, 5);
		
		blobs = vc_binary_blob_labelling(imagemHSVsemRuido, imagemEtiquetada, &nlabels);

		if (blobs)
		{
			puts("Tem blobs");
			vc_binary_blob_info(imagemEtiquetada, blobs, nlabels);
			for (int i = 0; i < nlabels; i++)
			{
				printf("-> Etiqueta %d\n", blobs[i].label);
				printf("-> Area %d\n", blobs[i].area);
				printf("-> XC %d\n", blobs[i].xc);
				printf("-> XY %d\n\n", blobs[i].yc);

			}

		}


		vc_mark_blobs(imagemCamera, imagemMarcada, blobs, nlabels);

		// Exibir contornos
		vc_gray_edge_laplace(imagemHSVsemRuido, imagemHSVcontornos);


		//// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, imagemHSVsemRuido->data, video.width * nCanais * video.height);

		free(blobs);


		/* Exibe a frame */
		cv::imshow("VC - Video", frame);

		// Espera um milissegundo por uma tecla pressionada pelo utilizador. 
		// Grava a tecla pressionada em key.
		key = cv::waitKey(1);
	}

	//// Liberta a mem�ria das imagens IVC
	vc_image_free(imagemCamera);
	vc_image_free(imagemHSV);
	vc_image_free(imagemHSVsegmentada);
	vc_image_free(imagemHSVsemRuido);
	vc_image_free(imagemHSVcontornos);

	/* Fecha a janela */
	cv::destroyWindow("VC - Video");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}