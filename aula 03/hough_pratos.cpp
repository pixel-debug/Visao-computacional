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

  int minR = static_cast<int>(minDim * 0.25); // exclui círculos de comida (rNorm≈0.22) em imagens grandes
  int maxR = static_cast<int>(minDim * 0.52); // cap em 52% — evita arcos espúrios muito grandes

  // minDist pequeno: permite retornar círculos concêntricos (comida + borda do prato)
  // para que selectBest escolha o de raio mais próximo ao ideal (rn≈0.42)
  double minDist = minDim * 0.15;

  std::vector<cv::Vec3f> circles;

  // --- Tentativa 0: HOUGH_GRADIENT_ALT — gradiente Scharr, mais preciso geometricamente ---
  // param1=150 (limiar Scharr baixo — detecta bordas fracas como pratos transparentes)
  // param2=0.70 (perfectness moderado — inclui borda de vidro com baixo contraste)
  cv::HoughCircles(blurred, circles,
                   cv::HOUGH_GRADIENT_ALT,
                   /*dp=*/1.5,
                   /*minDist=*/minDist,
                   /*param1=*/150,
                   /*param2=*/0.70,
                   minR, maxR);

  if (!circles.empty())
    return circles;

  // --- Tentativa 1: HOUGH_GRADIENT padrão ---
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
// Seleciona o melhor círculo: maximiza centralidade + raio próximo ao ideal.
// Score = (1 - dc_norm) * 0.6  +  (1 - |rn - 0.42| / 0.25) * 0.4
//   dc_norm: distância do centro ao centro da imagem, normalizada por minDim
//   rn:      raio normalizado por minDim  (ideal ≈ 0.42 para pratos)
// ──────────────────────────────────────────────────────────────────────────────
std::vector<cv::Vec3f> selectBest(const std::vector<cv::Vec3f> &circles, int imgH, int imgW)
{
  if (circles.empty())
    return {};
  if (circles.size() == 1)
    return {circles[0]};

  double cx0 = imgW / 2.0, cy0 = imgH / 2.0;
  double minDim = std::min(imgH, imgW);
  constexpr double IDEAL_RN = 0.42;
  constexpr double RN_RANGE = 0.25;
  constexpr double W_CENTER = 0.60;
  constexpr double W_RADIUS = 0.40;

  double bestScore = -1e9;
  cv::Vec3f best = circles[0];

  for (const auto &c : circles)
  {
    double dc = std::sqrt((c[0] - cx0) * (c[0] - cx0) +
                          (c[1] - cy0) * (c[1] - cy0)) /
                minDim;
    double rn = c[2] / minDim;
    double score = W_CENTER * (1.0 - dc) +
                   W_RADIUS * (1.0 - std::abs(rn - IDEAL_RN) / RN_RANGE);
    if (score > bestScore)
    {
      bestScore = score;
      best = c;
    }
  }
  return {best};
}

cv::Mat annotate(const cv::Mat &img, const std::vector<cv::Vec3f> &circles)
{
  cv::Mat out = img.clone();

  for (const auto &c : circles)
  {
    cv::Point center(cvRound(c[0]), cvRound(c[1]));
    int radius = cvRound(c[2]);

    cv::circle(out, center, radius, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    cv::circle(out, center, 5, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);
  }

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

int main()
{
  const std::string input_dir = "sel_data";
  const std::string output_dir = "output";

  fs::create_directories(output_dir);
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
  int detected = 0;
  int notFound = 0;

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

    cv::Mat blurred = preprocess(img);
    std::vector<cv::Vec3f> circles = detectCircles(blurred);
    circles = selectBest(circles, img.rows, img.cols);

    if (circles.empty())
      ++notFound;
    else
      ++detected;

    cv::Mat result = annotate(img, circles);
    std::string outpath = output_dir + "/" + path.filename().string();
    cv::imwrite(outpath, result);
    return 0;
  }
