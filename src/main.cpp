#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "../include/render.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;

std::string Renomear(const std::string& pasta, const std::string& prefixo, const std::string& extensao) {
    int contador = 1;
    std::string nome;
    do {
        nome = pasta + prefixo + "_" + std::to_string(contador) + extensao;
        contador++;
    } while (std::filesystem::exists(nome));
    return nome;
}

void Manual() {
    std::cout << "\n         RESGATE | RESTAURE | RELEMBRE         " << std::endl;
    std::cout << "\n Para iniciar, escolha uma das versões abaixo:\n" << std::endl;
    std::cout << "* Digite: free (para o Modo Iniciante)." << std::endl;
    std::cout << "* Digite: pro (para o Modo Profissional)." << std::endl;
    std::cout << "* Digite: demo (para o Modo de Demonstração)." << std::endl;
}

void Filtros() {
    std::cout << "\n             FILTROS DISPONÍVEIS            \n";
    std::cout << " [1] Girar              [2] Recortar\n";
    std::cout << " [3] Inserir Granulação [4] Ajustar Nitidez\n";
    std::cout << " [5] Desfocar           [6] Remover Falhas\n";
    std::cout << " [7] Reduzir Ruídos     [8] Ajustar Brilho\n";
    std::cout << " [9] Ajustar Contraste  [10] Alterar Cores\n";
    std::cout << " [11] Escala de Cinzas\n";
}

bool ValidarImagem(const std::string& extensao){
    std::string ext = extensao;
    for (auto& c : ext) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".webp");
}

