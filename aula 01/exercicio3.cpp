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

  int x, y;
  cout << "Digite a coordenada x: ";
  cin >> x;
  cout << "Digite a coordenada y: ";
  cin >> y;

  if (x >= 0 && x < imagem.cols && y >= 0 && y < imagem.rows)
  {
    int intensidade = imagem.at<uchar>(y, x);
    cout << "Intensidade do pixel (" << x << ", " << y << ") = "
         << intensidade << endl;
  }
  else
  {
    cout << "Coordenadas fora da imagem!" << endl;
  }
  return 0;
}