/*
Autores:
-Filipe Gajo
-Ricardo Sampaio
-Cláudio Silva
*/

// Headers "normais"
#include <iostream> // Header para input e output streams
#include <string> // Classe string do C++

// Módulos do openCV
#include <opencv2\opencv.hpp>  // Funções principais do OpenCV (Open Source Computer Vision Library)
#include <opencv2\core.hpp>	   // Estruturas de dados, operações sobre arrays, XML e desenho
#include <opencv2\highgui.hpp> // Funções de interface com o utilizador
#include <opencv2\videoio.hpp> // Funções de leitura/escrita de vídeo

// Avisar que estão a ser usadas convenções da linguagem de programação C para a linha #include "vc.h"
extern "C" {
#include "vc.h"
}

int main(void)
{
	IVC* imagemCamera, * imagemHSV, * imagemSegmentada, * imagemSemRuido, * imagemLabels, * imagemBoundingBox;
	OVC* blobs;
	int nblobs, maiorBlob;
	Sinal sinal = INDEFINIDO;
	Cor cor = AZUL;

	// Classe cv::VideoCapture: classe para captura de vídeo a partir de câmaras ou para leitura de ficheiros de vídeo e sequências de imagens
	cv::VideoCapture capture;

	// Estrutura do vídeo
	struct
	{
		int width, height;
		int ntotalframes; // Nº total de frames de um vídeo
		int fps; // Frames por segundo
		int nframe; // Nº da frame atual
	} video;

	std::string informacaoSinal = std::string("");
	int key = 0, nCanais = 3;

	// usar câmara do pc em vez (só com 0 se der erro, sem ',' e frente)
	// câmara 0. Se existisse outra câmara ligada por usb, seria a câmara 1
	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened()) // se tentarmos abrir um ficheiro que não temos
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}

	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	// cvv:WINDOW_AUTOSIZE: ajusta o tamanho da janela automaticamente para corresponder ao tamanho da imagem. Não permite alterar o tamanho da janela manualmente.
	cv::namedWindow("VC - Video", cv::WINDOW_AUTOSIZE);

	// Criação das imagens IVC
	imagemCamera = vc_image_new(video.width, video.height, nCanais, 255);
	imagemHSV = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
	imagemSegmentada = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemSemRuido = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemLabels = vc_image_new(imagemCamera->width, imagemCamera->height, 1, imagemCamera->levels);
	imagemBoundingBox = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);

	cv::Mat frame;

	// Fecha a captura/leitura de vídeo ao carregar na tecla q
	while (key != 'q')
	{
		/* Leitura de uma frame do vídeo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		// Quando chegar ao fim duma leitura de um ficheiro de vídeo é aqui que para o ciclo
		if (frame.empty()) break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		// !CUIDADO! cv::Mat está em BGR ao contrário de IVC que está em RGB !CUIDADO!

		//// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(imagemCamera->data, frame.data, video.width * nCanais * video.height);

		// Converter BGR para HSV
		vc_bgr_to_hsv(imagemCamera, imagemHSV);

		// Segmentar imagem HSV
		if (cor == AZUL) vc_hsv_segmentation(imagemHSV, imagemSegmentada, 192, 289, 10, 100, 15, 100);
		else if (cor == VERMELHO) vc_hsv_red_segmentation(imagemHSV, imagemSegmentada, 0, 34, 335, 360, 30, 100, 35, 100);

		// Eliminar ruído "salt-and-pepper"
		vc_gray_lowpass_median_filter(imagemSegmentada, imagemSemRuido, 7);

		// Etiquetar blobs da imagem
		blobs = vc_binary_blob_labelling(imagemSemRuido, imagemLabels, &nblobs);

		// Procurar o maior blob
		vc_encontrarMaiorBlob(imagemLabels, blobs, nblobs, &maiorBlob);

		// Verificar se o maior blob tem tamanho suficiente para ser um sinal de trânsito
		if (blobs != NULL && blobs[maiorBlob].area < 6000)
		{ // Não detetou o sinal (tentar a outra cor para a próxima frame)
			if (cor == AZUL) cor = VERMELHO;
			else if (cor == VERMELHO) cor = AZUL;
		}
		// Detetou o sinal
		else
		{
			// Cálculos de medidas do maior blob
			vc_maiorBlob_info(imagemLabels, blobs, nblobs, maiorBlob);

			// Marcar bounding box e centro de massa do maior blob
			vc_marcarMaiorBlob(imagemCamera, imagemBoundingBox, blobs, nblobs, maiorBlob);

			// Identificar o sinal de trânsito
			sinal = vc_identificarSinal(blobs, nblobs, maiorBlob, cor);

			// Escolher o texto que vai aparecer no ecrã de acordo com o sinal identificado
			switch (sinal)
			{
			case (INDEFINIDO):
				break;
			case (VIRAR_D):
				informacaoSinal = std::string("Obrigatorio Virar a Direita");
				break;
			case (VIRAR_E):
				informacaoSinal = std::string("Obrigatorio Virar a Esquerda");
				break;
			case (AUTOMOVEIS_MOTOCICLOS):
				informacaoSinal = std::string("Via Reservada a Automoveis e Motociclos");
				break;
			case (AUTO_ESTRADA):
				informacaoSinal = std::string("Entrada para Auto-Estrada");
				break;
			case (SENTIDO_PROIBIDO):
				informacaoSinal = std::string("Sentido Proibido");
				break;
			case (STOP):
				informacaoSinal = std::string("Paragem Obrigatoria");
				break;
			default:
				break;
			}

			//// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
			memcpy(frame.data, imagemBoundingBox->data, video.width * nCanais * video.height);

			// ESCREVER NO VÍDEO
			// putText: escreve texto sobre o vídeo
			// cv::putText(imagem, texto, ponto, fonte, tamanhoFonte, vetorCor, espessuraTexto)
			// contorno a preto (espessura = 2)
			cv::putText(frame, informacaoSinal, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 2);
			// texto branco interior (espessura = 1)
			cv::putText(frame, informacaoSinal, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(255, 255, 255), 1);
		}

		free(blobs);

		/* Exibe a frame */
		cv::imshow("VC - Video", frame);

		// Espera um milissegundo por uma tecla pressionada pelo utilizador. 
		// Grava a tecla pressionada em key.
		key = cv::waitKey(1);
	}

	//// Liberta a memória das imagens IVC
	vc_image_free(imagemCamera);
	vc_image_free(imagemHSV);
	vc_image_free(imagemSegmentada);
	vc_image_free(imagemSemRuido);
	vc_image_free(imagemLabels);
	vc_image_free(imagemBoundingBox);

	/* Fecha a janela */
	cv::destroyWindow("VC - Video");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}