int main(int argc, char* argv[]){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::string pastaDestino = "output/";
    
    if (!std::filesystem::exists(pastaDestino)) {
        if (!std::filesystem::create_directories(pastaDestino)) {
            std::cerr << "[ERRO] Não foi possível criar a pasta output/." << std::endl;
            return 1;
        }
    }

    std::string modo;

    if (argc < 2){
        Manual();
        std::cout << "\nEscolha a versão (free, pro, demo): ";
        std::cin >> modo;
    }
    else {
        modo = argv[1];
    }

    if (modo == "free") {
        Render processador;
        std::string entrada = " ";
        int opcao;
        int largura = 0;
        int altura = 0;

        while (true) {
            std::cout << "\nIniciando o Modo Iniciante..." << std::endl;
            std::cout << "[1] Abrir um arquivo de mídia (Imagem ou Vídeo)" << std::endl;
            std::cout << "[2] Usar a Webcam em tempo real" << std::endl;
            std::cout << "Escolha uma opção: ";
            std::cin >> opcao;

            if (opcao == 1) {
                while (true) {
                    std::cout << "Digite o caminho completo ou nome do arquivo: ";
                    std::cin >> entrada;
                    processador.midia(entrada);
                    if (!processador.leitor.isOpened()) {
                        std::cerr << "Erro! Não foi possível encontrar ou abrir este arquivo. Verifique o nome e tente novamente.\n" << std::endl;
                        continue;
                    }
                    cv::Mat x;
                    processador.leitor >> x;
                    if (!x.empty()) {
                        largura = x.cols;
                        altura = x.rows;
                        processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                    }
                    break;
                }
                break;
            }
            else if (opcao == 2) {
                processador.camera(0);
                std::cout << "Conectando e inicializando a sua Webcam..." << std::endl;

                if (!processador.leitor.isOpened()) {
                    std::cout << "Erro! Não conseguimos acessar a sua câmera. Verifique as permissões do dispositivo e tente novamente.\n" << std::endl;
                    continue;
                }
                cv::Mat x;
                for (int i = 0; i < 10; i++) {
                    processador.leitor >> x;
                    if (!x.empty()) {
                        largura = x.cols;
                        altura = x.rows;
                        break;
                    }
                    cv::waitKey(30);
                }
                break;
            }
            else {
                std::cout << "Opção inválida. Escolha entre 1 ou 2.\n" << std::endl;
            }
        }

        Filtros();
        int escolha;
        std::cout << "Selecione o número do filtro que deseja aplicar: ";
        std::cin >> escolha;

        Parametros filtro = {1.0, 0.0f, 0, 0};
        processador.recorte(0, 0);

        if (escolha == 1) {
            std::cout << "Digite o ângulo de rotação (ex: 90, 180, -45): ";
            std::cin >> filtro.alfa;
            std::cout << "Deseja espelhar a imagem? (1: Horizontal | 0: Vertical | -1: Ambos | 2: Não espelhar): ";
            std::cin >> filtro.gama;
        }
        else if (escolha == 2) {
            int esquerda, topo, direita, base;
            std::cout << "Quantos pixels quer cortar da ESQUERDA: ";
            std::cin >> esquerda;
            std::cout << "Quantos pixels quer cortar do TOPO: ";
            std::cin >> topo;
            std::cout << "Quantos pixels quer cortar da DIREITA: ";
            std::cin >> direita;
            std::cout << "Quantos pixels quer cortar da BASE: ";
            std::cin >> base;

            filtro.gama = largura - esquerda - direita;
            filtro.delta = altura - topo - base;
            processador.recorte(esquerda, topo);
        }
        else if (escolha == 3) {
            std::cout << "Ajuste a intensidade da Granulação (Escolha de 0 a 100): ";
            std::cin >> filtro.alfa;
        }
        else if (escolha == 4) {
            std::cout << "Ajuste a intensidade da Nitidez (Escolha de 0 a 100): ";
            std::cin >> filtro.alfa;
        }
        else if (escolha == 5) {
            std::cout << "Escolha o nível de desfoque (Recomendado usar números ímpares como 3, 5, 7 ou 15): ";
            std::cin >> filtro.gama;
        }
        else if (escolha == 6) {
            std::cout << "Defina o nível de sensibilidade da detecção: ";
            std::cin >> filtro.alfa;
        }
        else if (escolha == 7) {
            std::cout << "Digite o nível de redução de ruído (Recomendado: de 1.0 a 10.0): ";
            std::cin >> filtro.beta;
        }
        else if (escolha == 8) {
            std::cout << "Ajuste o brilho (0: Escuro | 50: Original | 100: Muito Claro): ";
            std::cin >> filtro.alfa;
        }
        else if (escolha == 9) {
            std::cout << "Ajuste o contraste (0: Menos contraste | 50: Original | 100: Mais contraste): ";
            std::cin >> filtro.alfa;
        }
        else if (escolha == 10) {
            std::cout << "Digite a intensidade da cor (Escolha de 0 a 100): ";
            std::cin >> filtro.alfa;
            std::cout << "Qual canal de cor deseja destacar? (1: Azul | 2: Verde | 3: Vermelho): ";
            std::cin >> filtro.gama;
        }
        else if (escolha == 11) {
            std::cout << "Ajuste o nível do efeito preto e branco (Escolha de 0 a 100): ";
            std::cin >> filtro.alfa;
        }

        std::string extensao = std::filesystem::path(entrada).extension().string();

        if (opcao == 1 && ValidarImagem(extensao)) {
            processador.leitor.release();
            processador.midia(entrada);

            cv::Mat resultado = processador.render(std::vector<int>{escolha}, std::vector<Parametros>{filtro});

            if (resultado.empty()){
                std::cerr << "Erro fatal! O filtro falhou e gerou uma imagem vazia." << std::endl;
                return -1;
            }

            std::string nomeJanela = "Finalizado!";
            cv::namedWindow(nomeJanela, cv::WINDOW_NORMAL);
            cv::imshow(nomeJanela, resultado);

            std::string destino = Renomear(pastaDestino, "resultado_processado", extensao);
            cv::imwrite(destino, resultado);
            std::cout << "Imagem salva com sucesso na pasta: " << destino << std::endl;

            cv::waitKey(0);
            cv::destroyWindow(nomeJanela);
        }
        else {
            cv::VideoWriter gravador;
            std::string video;

            if (opcao == 2) {
                video = Renomear(pastaDestino, "resultado_webcam", ".avi");
            } else {
                video = Renomear(pastaDestino, "resultado_processado", extensao);
            }

            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            bool pausado = false;

            while (true) {
                if (!pausado) {
                    cv::Mat resultado = processador.render(std::vector<int>{escolha}, std::vector<Parametros>{filtro});

                    if (resultado.empty()) {
                        if (opcao == 2) {
                            cv::waitKey(10);
                            continue;
                        } else {
                            break;
                        }
                    }

                    if (!gravador.isOpened()) {
                        gravador.open(video, codec, 30.0, resultado.size());
                    }

                    if (gravador.isOpened()) {
                        gravador.write(resultado);
                    }

                    cv::namedWindow("Resultado - Stream", cv::WINDOW_AUTOSIZE);
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
                std::cout << "\nVídeo salvo em: " << video << std::endl;
            }
            cv::destroyWindow("Resultado - Stream");
        }
    }

    else if (modo == "pro") {
        Render processador;
        std::string entrada;
        bool camera = false;
        int largura = 0;
        int altura = 0;

        while (true) {
            std::cout << "Informe o caminho da mídia (ou digite 'webcam' para usar a câmera): ";
            std::cin >> entrada;

            if (entrada == "webcam") {
                processador.camera(0);
                camera = true;
                if (!processador.leitor.isOpened()) {
                    std::cout << "Erro ao abrir a câmera. Tente novamente.\n" << std::endl;
                    continue;
                }

                cv::Mat x;
                for (int i = 0; i < 10; i++) {
                    processador.leitor >> x;
                    if (!x.empty()) {
                        largura = x.cols;
                        altura = x.rows;
                        break;
                    }
                    cv::waitKey(30);
                }
                break;
            } else {
                processador.midia(entrada);
                if (!processador.leitor.isOpened()) {
                    std::cerr << "Erro! Não foi possível abrir o arquivo informado. Tente novamente.\n" << std::endl;
                    continue;
                }
                cv::Mat x;
                processador.leitor >> x;
                if (!x.empty()) {
                    largura = x.cols;
                    altura = x.rows;
                }
                processador.leitor.release();
                processador.midia(entrada);
                break;
            }
        }

        std::vector<int> sequencia;
        std::vector<Parametros> filtros;
        int escolha = -1;

        Filtros();
        std::cout << "Digite a sequência dos filtros desejados em ordem (Digite 0 para encerrar a lista):" << std::endl;
        while (true) {
            std::cout << ">> Adicionar filtro: ";
            std::cin >> escolha;
            if (escolha == 0) break;

            if (escolha > 0 && escolha <= 11) {
                sequencia.push_back(escolha);
                Parametros filtro = {1.0, 0.0f, 0, 0};

                processador.recorte(0, 0);

                if (escolha == 1) {
                    std::cout << "   [Girar] -> Ângulo de rotação: ";
                    std::cin >> filtro.alfa;
                    std::cout << "   [Girar] -> Modo de espelhamento (1: Horiz, 0: Vert, -1: Ambos, 2: Nenhum): ";
                    std::cin >> filtro.gama;
                }
                else if (escolha == 2) {
                    int sizeofLeft, topo, direita, base;
                    std::cout << "   [Recortar] -> Margem da ESQUERDA (X inicial): ";
                    std::cin >> sizeofLeft;
                    std::cout << "   [Recortar] -> Margem do TOPO (Y inicial): ";
                    std::cin >> topo;
                    std::cout << "   [Recortar] -> Margem da DIREITA (X corte): ";
                    std::cin >> direita;
                    std::cout << "   [Recortar] -> Margem da BASE (Y corte): ";
                    std::cin >> base;

                    filtro.gama = largura - sizeofLeft - direita;
                    filtro.delta = altura - topo - base;
                    processador.recorte(sizeofLeft, topo);
                }
                else if (escolha == 3) { std::cout << "   [Granulação] -> Intensidade (0 a 100): "; std::cin >> filtro.alfa; }
                else if (escolha == 4) { std::cout << "   [Nitidez] -> Fator multiplicador (0 a 100): "; std::cin >> filtro.alfa; }
                else if (escolha == 5) { std::cout << "   [Desfocar] -> Dimensão do Kernel (Ímpar): "; std::cin >> filtro.gama; }
                else if (escolha == 6) { std::cout << "   [Remover Falhas] -> Sensibilidade de detecção: "; std::cin >> filtro.alfa; }
                else if (escolha == 7) { std::cout << "   [Reduzir Ruídos] -> Desvio padrão h (1.0 a 10.0): "; std::cin >> filtro.beta; }
                else if (escolha == 8) { std::cout << "   [Brilho] -> Incremento alfa (0 a 100): "; std::cin >> filtro.alfa; }
                else if (escolha == 9) { std::cout << "   [Contraste] -> Ganho alfa (0 a 100): "; std::cin >> filtro.alfa; }
                else if (escolha == 10) {
                    std::cout << "   [Alterar Cores] -> Escalar multiplicador: "; std::cin >> filtro.alfa;
                    std::cout << "   [Alterar Cores] -> ID do Canal (1: B | 2: G | 3: R): "; std::cin >> filtro.gama;
                }
                else if (escolha == 11) { std::cout << "   [Escala de Cinzas] -> Peso da conversão (0 a 100): "; std::cin >> filtro.alfa; }
                filtros.push_back(filtro);
            }
        }

        std::string extensao = std::filesystem::path(entrada).extension().string();
        for (auto& c : extensao) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        if (!camera && ValidarImagem(extensao)) {
            processador.leitor.release();
            processador.midia(entrada);

            cv::Mat resultado = processador.render(sequencia, filtros);

            if (!resultado.empty()) {
                std::string nomeJanela = "Resultado Pro - Imagem";
                cv::namedWindow(nomeJanela, cv::WINDOW_NORMAL);
                cv::imshow(nomeJanela, resultado);

                std::string destino = Renomear(pastaDestino, "resultado_pro", extensao);

                if (cv::imwrite(destino, resultado)) {
                    std::cout << "\n[SUCESSO] Imagem salva em: " << destino << std::endl;
                } else {
                    std::cerr << "\n[ERRO] Falha ao gravar o arquivo na pasta output/." << std::endl;
                }

                cv::waitKey(0);
                cv::destroyWindow(nomeJanela);
            } else {
                std::cerr << "\n[ERRO] O processador gerou uma imagem vazia." << std::endl;
            }
        }
        else {
            cv::VideoWriter gravador;
            std::string video = camera ? Renomear(pastaDestino, "resultado_pro_webcam", ".avi") : Renomear(pastaDestino, "resultado_pro_video", extensao);
            int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            bool pausado = false;

            while (true) {
                if (!pausado) {
                    cv::Mat resultado = processador.render(sequencia, filtros);

                    if (resultado.empty()) {
                        if (camera) {
                            cv::waitKey(10);
                            continue;
                        } else {
                            break;
                        }
                    }

                    if (!gravador.isOpened()) {
                        gravador.open(video, codec, 30.0, resultado.size());
                    }

                    if (gravador.isOpened()) {
                        gravador.write(resultado);
                    }

                    cv::namedWindow("Resultado Pro - Stream", cv::WINDOW_AUTOSIZE);
                    cv::imshow("Resultado Pro - Stream", resultado);
                }

                int tecla = cv::waitKey(30);
                if (tecla == 'c' || tecla == 27) {
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
        Render processador;
        std::string entrada;
        bool camera = false;
        int largura = 0;
        int altura = 0;

        while (true) {
            std::cout << "Informe o caminho da mídia (ou digite 'webcam' para a câmera): ";
            std::cin >> entrada;

            if (entrada == "webcam") {
                processador.camera(0);
                camera = true;
                if (!processador.leitor.isOpened()) {
                    std::cout << "Erro ao abrir a câmera. Tente novamente.\n" << std::endl;
                    continue;
                }

                cv::Mat x;
                for (int i = 0; i < 10; i++) {
                    processador.leitor >> x;
                    if (!x.empty()) {
                        largura = x.cols;
                        altura = x.rows;
                        break;
                    }
                    cv::waitKey(30);
                }
                break;
            } else {
                processador.midia(entrada);
                if (!processador.leitor.isOpened()) {
                    std::cerr << "Erro! Não foi possível abrir o arquivo informado. Tente novamente.\n" << std::endl;
                    continue;
                }
                cv::Mat x;
                processador.leitor >> x;
                if (!x.empty()) {
                    largura = x.cols;
                    altura = x.rows;
                    processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                }
                break;
            }
        }

        while (true) {
            std::vector<int> sequencia;
            std::vector<Parametros> filtros;
            int escolha = -1;

            Filtros();
            std::cout << "Digite a sequência dos filtros para a demonstração (Digite 0 para iniciar | Digite -1 para SAIR do programa):" << std::endl;

            bool sair = false;
            while (true) {
                std::cout << ">> Adicionar ao Demo: ";
                std::cin >> escolha;
                if (escolha == 0) break;
                if (escolha == -1) {
                    sair = true;
                    break;
                }

                if (escolha > 0 && escolha <= 11) {
                    sequencia.push_back(escolha);
                    Parametros filtro = {1.0, 0.0f, 0, 0};

                    processador.recorte(0, 0);

                    if (escolha == 1) {
                        std::cout << "   [Demo: Girar] -> Ângulo: ";
                        std::cin >> filtro.alfa;
                        std::cout << "   [Demo: Girar] -> Espelhamento: ";
                        std::cin >> filtro.gama;
                    }
                    else if (escolha == 2) {
                        int sizeofLeft, topo, direita, base;
                        std::cout << "   [Demo: Recortar] -> Borda Esquerda: ";
                        std::cin >> sizeofLeft;
                        std::cout << "   [Demo: Recortar] -> Borda Topo: ";
                        std::cin >> topo;
                        std::cout << "   [Demo: Recortar] -> Borda Direita: ";
                        std::cin >> direita;
                        std::cout << "   [Demo: Recortar] -> Borda Base: ";
                        std::cin >> base;

                        filtro.gama = largura - sizeofLeft - direita;
                        filtro.delta = altura - topo - base;
                        processador.recorte(sizeofLeft, topo);
                    }
                    else if (escolha == 3) { std::cout << "   [Demo: Granulação] -> Intensidade: "; std::cin >> filtro.alfa; }
                    else if (escolha == 4) { std::cout << "   [Demo: Nitidez] -> Intensidade: "; std::cin >> filtro.alfa; }
                    else if (escolha == 5) { std::cout << "   [Demo: Desfocar] -> Kernel: "; std::cin >> filtro.gama; }
                    else if (escolha == 6) { std::cout << "   [Demo: Remover Falhas] -> Sensibilidade: "; std::cin >> filtro.alfa; }
                    else if (escolha == 7) { std::cout << "   [Demo: Reduzir Ruídos] -> Fator: "; std::cin >> filtro.beta; }
                    else if (escolha == 8) { std::cout << "   [Demo: Brilho] -> Nível: "; std::cin >> filtro.alfa; }
                    else if (escolha == 9) { std::cout << "   [Demo: Contraste] -> Nível: "; std::cin >> filtro.alfa; }
                    else if (escolha == 10) {
                        std::cout << "   [Demo: Cores] -> Multiplicador: "; std::cin >> filtro.alfa;
                        std::cout << "   [Demo: Cores] -> Canal (1/2/3): "; std::cin >> filtro.gama;
                    }
                    else if (escolha == 11) { std::cout << "   [Demo: Cinzas] -> Mesclagem: "; std::cin >> filtro.alfa; }

                    filtros.push_back(filtro);
                }
            }

            if (sair) {
                break;
            }

            if (sequencia.empty()) {
                std::cout << "Nenhum filtro selecionado. Tente novamente.\n";
                continue;
            }

            if (!camera) {
                processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
            }

            processador.comparar(sequencia, filtros);
        }
    }
    else {
        Manual();
        return 1;
    }

    return 0;
}