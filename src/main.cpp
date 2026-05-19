#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/render.h"

int main()
{
    Render RELEMBRE;
    std::string arquivo;
    std::cout<<"Agora, me diga o endereço seu arquivo está salvo";
    std::getline(std::cin,arquivo);

    int escolha = 0;
    std::cout<<"\nSelecione a opção desejada:";
    std::cout<<"1 - Alinhar\n";
    std::cout<<"2 - Recortar\n"; \\gaussian
    std::cout<<"3 - Aplicar Desfoque\n"; \\balanço de brancos
    std::cout<<"4 - Reduzir os Ruídos\n"; \\clahe
    std::cout<<"5 - Disfarçar as Falhas\n"; \\ filtro bilateral
    std::cout<<"6 - Ajustar o Contraste\n"; \\ inpainting
    std::cout<<"7 - Balancear a Iluminação\n";
    std::cout<<"8 - Correção de Cores\n";
    std::cout<<"9 - Ganho de Nitidez\n";
    std::cout<<"10 - Tons de Cinza\n";



    if (!arquivo.empty()) && (arquivo.front()=='"')
}
{Onde encontro o arquivo que deseja editar? Digite o caminho de uma imagem ou video

    return 0;
}