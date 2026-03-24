#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main()
{
  Mat imagem = imread("imagem.jpg");
  Mat imagemCinza;
  if (imagem.empty())
  {
    cout << "Erro ao carregar a imagem!" << endl;
    return -1;
  }

  cvtColor(imagem, imagemCinza, COLOR_BGR2GRAY);

  imshow("Imagem Original", imagem);
  imshow("Imagem Cinza", imagemCinza);
  waitKey(0);
}
