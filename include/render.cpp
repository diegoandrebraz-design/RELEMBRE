#include "../include/render.h"
#include <iostream>
#include <algorithm>

Render::Render() = default;

Render::Render(const std::string& arquivo) {
    leitor.open(arquivo);

    if (!leitor.isOpened()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
    }
}

cv::Mat Render::render(int escolha, const Parametros& filtro) {
    leitor >> imagem;
    if (imagem.empty()) return {};

    switch (escolha) {
    case 1: resultado = girar(imagem, filtro.alfa, filtro.gama); break;
    case 2: resultado = recortar(imagem, filtro.gama, filtro.delta); break;
    case 3: resultado = granular(imagem, filtro.alfa); break;
    case 4: resultado = nitidez(imagem, filtro.alfa); break;
    case 5: resultado = desfocar(imagem, filtro.gama); break;
    case 6: resultado = remover(imagem, filtro.alfa); break;
    case 7: resultado = limpar(imagem, filtro.beta); break;
    case 8: resultado = brilho(imagem, filtro.alfa); break;
    case 9: resultado = contraste(imagem, filtro.alfa); break;
    case 10: resultado = cores(imagem, filtro.alfa, filtro.gama); break;
    case 11: resultado = cinzas(imagem, filtro.alfa); break;
    default:
        std::cout << "Opção invalida!\n";
        resultado = imagem.clone();
        break;
    }

    if (gravador.isOpened()) gravador.write(resultado);

    return resultado;
}

cv::Mat Render::render(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    leitor >> imagem;
    if (imagem.empty()) return {};
    cv::Mat processada = imagem.clone();

    for (size_t i = 0; i < escolhas.size(); ++i) {
        switch (escolhas[i]) {
        case 1: processada = girar(processada, filtro[i].alfa, filtro[i].gama); break;
        case 2: processada = recortar(processada, filtro[i].gama, filtro[i].delta); break;
        case 3: processada = granular(processada, filtro[i].alfa); break;
        case 4: processada = nitidez(processada, filtro[i].alfa); break;
        case 5: processada = desfocar(processada, filtro[i].gama); break;
        case 6: processada = remover(processada, filtro[i].alfa); break;
        case 7: processada = limpar(processada, filtro[i].beta); break;
        case 8: processada = brilho(processada, filtro[i].alfa); break;
        case 9: processada = contraste(processada, filtro[i].alfa); break;
        case 10: processada = cores(processada, filtro[i].alfa, filtro[i].gama); break;
        case 11: processada = cinzas(processada, filtro[i].alfa); break;
        default: break;
        }
    }
    return processada;
}

cv::Mat Render::comparar(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    bool camera = (leitor.get(cv::CAP_PROP_FRAME_COUNT) <= 0);
    bool pausado = false;

    cv::namedWindow("Comparacao Lado a Lado", cv::WINDOW_NORMAL);

    while (true) {
        if (!pausado) {
            cv::Mat processado = render(escolhas, filtro);

            if (imagem.empty() || processado.empty()) {
                if (camera) {
                    cv::waitKey(10);
                    continue;
                } else {
                    break;
                }
            }

            cv::Mat original = imagem.clone();

            if (original.size() != processado.size()) {
                cv::resize(original, original, processado.size());
            }

            if (processado.channels() == 1 && original.channels() == 3) {
                cv::cvtColor(processado, processado, cv::COLOR_GRAY2BGR);
            } else if (processado.channels() == 3 && original.channels() == 1) {
                cv::cvtColor(original, original, cv::COLOR_GRAY2BGR);
            }

            cv::hconcat(original, processado, resultado);

            cv::resizeWindow("Comparacao Lado a Lado", 1600, 450);
            cv::imshow("Comparacao Lado a Lado", resultado);
        }

        int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 'C' || tecla == 27) {
            break;
        }
        if (tecla == 'p') {
            pausado = !pausado;
        }
        if (tecla == 'r') {
            if (!camera) {
                leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                pausado = false;
            }
        }
    }
    cv::destroyWindow("Comparacao Lado a Lado");
    return resultado;
}

