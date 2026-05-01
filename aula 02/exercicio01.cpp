#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// Essa função realiza a convolução de uma imagem com um kernel,
// utilizando padding para lidar com as bordas da imagem
Mat convolucaoPadding(const Mat &imagem, const Mat &kernel)
{
	int image_rows = imagem.rows;
	int image_cols = imagem.cols;

	int kernel_rows = kernel.rows;
	int kernel_cols = kernel.cols;

	int pad_rows = kernel_rows / 2;
	int pad_cols = kernel_cols / 2;

	Mat resultado = Mat::zeros(image_rows, image_cols, CV_8UC1);

	for (int i = 0; i < image_rows; i++)
	{
		for (int j = 0; j < image_cols; j++)
		{
			float sum = 0.0;
			// Percorrer o kernel
			for (int k = 0; k < kernel_rows; k++)
			{
				for (int l = 0; l < kernel_cols; l++)
				{
					int x = i + k - pad_rows;
					int y = j + l - pad_cols;

					// Verificar se os índices estão dentro dos limites da imagem
					if (x >= 0 && x < image_rows && y >= 0 && y < image_cols)
					{
						sum += imagem.at<uchar>(x, y) * kernel.at<float>(k, l);
					}
				}
			}
			resultado.at<uchar>(i, j) = static_cast<uchar>(sum);
		}
	}
	return resultado;
}

// Essa função realiza a convolução de uma imagem com um kernel,
// sem utilizar padding.
// Isso significa que os pixels nas bordas da imagem não serão processados,
// resultando em uma imagem de saída menor do que a imagem original.
Mat convolucaoSemBordas(const Mat &imagem, const Mat &kernel)
{
	int image_rows = imagem.rows;
	int image_cols = imagem.cols;

	int kernel_rows = kernel.rows;
	int kernel_cols = kernel.cols;

	int pad_rows = kernel_rows / 2;
	int pad_cols = kernel_cols / 2;

	Mat resultado = Mat::zeros(image_rows, image_cols, CV_8UC1);

	for (int i = pad_rows; i < image_rows - pad_rows; i++)
	{
		for (int j = pad_cols; j < image_cols - pad_cols; j++)
		{
			float sum = 0.0;
			// Percorrer o kernel
			for (int k = 0; k < kernel_rows; k++)
			{
				for (int l = 0; l < kernel_cols; l++)
				{
					int x = i + k - pad_rows;
					int y = j + l - pad_cols;
					sum += imagem.at<uchar>(x, y) * kernel.at<float>(k, l);
				}
			}
			resultado.at<uchar>(i, j) = static_cast<uchar>(sum);
		}
	}
	return resultado;
}

int main()
{
	Mat imagem = imread("../images/imagem.jpg", IMREAD_GRAYSCALE);
	if (imagem.empty())
	{
		cout << "Erro ao carregar a imagem!" << endl;
		return -1;
	}

	Mat kernel = (Mat_<float>(3, 3) << 1.0 / 9, 1.0 / 9, 1.0 / 9,
								1.0 / 9, 1.0 / 9, 1.0 / 9,
								1.0 / 9, 1.0 / 9, 1.0 / 9);

	Mat resultado = convolucaoPadding(imagem, kernel);
	Mat resultadoSemBordas = convolucaoSemBordas(imagem, kernel);

	imshow("Imagem Original", imagem);
	imshow("Imagem Filtrada", resultado);
	imshow("Imagem Filtrada Sem Bordas", resultadoSemBordas);
	waitKey(0);

	return 0;
}