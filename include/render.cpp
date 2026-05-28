#include "../include/render.h"
#include <iostream>

Render::Render() : pausar(false), cancelar(false), reiniciar(false){
}

Render::Render(const std::string& arquivo) {
    leitor.open(arquivo);
    pausar = false;
    cancelar = false;
    reiniciar = false;

    if (!leitor.isOpened()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
    }
}

cv::Mat Render::render(int escolha, const Parametros& filtro) {
    while (true) {
        leitor >> imagem;
        if (imagem.empty()) break;

        switch (escolha) {
        case 1: resultado = girar(imagem, filtro.alfa, filtro.gama); break;
        case 2: resultado = recortar(imagem, filtro.gama, filtro.delta); break;
        case 3: resultado = nitidez(imagem, filtro.alfa); break;
        case 4: resultado = desfocar(imagem, filtro.gama); break;
        case 5: resultado = remover(imagem, filtro.gama); break;
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

cv::imshow("Convertendo Video...", resultado);
    int tecla = cv::waitKey(1);

        if (tecla == 'c') {
            std::cout << "Conversão cancelada!\n";
            break;
            }

        if (tecla == 'p') {
            std::cout << "Conversão pausada. Aperte novamente para continuar\n";
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

cv::Mat Render::render(const std::vector<int>& escolhas, const Parametros& filtro) {
    leitor >> imagem;
    cv::Mat processada = imagem.clone();

    for (int escolha : escolhas) {
        switch (escolha) {
        case 1: processada = girar(processada, filtro.alfa, filtro.gama); break;
        case 2: processada = recortar(processada, filtro.gama, filtro.delta); break;
        case 3: processada = nitidez(processada, filtro.alfa); break;
        case 4: processada = desfocar(processada, filtro.gama); break;
        case 5: processada = remover(processada, filtro.gama); break;
        case 6: processada = limpar(processada, filtro.beta); break;
        case 7: processada = brilho(processada, filtro.alfa); break;
        case 8: processada = contraste(processada, filtro.alfa); break;
        case 9: processada = cores(processada, filtro.alfa, filtro.gama); break;
        case 10: processada = cinzas(processada, filtro.alfa); break;
        default:
            break;
        }
    }
    return processada;
}

void Render::demo(int escolha, const Parametros& filtro) {
    cv::Mat frame;

    bool eh_webcam = (leitor.get(cv::CAP_PROP_FRAME_COUNT) <= 0);

    std::cout << "\n[DEMO] Iniciando exibição. Pressione 'ESC' na janela para fechar.\n";

    while (true) {
        leitor >> frame;

        if (frame.empty()) {
            if (eh_webcam) {
                cv::waitKey(10);
                continue;
            } else {
                break;
            }
        }

        switch (escolha) {
        case 1: resultado = girar(frame, filtro.alfa, filtro.gama); break;
        case 2: resultado = recortar(frame, filtro.gama, filtro.delta); break;
        case 3: resultado = nitidez(frame, filtro.alfa); break;
        case 4: resultado = desfocar(frame, filtro.gama); break;
        case 5: resultado = remover(frame, filtro.gama); break;
        case 6: resultado = limpar(frame, filtro.beta); break;
        case 7: resultado = brilho(frame, filtro.alfa); break;
        case 8: resultado = contraste(frame, filtro.alfa); break;
        case 9: resultado = cores(frame, filtro.alfa, filtro.gama); break;
        case 10: resultado = cinzas(frame, filtro.alfa); break;
        default: resultado = frame.clone(); break;
        }

        cv::imshow("Demo Tempo Real", resultado);

        int tecla = cv::waitKey(30);
        if (tecla == 27) {
            break;
        }
    }
    cv::destroyWindow("Demo Tempo Real");
}

void Render::camera(int dispositivo){
    if (leitor.isOpened()) {
        leitor.release();
    }

    if (!leitor.open(dispositivo, cv::CAP_DSHOW)){
        std::cerr << "Não foi possível acessar a câmera." << std::endl;
    }
}

void Render::midia(const std::string& arquivo)
{
    leitor.open(arquivo);

    pausar = false;
    cancelar = false;
    reiniciar = false;

    if (!leitor.isOpened())
    {
        std::cout << "Erro ao abrir o arquivo" << std::endl;
    }
}

Render::~Render(){
        if (leitor.isOpened())   { leitor.release(); }
        if (gravador.isOpened()) { gravador.release(); }
    }

void Render::janela(){
        cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);

        while (true) {
            if (!pausar) {
                leitor >> imagem;
                if (imagem.empty()) {
                    break;
                }
                cv::imshow("Reprodutor", imagem);
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