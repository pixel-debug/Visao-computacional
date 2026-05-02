#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

cv::Mat preprocess(const cv::Mat &img)
{
  cv::Mat gray, eq, blurred;
  cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

  // CLAHE — equalização adaptativa para melhorar contraste das bordas
  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
  clahe->apply(gray, eq);

  // Filtro Gaussiano (9×9, σ=2) — reduz ruído e textura dos alimentos
  cv::GaussianBlur(eq, blurred, cv::Size(9, 9), 2, 2);

  return blurred;
}

std::vector<cv::Vec3f> detectCircles(const cv::Mat &blurred)
{
  int h = blurred.rows;
  int w = blurred.cols;
  int minDim = std::min(h, w);

  int minR = minDim / 7;
  int maxR = minDim / 2 + 30;

  double minDist = static_cast<double>(std::max(h, w)) / 3.0;

  std::vector<cv::Vec3f> circles;

  // --- Tentativa 1: parâmetros padrão ---
  cv::HoughCircles(blurred, circles,
                   cv::HOUGH_GRADIENT,
                   /*dp=*/1.2,
                   /*minDist=*/minDist,
                   /*param1=*/100, // limiar superior do Canny
                   /*param2=*/40,  // limiar do acumulador
                   minR, maxR);

  if (!circles.empty())
    return circles;

  // --- Tentativa 2: reduz param2 para capturar círculos mais fracos ---
  cv::HoughCircles(blurred, circles,
                   cv::HOUGH_GRADIENT,
                   1.2, minDist,
                   /*param1=*/80,
                   /*param2=*/25,
                   minR, maxR);

  if (!circles.empty())
    return circles;

  // --- Tentativa 3: suavização extra + param2 ainda menor ---
  cv::Mat blurred2;
  cv::GaussianBlur(blurred, blurred2, cv::Size(15, 15), 3, 3);
  cv::HoughCircles(blurred2, circles,
                   cv::HOUGH_GRADIENT,
                   1.5, minDist,
                   /*param1=*/60,
                   /*param2=*/20,
                   minR, maxR);

  return circles;
}

// ──────────────────────────────────────────────────────────────────────────────
// Mantém apenas o maior círculo detectado (o prato principal)
// ──────────────────────────────────────────────────────────────────────────────
std::vector<cv::Vec3f> keepLargest(const std::vector<cv::Vec3f> &circles)
{
  if (circles.size() <= 1)
    return circles;

  auto it = std::max_element(circles.begin(), circles.end(),
                             [](const cv::Vec3f &a, const cv::Vec3f &b)
                             { return a[2] < b[2]; });

  return {*it};
}

// ──────────────────────────────────────────────────────────────────────────────
// Anota a imagem com os círculos detectados e um rótulo de status
// ──────────────────────────────────────────────────────────────────────────────
cv::Mat annotate(const cv::Mat &img, const std::vector<cv::Vec3f> &circles)
{
  cv::Mat out = img.clone();

  for (const auto &c : circles)
  {
    cv::Point center(cvRound(c[0]), cvRound(c[1]));
    int radius = cvRound(c[2]);

    // Círculo externo — vermelho
    cv::circle(out, center, radius, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    // Centro — verde
    cv::circle(out, center, 5, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);
  }

  // Rótulo no canto superior esquerdo
  std::string label;
  cv::Scalar color;
  if (circles.empty())
  {
    label = "NAO DETECTADO";
    color = cv::Scalar(0, 0, 200);
  }
  else
  {
    label = "DETECTADO (" + std::to_string(static_cast<int>(circles.size())) + ")";
    color = cv::Scalar(0, 180, 0);
  }

  // Fundo escuro para o texto
  int baseline = 0;
  double fontScale = 0.75;
  int thickness = 2;
  cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
  cv::rectangle(out, cv::Point(5, 5), cv::Point(15 + ts.width, 20 + ts.height),
                cv::Scalar(0, 0, 0), cv::FILLED);
  cv::putText(out, label, cv::Point(10, 10 + ts.height),
              cv::FONT_HERSHEY_SIMPLEX, fontScale, color, thickness, cv::LINE_AA);

  return out;
}

// ──────────────────────────────────────────────────────────────────────────────
int main()
{
  const std::string input_dir = "sel_data";
  const std::string output_dir = "output";

  // Criar diretório de saída
  fs::create_directories(output_dir);

  // Coletar e ordenar imagens
  std::vector<fs::path> images;
  for (const auto &entry : fs::directory_iterator(input_dir))
    if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png")
      images.push_back(entry.path());
  std::sort(images.begin(), images.end());

  if (images.empty())
  {
    std::cerr << "Nenhuma imagem encontrada em '" << input_dir << "'\n";
    return 1;
  }

  int total = 0;
  int detected = 0; // 1 ou mais círculos encontrados
  int notFound = 0; // nenhum círculo encontrado

  std::cout << "Processando " << images.size() << " imagens...\n\n";

  for (const auto &path : images)
  {
    cv::Mat img = cv::imread(path.string());
    if (img.empty())
    {
      std::cerr << "  [ERRO] Não foi possível ler: " << path << "\n";
      continue;
    }
    ++total;

    // Pré-processamento
    cv::Mat blurred = preprocess(img);

    // Detecção de círculos
    std::vector<cv::Vec3f> circles = detectCircles(blurred);

    // Mantém apenas o maior (prato principal)
    circles = keepLargest(circles);

    // Contabilização
    if (circles.empty())
      ++notFound;
    else
      ++detected;

    // Imagem anotada
    cv::Mat result = annotate(img, circles);

    // Salvar resultado
    std::string outpath = output_dir + "/" + path.filename().string();
    cv::imwrite(outpath, result);

    std::printf("  %-12s → %s\n",
                path.filename().string().c_str(),
                circles.empty() ? "não detectado"
                                : ("detectado  r=" + std::to_string(cvRound(circles[0][2])) + "px").c_str());
  }

  // ── Resumo final ──────────────────────────────────────────────────────────
  std::cout << "\n════════════════════════════════════════\n";
  std::cout << "  RESUMO DE DETECÇÃO\n";
  std::cout << "════════════════════════════════════════\n";
  std::printf("  %-38s %d\n", "Detectado corretamente / parcialmente:", detected);
  std::printf("  %-38s %d\n", "Não detectado / incorreto:", notFound);
  std::printf("  %-38s %d\n", "Total:", total);
  std::cout << "════════════════════════════════════════\n";
  std::cout << "Imagens salvas em: " << output_dir << "/\n";

  return 0;
}
