#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
  Mat imagem = imread("imagem.jpg", IMREAD_GRAYSCALE);

  if (imagem.empty())
  {
    cout << "Erro ao carregar a imagem!" << endl;
    return -1;
  }
  const int limiar = 128;

  Mat binariaManual(imagem.rows, imagem.cols, CV_8UC1);
  for (int y = 0; y < imagem.rows; y++)
  {
    for (int x = 0; x < imagem.cols; x++)
    {
      uchar pixel = imagem.at<uchar>(y, x);
      binariaManual.at<uchar>(y, x) = (pixel >= limiar) ? 255 : 0;
    }
  }

  imshow("Imagem Binaria Manual", binariaManual);

  waitKey(0);
  return 0;
}