#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

static Mat quantizarCinza(const Mat &imagemCinza, int niveis)
{
  Mat saida(imagemCinza.rows, imagemCinza.cols, CV_8UC1);

  const int passo = 256 / niveis;
  for (int y = 0; y < imagemCinza.rows; y++)
  {
    for (int x = 0; x < imagemCinza.cols; x++)
    {
      int pixel = imagemCinza.at<uchar>(y, x);
      int nivel = pixel / passo;
      if (nivel >= niveis)
      {
        nivel = niveis - 1;
      }

      // Mapeia para o centro da faixa para preservar melhor o contraste visual.
      int valorQuantizado = nivel * passo + passo / 2;
      if (valorQuantizado > 255)
      {
        valorQuantizado = 255;
      }

      saida.at<uchar>(y, x) = valorQuantizado;
    }
  }

  return saida;
}

int main()
{
  Mat imagemCinza = imread("imagem.jpg", IMREAD_GRAYSCALE);
  if (imagemCinza.empty())
  {
    cout << "Erro ao carregar a imagem em tons de cinza!" << endl;
    return -1;
  }

  vector<int> niveis = {128, 64, 16, 4};
  imshow("Original (256 niveis)", imagemCinza);

  for (int nivel : niveis)
  {
    Mat quantizada = quantizarCinza(imagemCinza, nivel);

    string nomeJanela = "Quantizada - " + to_string(nivel) + " niveis";
    string nomeArquivo = "imagem_quantizada_" + to_string(nivel) + "niveis.png";

    imshow(nomeJanela, quantizada);
  }

  waitKey(0);
  return 0;
}