cv::Mat Render::girar(const cv::Mat& arquivo, double alfa, int gama) {
    cv::Point2f centro(static_cast<float>(arquivo.cols) / 2.0f, static_cast<float>(arquivo.rows) / 2.0f);
    cv::Mat mocao = cv::getRotationMatrix2D(centro, alfa, 1.0);

    cv::warpAffine(arquivo, resultado, mocao, arquivo.size());

    if (gama == 1 || gama == 0 || gama == -1) {
        cv::flip(resultado, resultado, gama);
    }
    return resultado;
}

void Render::recorte(int x, int y) {
    esquerda = x;
    topo = y;
}

cv::Mat Render::recortar(const cv::Mat& arquivo, int gama, int delta) {
    if (gama <= 1 || delta <= 1) {
        return arquivo.clone();
    }

    if (esquerda < 0) esquerda = 0;
    if (topo < 0) topo = 0;

    if (esquerda + gama > arquivo.cols) {
        gama = arquivo.cols - esquerda;
    }

    if (topo + delta > arquivo.rows) {
        delta = arquivo.rows - topo;
    }

    cv::Rect areaCorte(esquerda, topo, gama, delta);
    resultado = arquivo(areaCorte).clone();

    return resultado;
}

cv::Mat Render::granular (const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) return arquivo.clone();
    if (alfa < 0.0) alfa = 0.0;
    if (alfa > 100.0) alfa = 100.0;

    cv::Mat ruidoCinza = cv::Mat::zeros(arquivo.size(), CV_8SC1);
    cv::randn(ruidoCinza, cv::Scalar(0), cv::Scalar(alfa));

    cv::Mat ruidoFinal;
    if (arquivo.channels() == 3) {
        std::vector<cv::Mat> canaisRuido = {ruidoCinza, ruidoCinza, ruidoCinza};
        cv::merge(canaisRuido, ruidoFinal);
    } else {
        ruidoFinal = ruidoCinza;
    }
    cv::Mat mascaraPreto;
    if (arquivo.channels() == 3) {
        cv::Mat hsv;
        cv::cvtColor(arquivo, hsv, cv::COLOR_BGR2HSV);

        cv::Scalar limiteInferiorPreto(0, 0, 0);
        cv::Scalar limiteSuperiorPreto(180, 255, 10);

        cv::inRange(hsv, limiteInferiorPreto, limiteSuperiorPreto, mascaraPreto);
        cv::bitwise_not(mascaraPreto, mascaraPreto);
    } else {
        cv::threshold(arquivo, mascaraPreto, 10, 255, cv::THRESH_BINARY);
    }

    cv::Mat ruidoFiltrado = cv::Mat::zeros(arquivo.size(), arquivo.type());
    ruidoFinal.copyTo(ruidoFiltrado, mascaraPreto);

    cv::Mat resultadoProvisorio;
    cv::add(arquivo, ruidoFiltrado, resultadoProvisorio, cv::Mat(), arquivo.type());

    resultado = resultadoProvisorio;
    return resultado;
}

cv::Mat Render::nitidez(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 12.5;

    cv::Mat epsilon;
    cv::GaussianBlur(arquivo, epsilon, cv::Size(3, 3), 0);
    cv::addWeighted(arquivo, 1.0 + alfa, epsilon, -alfa, 0, resultado);

    return resultado;
}

cv::Mat Render::desfocar(const cv::Mat& arquivo, int gama) {
    int ksize = gama;

    if (ksize <= 0) {
        ksize = 1;
    }
    else if (ksize % 2 == 0) {
        ksize = ksize + 1;
    }

    cv::GaussianBlur(arquivo, resultado, cv::Size(ksize, ksize), 0);
    return resultado;
}

