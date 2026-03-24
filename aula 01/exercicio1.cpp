#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{
  Mat imagem = imread("imagem.jpg");
  if (imagem.empty())
  {
    cout << "Erro ao carregar a imagem!" << endl;
    return -1;
  }

  int largura = imagem.cols;
  int altura = imagem.rows;
  int canais = imagem.channels();

  cout << "Largura: " << largura << " pixels" << endl;
  cout << "Altura: " << altura << " pixels" << endl;
  cout << "Canais: " << canais << endl;

  imshow("Imagem Original", imagem);
  waitKey(0);
  return 0;
}