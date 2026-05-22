#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/render.h"

int main(int argc, char** argv){
    std::string entrada;
    int opcao;

std::cout << "Resgate. Restaure. Relembre" << std::endl;
std::cout << "[1] Abrir um arquivo de mídia" << std::endl;
std::Cout << "[2] Sincronizar a Webcam" << std::endl;
std::cout << "Escolha uma opção, digite 1 ou 2" << std::endl;

std::cout << "Opção: ";
std::cin >> opcao;

if opcao == 1 {
    std::cout << "Informe o caminho do arquivo: ";
    std::cin >> entrada;
}
    else if opcao == 2 {
    entrada = "0";
    std::cout << "Acessando a Webcam..." << std::endl;
}
    else {
    std::cout << "Opção inválida" << std::endl;

return -1;
    }

Render processador(entrada);

std::cout << "Processador de Mídia" << std::endl;
std::cout << "Escolha o filtro desejado: " << std::endl;
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
std::cin >> escolha;

double alfa = 50.0, beta = 50.0;

processador.render(cv::Mat(), escolha, alfa, beta, gama, delta);

return o;
}
