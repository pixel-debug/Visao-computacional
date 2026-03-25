#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

static Mat reduzirResolucaoEspacial(const Mat &imgCinza, int fator)
{
  Mat pequena;
  Mat ampliada;

  resize(imgCinza, pequena, Size(imgCinza.cols / fator, imgCinza.rows / fator), 0, 0, INTER_AREA);
  resize(pequena, ampliada, imgCinza.size(), 0, 0, INTER_NEAREST);

  return ampliada;
}

static Mat reduzirResolucaoIntensidade(const Mat &imgCinza, int niveis)
{
  Mat saida(imgCinza.rows, imgCinza.cols, CV_8UC1);
  int passo = 256 / niveis;

  for (int y = 0; y < imgCinza.rows; y++)
  {
    for (int x = 0; x < imgCinza.cols; x++)
    {
      int pixel = imgCinza.at<uchar>(y, x);
      int nivel = pixel / passo;
      if (nivel >= niveis)
      {
        nivel = niveis - 1;
      }

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
  Mat imagem = imread("imagem.jpg", IMREAD_GRAYSCALE);

  if (imagem.empty())
  {
    cout << "Erro ao carregar a imagem!" << endl;
    return -1;
  }

  const int fatorEspacial = 4;
  const int niveisCinza = 8;

  Mat imagemEspacial = reduzirResolucaoEspacial(imagem, fatorEspacial);
  Mat imagemIntensidade = reduzirResolucaoIntensidade(imagem, niveisCinza);
  Mat imagemCombinada = reduzirResolucaoIntensidade(imagemEspacial, niveisCinza);

  cout << "Comparacao visual:" << endl;
  cout << "1. Perda espacial: reduz detalhes finos e deixa a imagem pixelizada, mas os tons continuam relativamente suaves." << endl;
  cout << "2. Perda de intensidade: preserva contornos espaciais, mas cria faixas de cinza (banding) por falta de tons intermediarios." << endl;
  cout << "3. Efeito combinado: une pixelizacao e banding, causando maior degradacao perceptiva geral." << endl;

  imshow("Original (256 niveis)", imagem);
  imshow("Perda de resolucao espacial", imagemEspacial);
  imshow("Perda de resolucao de intensidade", imagemIntensidade);
  imshow("Perda combinada", imagemCombinada);

  waitKey(0);
  return 0;
}