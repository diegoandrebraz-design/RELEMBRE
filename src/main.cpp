#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "../include/render.h"
#include <opencv2/opencv.hpp>

using namespace std;

void Manual() {
    std::cout << "         RESGATE | RESTAURE | RELEMBRE         " << std::endl;
    std::cout << "\n Para iniciar, escolha um das versões abaixo:\n" << std::endl;
    std::cout << "* Digite: free (para o Módulo Iniciante)." << std::endl;
    std::cout << "* Digite: pro (para o Módulo Profissional)." << std::endl;
    std::cout << "* Digite: demo (para o Módulo de Demonstração)." << std::endl;
}

int main(int argc, char* argv[]) {
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
        std::string entrada = "";
        int opcao;

        std::cout << "Iniciando o módulo para iniciantes..." << std::endl;
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
                std::cout << "Erro: Câmera não abriu." << std::endl;
                return -1;
            }
        }
        else {
            std::cout << "Opção inválida." << std::endl;
            return -1;
        }

        std::cout << "\n ESCOLHA O FILTRO " << std::endl;
        int escolha;
        std::cout << "Digite o número do filtro: ";
        std::cin >> escolha;

        int gama = 1, delta = 1;

        if (escolha == 2 && opcao == 1) {
            static int esquerda = -1, topo = -1;

            cv::Mat temp = cv::imread(entrada);

            if (!temp.empty()) {
                if (esquerda == -1 && topo == -1) {
                    std::cout << "\n CORTE DE IMAGEM \n";
                    std::cout << "As dimensões atuais são: " << temp.cols << "x" << temp.rows << "\n";
                    std::cout << "Digite o ponto horizontal inicial: ";
                    std::cin >> esquerda;
                    std::cout << "Digite o ponto vertical inicial: ";
                    std::cin >> topo;
                    gama = esquerda;
                    delta = topo;
                }
            }
        }

        if (opcao == 1 && (entrada.find(".png") != std::string::npos || entrada.find(".jpg") != std::string::npos)) {
            cv::Mat resultado = processador.render(escolha, {50.0, 50.0f, gama, delta});

            if (resultado.empty()){
                std::cerr << "Erro: O filtro retornou uma imagem vazia." << std::endl;
                return -1;
            }

            cv::imshow("Resultado - Imagem", resultado);

            std::string saida_imagem = "resultado_processado.png";
            cv::imwrite(saida_imagem, resultado);
            std::cout << "Imagem salva com sucesso como: " << saida_imagem << std::endl;

            cv::waitKey(0);
        }

        else {
            cv::VideoWriter gravador;
            std::string arquivo_saida = "resultado_webcam.mp4";
            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            double fps = 30.0;

            std::cout << "\nGravando! Pressione a tecla 'ESC' na janela do vídeo para encerrar e salvar.\n";

            while (true) {
                cv::Mat resultado = processador.render(escolha, {50.0, 50.0f, gama, delta});

                if (resultado.empty()) {

                    continue;
                }

                if (!gravador.isOpened()) {
                    gravador.open(arquivo_saida, codec, fps, resultado.size());
                    if (!gravador.isOpened()) {
                        std::cerr << "Erro ao iniciar o gravador de vídeo." << std::endl;
                    }
                }

                if (gravador.isOpened()) {
                    gravador.write(resultado);
                }

                cv::imshow("Resultado - Stream", resultado);

                char tecla = (char)cv::waitKey(30);
                if (tecla == 27) {
                    break;
                }
            }

            if (gravador.isOpened()) {
                gravador.release();
                std::cout << "\nVídeo salvo com sucesso em: " << arquivo_saida << std::endl;
            }
        }
    }

    else if (modo == "pro") {
        std::string entrada;
        std::cout << "Informe o caminho da mídia (ou digite 'webcam' para usar a câmera): ";
        std::cin >> entrada;

        Render processador;
        bool eh_webcam = false;

        if (entrada == "webcam") {
            processador.camera(0);
            eh_webcam = true;
            if (!processador.leitor.isOpened()) {
                std::cout << "Erro: Câmera não abriu." << std::endl;
                return -1;
            }
        } else {
            processador.midia(entrada);
        }

        std::vector<int> sequencia;
        int escolha = -1;

        std::cout << "Digite os números dos filtros (0 para finalizar): " << std::endl;
        while (escolha != 0) {
            std::cin >> escolha;
            if (escolha > 0 && escolha <= 10) {
                sequencia.push_back(escolha);
            }
        }

        if (!eh_webcam && (entrada.find(".png") != std::string::npos || entrada.find(".jpg") != std::string::npos)) {
            cv::Mat resultado = processador.render(sequencia, {50.0, 50.0f, 1, 1});

            if (!resultado.empty()) {
                cv::imshow("Resultado Pro - Imagem", resultado);
                cv::imwrite("resultado_pro.png", resultado);
                std::cout << "Imagem Pro salva com sucesso como: resultado_pro.png" << std::endl;
                cv::waitKey(0);
            } else {
                std::cerr << "Erro ao renderizar a imagem no modo Pro." << std::endl;
            }
        }

        else {
            cv::VideoWriter gravador;
            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            std::cout << "\nProcessando vídeo/webcam no modo Pro. Pressione 'ESC' na janela para sair.\n";

            while (true) {
                cv::Mat resultado = processador.render(sequencia, {50.0, 50.0f, 1, 1});
                if (resultado.empty()) continue;

                if (!gravador.isOpened()) {
                    gravador.open("resultado_pro_video.mp4", codec, 30.0, resultado.size());
                }

                if (gravador.isOpened()) {
                    gravador.write(resultado);
                }

                cv::imshow("Resultado Pro - Stream", resultado);
                if ((char)cv::waitKey(30) == 27) break; // ESC sai do loop
            }

            if (gravador.isOpened()) {
                gravador.release();
                std::cout << "\nVídeo Pro salvo com sucesso em: resultado_pro_video.mp4" << std::endl;
            }
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
                std::cout << "Erro: Câmera não abriu." << std::endl;
                return -1;
            }
        } else {
            processador.midia(entrada);
        }

        int escolha;
        std::cout << "Escolha o filtro (1-10): ";
        std::cin >> escolha;

        Parametros filtro = {50.0, 50.0f, 1, 1};
        processador.demo(escolha, filtro);
    }

    else {
        std::cout << "Erro: Modo '" << modo << "' desconhecido." << std::endl;
        Manual();
        return 1;
    }

    return 0;
}