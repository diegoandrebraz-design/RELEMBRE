#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "../include/render.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;

void Manual() {
    std::cout << "\n         RESGATE | RESTAURE | RELEMBRE         " << std::endl;
    std::cout << "\n Para iniciar, escolha um das versões abaixo:\n" << std::endl;
    std::cout << "* Digite: free (para o Modo Iniciante)." << std::endl;
    std::cout << "* Digite: pro (para o Modo Profissional)." << std::endl;
    std::cout << "* Digite: demo (para o Modo de Demonstração)." << std::endl;
}

void Filtros() {
    std::cout << "\n            FILTROS DISPONÍVEIS            \n";
    std::cout << " [1] Girar              [2] Recortar\n";
    std::cout << " [3] Ajustar Nitidez    [4] Desfocar\n";
    std::cout << " [5] Remover Falhas     [6] Reduzir Ruídos\n";
    std::cout << " [7] Ajustar Brilho     [8] Ajustar Contraste\n";
    std::cout << " [9] Alterar Cores      [10] Escala de Cinzas\n";
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::string modo;

    if (argc < 2){
        Manual();
        std::cout << "\nEscolha a versão (free, pro, demo): \n";
        std::cin >> modo;
    }
    else {
        modo = argv[1];
    }

    if (modo == "free") {
        std::string entrada = " ";
        int opcao;

        std::cout << "Iniciando a versão para iniciantes..." << std::endl;
        std::cout << "[1] Abrir um arquivo de mídia" << std::endl;
        std::cout << "[2] Sincronizar a Webcam" << std::endl;
        std::cout << "Opção: ";
        std::cin >> opcao;

        Render processador;

        if (opcao == 1) {
            std::cout << "Informe o caminho do arquivo: ";
            std::cin >> entrada;
            processador.midia(entrada);
        }
        else if (opcao == 2) {
            processador.camera(0);
            std::cout << "Acessando a Webcam..." << std::endl;

            if (!processador.leitor.isOpened()) {
                std::cout << "Erro! Não foi possível abrir a câmera." << std::endl;
                return -1;
            }
        }
        else {
            std::cout << "Opção inválida." << std::endl;
            return -1;
        }

        Filtros();
        int escolha;
        std::cout << "Digite o número do filtro: ";
        std::cin >> escolha;

        int gama = 1, delta = 1;

        if (escolha == 2) {
            int esquerda, topo, direita, base;

            std::cout << "Digite o ponto do ESQUERDA (X inicial): ";
            std::cin >> esquerda;
            std::cout << "Digite o ponto do TOPO (Y inicial): ";
            std::cin >> topo;
            std::cout << "Digite o ponto da DIREITA (X final): ";
            std::cin >> direita;
            std::cout << "Digite o ponto da BASE (Y final): ";
            std::cin >> base;

            gama = direita - esquerda;
            delta = base - topo;
            processador.recorte(esquerda, topo);
        }

        std::string extensao = std::filesystem::path(entrada).extension().string();

        if (opcao == 1 && (entrada.find(".png") != std::string::npos || entrada.find(".jpg") != std::string::npos)) {
            cv::Mat resultado = processador.render(escolha, {50.0, 50.0f, gama, delta});

            if (resultado.empty()){
                std::cerr << "Erro! O filtro retornou uma imagem vazia." << std::endl;
                return -1;
            }

            cv::imshow("Finalizado!", resultado);

            std::string saida_imagem = "output/resultado_processado" + extensao;
            cv::imwrite(saida_imagem, resultado);
            std::cout << "Imagem salva com sucesso como: " << saida_imagem << std::endl;

            cv::waitKey(0);
        }
        else {
            cv::VideoWriter gravador;
            std::string arquivo_saida;

            if (opcao == 2) {
                arquivo_saida = "output/resultado_webcam.mp4";
            } else {
                arquivo_saida = "output/resultado_processado" + extensao;
            }

            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            bool pausado = false;

            while (true) {
                if (!pausado) {
                    cv::Mat resultado = processador.render(escolha, {50.0, 50.0f, gama, delta});

                    if (resultado.empty()) {
                        if (opcao == 2) {
                            cv::waitKey(10);
                            continue;
                        } else {
                            break;
                        }
                    }

                    if (!gravador.isOpened()) {
                        gravador.open(arquivo_saida, codec, 30.0, resultado.size());
                    }

                    if (gravador.isOpened()) {
                        gravador.write(resultado);
                    }

                    cv::imshow("Resultado - Stream", resultado);
                }

                int tecla = cv::waitKey(30);
                if (tecla == 'c' || tecla == 27) {
                    break;
                }
                if (tecla == 'p') {
                    pausado = !pausado;
                }
                if (tecla == 'r') {
                    if (opcao != 2) {
                        processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                        pausado = false;
                    }
                }
            }

            if (gravador.isOpened()) {
                gravador.release();
                std::cout << "\nVídeo salva com sucesso em: " << arquivo_saida << std::endl;
            }
            cv::destroyWindow("Resultado - Stream");
        }
    }

    else if (modo == "pro")
    {
        std::string entrada;
        std::cout << "Informe o caminho da mídia (ou digite 'webcam' para usar a câmera): ";
        std::cin >> entrada;

        Render processador;
        bool camera = false;

        if (entrada == "webcam") {
            processador.camera(0);
            camera = true;
            if (!processador.leitor.isOpened()) {
                std::cout << "Erro ao abrir a câmera." << std::endl;
                return -1;
            }

            cv::Mat frame_aquecimento;
            for (int i = 0; i < 10; i++) {
                processador.leitor >> frame_aquecimento;
                if (!frame_aquecimento.empty()) break;
                cv::waitKey(30);
            }
        } else {
            processador.midia(entrada);
        }

        std::vector<int> sequencia;
        int escolha = -1;
        int gama = 1, delta = 1;

        Filtros();
        std::cout << "Digite os números dos filtros na sequência desejada (0 para finalizar): " << std::endl;
        while (escolha != 0) {
            std::cin >> escolha;
            if (escolha > 0 && escolha <= 10) {
                sequencia.push_back(escolha);

                if (escolha == 2) {
                    int esquerda, topo, direita, base;
                    std::cout << "Digite o ponto do ESQUERDA (X inicial): ";
                    std::cin >> esquerda;
                    std::cout << "Digite o ponto do TOPO (Y inicial): ";
                    std::cin >> topo;
                    std::cout << "Digite o ponto da DIREITA (X final): ";
                    std::cin >> direita;
                    std::cout << "Digite o ponto da BASE (Y final): ";
                    std::cin >> base;

                    gama = direita - esquerda;
                    delta = base - topo;
                    processador.recorte(esquerda, topo);
                }
            }
        }

        std::string extensao = std::filesystem::path(entrada).extension().string();

        if (!camera && (entrada.find(".png") != std::string::npos || entrada.find(".jpg") != std::string::npos)) {
            cv::Mat resultado = processador.render(sequencia, {50.0, 50.0f, gama, delta});

            if (!resultado.empty()) {
                cv::imshow("Resultado Pro - Imagem", resultado);
                std::string saida_imagem = "output/resultado_pro" + extensao;
                cv::imwrite(saida_imagem, resultado);
                cv::waitKey(0);
            }
        }
        else {
            cv::VideoWriter gravador;
            std::string arquivo_saida;

            if (camera) {
                arquivo_saida = "output/resultado_pro_webcam.mp4";
            } else {
                arquivo_saida = "output/resultado_pro_video" + extensao;
            }

            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            bool pausado = false;

            while (true) {
                if (!pausado) {
                    cv::Mat resultado = processador.render(sequencia, {50.0, 50.0f, gama, delta});

                    if (resultado.empty()) {
                        if (camera) {
                            cv::waitKey(10);
                            continue;
                        } else {
                            break;
                        }
                    }

                    if (!gravador.isOpened()) {
                        gravador.open(arquivo_saida, codec, 30.0, resultado.size());
                    }

                    if (gravador.isOpened()) {
                        gravador.write(resultado);
                    }

                    cv::imshow("Resultado Pro - Stream", resultado);
                }

                int tecla = cv::waitKey(30);

                if (tecla == 'c') {
                    break;
                }
                if (tecla == 'p') {
                    pausado = !pausado;
                }
                if (tecla == 'r') {
                    if (!camera) {
                        processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                        pausado = false;
                    }
                }
            }

            if (gravador.isOpened()) {
                gravador.release();
            }
            cv::destroyWindow("Resultado Pro - Stream");
        }
    }

    else if (modo == "demo") {
        std::string entrada;
        std::cout << "Informe o caminho da mídia (ou digite 'webcam' para a câmera): ";
        std::cin >> entrada;

        Render processador;

        if (entrada == "webcam") {
            processador.camera(0);
            if (!processador.leitor.isOpened()) {
                return -1;
            }

            cv::Mat frame_aquecimento;
            for (int i = 0; i < 10; i++) {
                processador.leitor >> frame_aquecimento;
                if (!frame_aquecimento.empty()) break;
                cv::waitKey(30);
            }
        } else {
            processador.midia(entrada);
        }

        std::vector<int> sequencia;
        int escolha = -1;
        int gama = 1, delta = 1;

        Filtros();
        std::cout << "Digite os números dos filtros na sequência desejada (0 para finalizar): " << std::endl;
        while (escolha != 0) {
            std::cin >> escolha;
            if (escolha > 0 && escolha <= 10) {
                sequencia.push_back(escolha);

                if (escolha == 2) {
                    int esquerda, topo, direita, base;

                    std::cout << "Digite o ponto do ESQUERDA (X inicial): ";
                    std::cin >> esquerda;
                    std::cout << "Digite o ponto do TOPO (Y inicial): ";
                    std::cin >> topo;
                    std::cout << "Digite o ponto da DIREITA (X final): ";
                    std::cin >> direita;
                    std::cout << "Digite o ponto da BASE (Y final): ";
                    std::cin >> base;

                    gama = direita - esquerda;
                    delta = base - topo;
                    processador.recorte(esquerda, topo);
                }
            }
        }

        Parametros filtro = {50.0, 50.0f, gama, delta};
        processador.demo(sequencia, filtro);
    }
    else {
        Manual();
        return 1;
    }

    return 0;
}