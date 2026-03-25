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
  Mat negativo = 255 - imagem;

  imshow("Imagem Original", imagem);
  imshow("Imagem Negativa", negativo);

  waitKey(0);
  return 0;
}