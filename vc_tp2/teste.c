//#include "vc.h"
//
//int main() 
//{
//
//	IVC* imagemCamera, * imagemHSV, * imagemHSVsegmentada, * imagemHSVsemRuido, * imagemHSVcontornos, * imagemEtiquetada, * imagemMarcada, * imagemEqualizada;
//	
//	imagemCamera = vc_read_image("ArrowLeft.ppm");
//	imagemHSV = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVsegmentada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVsemRuido = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemHSVcontornos = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemEtiquetada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemMarcada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//	imagemEqualizada = vc_image_new(imagemCamera->width, imagemCamera->height, imagemCamera->channels, imagemCamera->levels);
//
//	vc_bgr_to_hsv(imagemCamera, imagemHSV);
//
//	// Segmentar imagem HSV por cores azuis
//	vc_hsv_segmentation(imagemHSV, imagemHSVsegmentada, 192, 289, 20, 100, 15, 100);
//
//	//vc_gray_histogram_equalization(imagemHSVsegmentada, imagemEqualizada);
//
//	// Eliminar ruído "salt-and-pepper"
//	vc_gray_lowpass_median_filter(imagemHSVsegmentada, imagemHSVsemRuido, 3);
//
//	// Exibir contornos
//	vc_gray_edge_laplace(imagemHSVsemRuido, imagemHSVcontornos);
//
//	 
//
//	vc_write_image("hsvContornos.ppm", imagemHSVsegmentada);
//
//	return 0;
//}