cv::Mat Render::remover(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) return arquivo.clone();

    double sensibilidade = alfa;
    if (sensibilidade <= 0.0) sensibilidade = 40.0;
    if (sensibilidade > 254.0) sensibilidade = 254.0;

    cv::Mat cinza;
    if (arquivo.channels() == 3) {
        cv::cvtColor(arquivo, cinza, cv::COLOR_BGR2GRAY);
    } else {
        cinza = arquivo.clone();
    }

    cv::Mat gradY;
    cv::Sobel(cinza, gradY, CV_16S, 0, 1, 3);
    cv::convertScaleAbs(gradY, gradY);

    cv::Mat mascara;
    cv::threshold(gradY, mascara, sensibilidade, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 1));
    cv::morphologyEx(mascara, mascara, cv::MORPH_OPEN, kernel);

    int espessura = 3;
    if (sensibilidade < 30.0) espessura = 5;

    cv::dilate(mascara, mascara,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(espessura, espessura)));

    if (mascara.type() != CV_8UC1) {
        mascara.convertTo(mascara, CV_8UC1);
    }

    cv::Mat resultado;

    auto raio = static_cast<double>(espessura);

    cv::inpaint(arquivo, mascara, resultado, raio, cv::INPAINT_NS);

    return resultado;
}

cv::Mat Render::limpar(const cv::Mat& arquivo, float beta) {
    if (beta > 10.0f) { beta = 10.0f; }
    if (beta < 0.0f)  { beta = 0.0f; }

    cv::fastNlMeansDenoisingColored(arquivo, resultado, beta, beta, 7, 21);
    return resultado;
}

cv::Mat Render::brilho(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = (alfa - 50.0) * 2.54;
    arquivo.convertTo(resultado, -1, 1.0, alfa);
    return resultado;
}

cv::Mat Render::contraste(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 50.0;
    arquivo.convertTo(resultado, -1, alfa, 0);
    return resultado;
}

cv::Mat Render::cores(const cv::Mat& arquivo, double alfa, int gama) {
    if (arquivo.channels() < 3) return arquivo.clone();

    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 50.0;

    int canalAlvo = gama - 1;
    if (canalAlvo < 0 || canalAlvo > 2) return arquivo.clone();

    cv::Mat hsv, mascaraBranco;
    cv::cvtColor(arquivo, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar limiteInferiorBranco(0, 0, 64);
    cv::Scalar limiteSuperiorBranco(180, 3, 255);

    cv::inRange(hsv, limiteInferiorBranco, limiteSuperiorBranco, mascaraBranco);

    cv::Mat processada = arquivo.clone();

    for (int i = 0; i < processada.rows; i++) {
        for (int j = 0; j < processada.cols; j++) {
            if (mascaraBranco.at<uchar>(i, j) == 255) {
                continue;
            }

            auto& pixel = processada.at<cv::Vec3b>(i, j);

            uchar max_val = std::max({pixel[0], pixel[1], pixel[2]});
            uchar min_val = std::min({pixel[0], pixel[1], pixel[2]});

            if ((max_val - min_val) < 15) {
                continue;
            }

            pixel[canalAlvo] = cv::saturate_cast<uchar>(pixel[canalAlvo] * alfa);
        }
    }

    resultado = processada;
    return resultado;
}

cv::Mat Render::cinzas(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 100.0;

    cv::Mat imagemCinza;
    cv::cvtColor(arquivo, imagemCinza, cv::COLOR_BGR2GRAY);
    cv::cvtColor(imagemCinza, imagemCinza, cv::COLOR_GRAY2BGR);

    cv::addWeighted(arquivo, 1.0 - alfa, imagemCinza, alfa, 0, resultado);
    return resultado;
}

void Render::camera(int dispositivo) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    if (!leitor.open(dispositivo, cv::CAP_DSHOW)) {
        std::cerr << "Não foi possível acessar a câmera." << std::endl;
    }
}

void Render::midia(const std::string& arquivo) {
    leitor.open(arquivo);

    if (!leitor.isOpened()) {
        std::cout << "Erro ao abrir o arquivo" << std::endl;
    }
}

void Render::janela() {
    cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);
    bool localPausar = false;

    while (true) {
        if (!localPausar) {
            leitor >> imagem;
            if (imagem.empty()) {
                break;
            }
            cv::imshow("Reprodutor", imagem);
        }

        int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 27) {
            break;
        }
        if (tecla == 'p') {
            localPausar = !localPausar;
        }
        else if (tecla == 'r') {
            leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
            localPausar = false;
        }
    }
    cv::destroyWindow("Reprodutor");
}

Render::~Render() {
    if (leitor.isOpened())   { leitor.release(); }
    if (gravador.isOpened()) { gravador.release(); }
}