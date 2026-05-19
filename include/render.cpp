#include "../include/render.h"
#include <cassert>
#include <iostream>

cv::Mat Render::render(const cv::Mat& arquivo, int escolha, double alfa, double beta) {
    imagem = arquivo.clone();

    if (arquivo.empty()) {
        std::cerr << "Erro: O arquivo de mídia fornecido está vazio ou não pôde ser aberto!" << std::endl;
        return arquivo;
    }

    while (true) {
        leitor >> imagem;
        if (imagem.empty()) {
            break;
        }

        switch (escolha) {
            case 1: resultado = girar(imagem, alfa); break;
            case 2: resultado = recortar(imagem, alfa, beta); break;
            case 3: resultado = desfocar(imagem, alfa); break;
            case 4: resultado = limpar(imagem, alfa); break;
            case 5: resultado = remover(imagem, alfa); break;
            case 6: resultado = contraste(imagem, alfa); break;
            case 7: resultado = brilho(imagem, alfa); break;
            case 8: resultado = cores(imagem, alfa, beta); break; // Atualizado para passar alfa e beta
            case 9: resultado = nitidez(imagem, alfa); break;
            case 10: resultado = cinzas(imagem, alfa); break;
            default:
                std::cout << "Opção invalida. Mantendo arquivo original.\n";
                resultado = imagem.clone();
                break;
        }

        if (gravador.isOpened()) {
            gravador.write(resultado);
        }

        cv::imshow("Convertendo Video...", resultado);

        int tecla = cv::waitKey(1);

        if (tecla == 'c') {
            std::cout << "Conversão cancelada!\n";
            break;
        }
        if (tecla == 'p') {
            std::cout << "Conversao pausada. Aperte novamente para continuar\n";
            while (true) {
                int pausa = cv::waitKey(30);
                if (pausa == 'p') {
                    std::cout << "Conversao retomada.\n";
                    break;
                }
            }
        }
        else if (tecla == 'r') {
            std::cout << "Conversão reiniciada\n";
            leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
        }
    }
    return resultado;
}

cv::Mat Render::girar(const cv::Mat& arquivo, double alfa) {
    cv::Point2f centro(arquivo.cols / 2.0f, arquivo.rows / 2.0f);
    cv::Mat rotacao = cv::getRotationMatrix2D(centro, alfa, 1.0);
    cv::warpAffine(arquivo, resultado, rotacao, arquivo.size());

    return resultado;
}

cv::Mat Render::recortar(const cv::Mat& arquivo, double alfa, double beta) {
    static int x = -1, y = -1;

    int largura = static_cast<int>(alfa);
    int altura = static_cast<int>(beta);

    if (x == -1 && y == -1) {
        std::cout << "\n CORTE DE IMAGEM \n";
        std::cout << "As dimensões atuais são: " << arquivo.cols << "x" << arquivo.rows << "\n";

        std::cout << "Digite o ponto horizontal inicial (da esquerda para a direita): ";
        std::cin >> x;
        std::cout << "Digite o ponto vertical inicial (de cima para baixo): ";
        std::cin >> y;
    }
    if (x < 0) { x = 0; }
    if (y < 0) { y = 0; }
    if (x + largura > arquivo.cols) { largura = arquivo.cols - x; }
    if (y + altura > arquivo.rows)  { altura = arquivo.rows - y; }

    cv::Rect areaCorte(x, y, largura, altura);
    resultado = arquivo(areaCorte).clone();

    return resultado;
}

cv::Mat Render::desfocar(const cv::Mat& arquivo, double alfa) {
    int ksize = static_cast<int>(alfa);

    if (ksize <= 0) {
        ksize = 1;
    }
    else if (ksize % 2 == 0) {
        ksize = ksize + 1;
    }

    cv::GaussianBlur(arquivo, resultado, cv::Size(ksize, ksize), 0);
    return resultado;
}

cv::Mat Render::limpar(const cv::Mat& arquivo, double alfa) {
    float intensidade = static_cast<float>(alfa);

    if (intensidade > 10.0f) { intensidade = 10.0f; }
    if (intensidade < 0.0f)  { intensidade = 0.0f; }

    cv::fastNlMeansDenoisingColored(arquivo, resultado, intensidade, intensidade, 7, 21);
    return resultado;
}

cv::Mat Render::remover(const cv::Mat& arquivo, double alfa) {
    int tamanho = static_cast<int>(alfa);

    if (tamanho < 3)     { tamanho = 3; }
    if (tamanho % 2 == 0){ tamanho += 1; }

    cv::medianBlur(arquivo, resultado, tamanho);
    return resultado;
}

cv::Mat Render::contraste(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 50.0;
    arquivo.convertTo(resultado, -1, alfa, 0);
    return resultado;
}

cv::Mat Render::brilho(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = (alfa - 50.0) * 2.54;
    arquivo.convertTo(resultado, -1, 1.0, alfa);
    return resultado;
}

cv::Mat Render::cores(const cv::Mat& arquivo, double alfa, double beta) {
    int cor = static_cast<int>(beta);

    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = alfa / 50.0;

    std::vector<cv::Mat> canais;
    cv::split(arquivo, canais);

    if (cor == 1)      { canais[0] = canais[0] * alfa; } // Azul
    else if (cor == 2) { canais[1] = canais[1] * alfa; } // Verde
    else if (cor == 3) { canais[2] = canais[2] * alfa; } // Vermelho

    cv::merge(canais, resultado);
    return resultado;
}

cv::Mat Render::nitidez(const cv::Mat& arquivo, double alfa) {
    if (alfa < 0.0)   { alfa = 0.0; }
    if (alfa > 100.0) { alfa = 100.0; }

    alfa = (alfa - 50.0) / 25.0;

    cv::Mat gama;
    cv::GaussianBlur(arquivo, gama, cv::Size(3, 3), 0);
    cv::addWeighted(arquivo, 1.0 + alfa, gama, -alfa, 0, resultado);

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

Render::Render(const std::string& arquivo) {
    leitor.open(arquivo);

    if (!leitor.isOpened()) {
        std::cout << "Erro ao abrir o arquivo" << std::endl;
    }
    assert(leitor.isOpened());
    pausar = false;
    cancelar = false;
    reiniciar = false;
}

Render::~Render() {
    if (leitor.isOpened())   { leitor.release(); }
    if (gravador.isOpened()) { gravador.release(); }
}

void Render::janela() {
    cv::Mat frame;
    cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);

    while (true) {
        if (!pausar) {
            leitor >> frame;
            if (frame.empty()) {
                break;
            }
            cv::imshow("Reprodutor", frame);
        }

       int tecla = cv::waitKey(30);

        if (tecla == 'c') {
            break;
        }
        if (tecla == 'p') {
            pausar = !pausar;
        }
        else if (tecla == 'r') {
            leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
            pausar = false;
        }
    }
    cv::destroyWindow("Reprodutor");
}