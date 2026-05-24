#include <iostream>
#include <string>
#include "../include/render.h"
#include <opencv2/opencv.hpp>

using namespace std;

void Manual() {
    std::cout << "RESGATE. RESTAURE. RELEMBRE" << std::endl;
    std::cout << "Uso correto: ./RELEMBRE [MODO]" << std::endl;
    std::cout << "Modos disponíveis:" << std::endl;
    std::cout << "free ->  Módulo para iniciantes." << std::endl;
    std::cout << "pro  -> Módulo para profissionais." << std::endl;
    std::cout << "demo -> Módulo para demonstração." << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        Manual();
        return 0;
    }

    std::string modo = argv[1];

    if (modo == "free") {
        std::string entrada;
        int opcao;

        std::cout << "Iniciando o módulo para iniciantes..." << std::endl;
        std::cout << "[1] Abrir um arquivo de mídia" << std::endl;
        std::cout << "[2] Sincronizar a Webcam" << std::endl;
        std::cout << "Opção: ";
        std::cin >> opcao;

        if (opcao == 1) {
            std::cout << "Informe o caminho do arquivo: ";
            std::cin >> entrada;
        } else if (opcao == 2) {
            entrada = "0";
            std::cout << "Acessando a Webcam..." << std::endl;
        } else {
            std::cout << "Opção inválida." << std::endl;
            return -1;
        }

        Render processador(entrada);

        std::cout << "\n ESCOLHA O FILTRO " << std::endl;
        std::cout << "1. Girar e Inverter" << std::endl;
        std::cout << "2. Recortar" << std::endl;
        std::cout << "3. Desfocar" << std::endl;
        std::cout << "4. Reduzir Ruídos" << std::endl;
        std::cout << "5. Remoção de Falhas" << std::endl;
        std::cout << "6. Ajustar Contraste" << std::endl;
        std::cout << "7. Calibrar Iluminação" << std::endl;
        std::cout << "8. Correção de Cores" << std::endl;
        std::cout << "9. Realçar Nitidez" << std::endl;
        std::cout << "10. Tons de Cinza" << std::endl;

        int escolha;
        std::cout << "Digite o número do filtro: ";
        std::cin >> escolha;

        double alfa = 50.0;
        float beta = 50.0f;
        int gama = 1;
        int delta = 1;

        if (escolha == 2 && opcao == 1) {
            static int esquerda = -1, topo = -1;
            cv::Mat temp = cv::imread(entrada);

            if (!temp.empty()) {
                if (esquerda == -1 && topo == -1) {
                    std::cout << "\n CORTE DE IMAGEM \n";
                    std::cout << "As dimensões atuais são: " << temp.cols << "x" << temp.rows << "\n";
                    std::cout << "Digite o ponto horizontal inicial (da esquerda para a direita): ";
                    std::cin >> esquerda;
                    std::cout << "Digite o ponto vertical inicial (de cima para baixo): ";
                    std::cin >> topo;
                    gama = esquerda;
                    delta = topo;
                }
            }
        }

        cv::Mat resultado = processador.render(escolha, {alfa, beta, gama, delta});

        cv::imshow("Resultado", resultado);
        cv::waitKey(0);

        return 0;
    }

    else if (modo == "pro"){
        std::string entrada;
    std::cout << "Informe o caminho da midia: ";
    std::cin >> entrada;

    Render processador(entrada);
    std::vector<int> sequencia;
    int escolha = -1;

    std::cout << "Digite os números dos filtros (0 para finalizar): " << std::endl;
    while (escolha != 0) {
        std::cin >> escolha;
        if (escolha > 0 && escolha <= 10) {
            sequencia.push_back(escolha);
        }
    }

    double alfa = 50.0;
    float beta = 50.0f;
    int gama = 1;
    int delta = 1;

    cv::Mat resultado = processador.render(sequencia, {alfa, beta, gama, delta});

    cv::imshow("Resultado Pro", resultado);
    cv::waitKey(0);
}

    else if (modo == "demo") {
        std::string entrada;
        std::cout << "Informe o caminho da mídia para o módulo de demonstração: ";
        std::cin >> entrada;

        Render processador(entrada);
        int escolha;

        std::cout << "Escolha o filtro para exibição em tempo real (1-10): ";
        std::cin >> escolha;

        Parametros p = {50.0, 50.0f, 1, 1};
        processador.demo(escolha, p);

        return 0;
    }

    else {
        std::cout << "Erro: Modo '" << modo << "' desconhecido." << std::endl;
        Manual();
        return 1;
    }

    return 0;
}