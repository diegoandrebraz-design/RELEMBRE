#include "../include/render.h"
#include <cassert>
#include <iostream>

cv::Mat Render::render(const cv::Mat& arquivo, int escolha, double alfa, float beta, int gama, int delta) {
    imagem = arquivo.clone();

    if (arquivo.empty()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return arquivo;
    }

    while (true) {
        leitor >> imagem;
        if (imagem.empty()) {
            break;
        }

        switch (escolha) {
            case 1: resultado = girar(imagem, alfa, gama); break;
            case 2: resultado = recortar(imagem, gama, delta); break;
            case 3: resultado = nitidez(imagem, alfa); break;
            case 4: resultado = desfocar(imagem, gama); break;
            case 5: resultado = remover(imagem, gama); break;
            case 6: resultado = limpar(imagem, beta); break;
            case 7: resultado = brilho(imagem, alfa); break;
            case 8: resultado = contraste(imagem, alfa); break;
            case 9: resultado = cores(imagem, alfa, gama); break;
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

cv::Mat Render::girar(const cv::Mat& arquivo, double alfa, int gama) {
    
    cv::Point2f centro(static_cast<float>(arquivo.cols) / 2.0f, static_cast<float>(arquivo.rows) / 2.0f);
    cv::Mat rotacao = cv::getRotationMatrix2D(centro, alfa, 1.0);

    cv::warpAffine(arquivo, resultado, rotacao, arquivo.size());

    if (gama == 1 || gama == 0 || gama == -1) {
        cv::flip(resultado, resultado, gama);
    }
    return resultado;
}

cv::Mat Render::recortar(const cv::Mat& arquivo, int gama, int delta) {
    static int esquerda = -1, topo = -1;

    int largura = gama;
    int altura = delta;

    if (esquerda == -1 && topo == -1) {
        std::cout << "\n CORTE DE IMAGEM \n";
        std::cout << "As dimensões atuais são: " << arquivo.cols << "x" << arquivo.rows << "\n";

        std::cout << "Digite o ponto horizontal inicial (da esquerda para a direita): ";
        std::cin >> esquerda;
        std::cout << "Digite o ponto vertical inicial (de cima para baixo): ";
        std::cin >> topo;
    }
    if (esquerda < 0) { esquerda = 0; }
    if (topo < 0) { topo = 0; }
    if (esquerda + largura > arquivo.cols) { largura = arquivo.cols - esquerda; }
    if (topo + altura > arquivo.rows)  { altura = arquivo.rows - topo; }

    cv::Rect areaCorte(esquerda, topo, largura, altura);
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

cv::Mat Render::desfocar(const cv::Mat& arquivo, int gama){
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

cv::Mat Render::remover(const cv::Mat& arquivo, int gama) {

    if (gama < 3)     { gama = 3; }
    if (gama % 2 == 0){ gama += 1; }

    cv::medianBlur(arquivo, resultado, gama);
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