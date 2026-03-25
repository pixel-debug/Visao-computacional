#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
  Mat imagem = imread("imagem.jpg");

  if (imagem.empty())
  {
    cout << "Erro ao carregar a imagem!" << endl;
    return -1;
  }

  Mat imagemReduzida;
  resize(imagem, imagemReduzida, Size(imagem.cols / 2, imagem.rows / 2));

  Mat imagemReduzidaDeVolta;
  resize(imagemReduzida, imagemReduzidaDeVolta, Size(imagem.cols, imagem.rows));

  imshow("Imagem Original", imagem);
  imshow("Imagem Reduzida", imagemReduzida);
  imshow("Imagem Reduzida de Volta", imagemReduzidaDeVolta);

  waitKey(0);
  return 0;
}