#include "../include/render.h"
#include <iostream>

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
    case 3: resultado = nitidez(imagem, filtro.alfa); break;
    case 4: resultado = desfocar(imagem, filtro.gama); break;
    case 5: resultado = remover(imagem, filtro.alfa); break;
    case 6: resultado = limpar(imagem, filtro.beta); break;
    case 7: resultado = brilho(imagem, filtro.alfa); break;
    case 8: resultado = contraste(imagem, filtro.alfa); break;
    case 9: resultado = cores(imagem, filtro.alfa, filtro.gama); break;
    case 10: resultado = cinzas(imagem, filtro.alfa); break;
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
        case 3: processada = nitidez(processada, filtro[i].alfa); break;
        case 4: processada = desfocar(processada, filtro[i].gama); break;
        case 5: processada = remover(processada, filtro[i].alfa); break;
        case 6: processada = limpar(processada, filtro[i].beta); break;
        case 7: processada = brilho(processada, filtro[i].alfa); break;
        case 8: processada = contraste(processada, filtro[i].alfa); break;
        case 9: processada = cores(processada, filtro[i].alfa, filtro[i].gama); break;
        case 10: processada = cinzas(processada, filtro[i].alfa); break;
        default: break;
        }
    }
    return processada;
}

cv::Mat Render::comparar(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    bool camera = (leitor.get(cv::CAP_PROP_FRAME_COUNT) <= 0);
    bool pausado = false;

    while (true) {
        if (!pausado) {
            if (leitor.isOpened()) {
                double pos = leitor.get(cv::CAP_PROP_POS_FRAMES);
                leitor >> imagem;
                leitor.set(cv::CAP_PROP_POS_FRAMES, pos);
            }

            if (imagem.empty()) {
                if (camera) {
                    cv::waitKey(10);
                    continue;
                } else {
                    break;
                }
            }

            cv::Mat processado = render(escolhas, filtro);
            if (processado.empty()) {
                break;
            }

            if (imagem.size() != processado.size()) {
                cv::resize(imagem, imagem, processado.size());
            }

            if (processado.channels() == 1 && imagem.channels() == 3) {
                cv::cvtColor(processado, processado, cv::COLOR_GRAY2BGR);
            } else if (processado.channels() == 3 && imagem.channels() == 1) {
                cv::cvtColor(imagem, imagem, cv::COLOR_GRAY2BGR);
            }

            cv::hconcat(imagem, processado, resultado);
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

cv::Mat Render::nitidez(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = (alfa - 50.0) / 25.0;

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

    if (alfa <= 0.0) {
        alfa = 3.0;
    }

    cv::Mat mascara;
    cv::Mat cinza;
    if (arquivo.channels() == 3) {
        cv::cvtColor(arquivo, cinza, cv::COLOR_BGR2GRAY);
    } else {
        cinza = arquivo.clone();
    }
    cv::threshold(cinza, mascara, 250, 255, cv::THRESH_BINARY);

    cv::inpaint(arquivo, mascara, resultado, alfa, cv::INPAINT_TELEA);

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

    std::vector<cv::Mat> canais;
    cv::split(arquivo, canais);

    if (gama == 1)      { canais[0] = canais[0] * alfa; }
    else if (gama == 2) { canais[1] = canais[1] * alfa; }
    else if (gama == 3) { canais[2] = canais[2] * alfa; }

    cv::merge(canais, resultado);
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
