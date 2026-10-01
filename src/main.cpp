#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <windows.h>
#include <commdlg.h>
#include <opencv2/opencv.hpp>
#include "../include/render.h"

#pragma comment(lib, "Comdlg32.lib")

namespace fs = std::filesystem;

struct DadosRecorte {
    int esquerda = 0;
    int direita = 0;
    int topo = 0;
    int base = 0;
    bool ativo = false;
};

std::string Renomear(
    const std::string& pasta,
    const std::string& prefixo,
    const std::string& extensao
) {
    int contador = 1;
    std::string nome;

    do {
        nome =
            pasta +
            prefixo +
            "_" +
            std::to_string(contador) +
            extensao;

        ++contador;
    } while (fs::exists(nome));

    return nome;
}

void Manual() {
    std::cout << "\n RESGATE | RESTAURE | RELEMBRE \n";
    std::cout << "\nDigite uma das versões:\n";
    std::cout << "* free - Modo Iniciante\n";
    std::cout << "* pro  - Modo Profissional\n";
    std::cout << "* demo - Modo de Demonstração\n";
}

void Filtros() {
    std::cout << "\n FILTROS DISPONÍVEIS\n";
    std::cout << " [1] Girar                  [2] Recortar\n";
    std::cout << " [3] Inserir Granulação    [4] Ajustar Nitidez\n";
    std::cout << " [5] Desfocar              [6] Remover Falhas\n";
    std::cout << " [7] Reduzir Ruídos        [8] Ajustar Brilho\n";
    std::cout << " [9] Ajustar Contraste    [10] Alterar Cores\n";
    std::cout << "[11] Escala de Cinzas\n";
}

std::string NomeFiltro(int escolha) {
    switch (escolha) {
    case 1: return "Girar";
    case 2: return "Recortar";
    case 3: return "Inserir Granulação";
    case 4: return "Ajustar Nitidez";
    case 5: return "Desfocar";
    case 6: return "Remover Falhas";
    case 7: return "Reduzir Ruídos";
    case 8: return "Ajustar Brilho";
    case 9: return "Ajustar Contraste";
    case 10: return "Alterar Cores";
    case 11: return "Escala de Cinzas";
    default: return "Desconhecido";
    }
}

bool ValidarImagem(const std::string& extensao) {
    std::string ext = extensao;

    for (char& c : ext) {
        c = static_cast<char>(
            std::tolower(static_cast<unsigned char>(c))
        );
    }

    return
        ext == ".png" ||
        ext == ".jpg" ||
        ext == ".jpeg" ||
        ext == ".bmp" ||
        ext == ".webp";
}

std::vector<std::string> SelecionarArquivos(bool multiplos) {
    char buffer[65536] = {};
    OPENFILENAMEA dialogo{};

    dialogo.lStructSize = sizeof(dialogo);
    dialogo.lpstrFile = buffer;
    dialogo.nMaxFile = sizeof(buffer);

    dialogo.lpstrFilter =
        "Mídias suportadas\0"
        "*.png;*.jpg;*.jpeg;*.bmp;*.webp;*.mp4;*.avi;*.mov;*.mkv;*.wmv\0"
        "Todos os arquivos\0"
        "*.*\0";

    dialogo.nFilterIndex = 1;

    dialogo.Flags =
        OFN_PATHMUSTEXIST |
        OFN_FILEMUSTEXIST |
        OFN_EXPLORER;

    if (multiplos) {
        dialogo.Flags |= OFN_ALLOWMULTISELECT;
    }

    if (!GetOpenFileNameA(&dialogo)) {
        return {};
    }

    std::vector<std::string> arquivos;
    char* cursor = buffer;

    if (
        !multiplos ||
        cursor[std::strlen(cursor) + 1] == '\0'
    ) {
        arquivos.emplace_back(cursor);
        return arquivos;
    }

    const std::string pasta = cursor;
    cursor += pasta.size() + 1;

    while (*cursor) {
        const std::string nome = cursor;
        arquivos.push_back(pasta + "\\" + nome);
        cursor += nome.size() + 1;
    }

    return arquivos;
}

bool LerParametros(
    Render& processador,
    int escolha,
    Parametros& filtro,
    int largura,
    int altura,
    DadosRecorte* dadosRecorte = nullptr
) {
    filtro = {1.0, 0.0f, 0, 0};

    processador.recorte(0, 0);

    if (dadosRecorte) {
        *dadosRecorte = {};
    }

    switch (escolha) {
    case 1:
        std::cout << " [Girar] Ângulo: ";
        std::cin >> filtro.alfa;

        std::cout
            << " [Girar] Espelhamento "
            << "(1 Horizontal | 0 Vertical | -1 Ambos | 2 Nenhum): ";

        std::cin >> filtro.gama;
        break;

    case 2: {
        DadosRecorte recorte;

        std::cout << " [Recortar] Esquerda: ";
        std::cin >> recorte.esquerda;

        std::cout << " [Recortar] Direita: ";
        std::cin >> recorte.direita;

        std::cout << " [Recortar] Topo: ";
        std::cin >> recorte.topo;

        std::cout << " [Recortar] Base: ";
        std::cin >> recorte.base;

        if (
            recorte.esquerda < 0 ||
            recorte.direita < 0 ||
            recorte.topo < 0 ||
            recorte.base < 0
        ) {
            std::cout <<
                "[ERRO] Os quatro valores do recorte devem ser >= 0.\n";
            return false;
        }

        filtro.gama =
            largura -
            recorte.esquerda -
            recorte.direita;

        filtro.delta =
            altura -
            recorte.topo -
            recorte.base;

        if (
            filtro.gama <= 1 ||
            filtro.delta <= 1
        ) {
            std::cout
                << "[ERRO] Área de recorte inválida para "
                << largura << "x" << altura << ".\n";
            return false;
        }

        processador.recorte(
            recorte.esquerda,
            recorte.topo
        );

        if (dadosRecorte) {
            *dadosRecorte = recorte;
            dadosRecorte->ativo = true;
        }

        break;
    }

    case 3:
        std::cout
            << " [Granulação] Intensidade (0 a 100): ";
        std::cin >> filtro.alfa;
        break;

    case 4:
        std::cout
            << " [Nitidez] Intensidade (0 a 100): ";
        std::cin >> filtro.alfa;
        break;

    case 5:
        std::cout << " [Desfocar] Kernel: ";
        std::cin >> filtro.gama;
        break;

    case 6:
        std::cout
            << " [Remover Falhas] Sensibilidade: ";
        std::cin >> filtro.alfa;
        break;

    case 7:
        std::cout
            << " [Reduzir Ruídos] Fator (1 a 10): ";
        std::cin >> filtro.beta;
        break;

    case 8:
        std::cout << " [Brilho] Intensidade: ";
        std::cin >> filtro.alfa;
        break;

    case 9:
        std::cout << " [Contraste] Ganho: ";
        std::cin >> filtro.alfa;
        break;

    case 10:
        std::cout << " [Cores] Intensidade: ";
        std::cin >> filtro.alfa;

        std::cout
            << " [Cores] Canal "
            << "(1 Azul | 2 Verde | 3 Vermelho): ";

        std::cin >> filtro.gama;
        break;

    case 11:
        std::cout << " [Cinzas] Peso: ";
        std::cin >> filtro.alfa;
        break;

    default:
        std::cout << "Opção inválida.\n";
        return false;
    }

    return true;
}

bool MontarSequencia(
    Render& processador,
    int largura,
    int altura,
    std::vector<int>& sequencia,
    std::vector<Parametros>& filtros,
    bool permitirSair
) {
    sequencia.clear();
    filtros.clear();

    Filtros();

    std::cout << "Digite 0 para iniciar";

    if (permitirSair) {
        std::cout << " ou -1 para sair";
    }

    std::cout << ".\n";

    while (true) {
        int escolha;

        std::cout << ">> Adicionar filtro: ";
        std::cin >> escolha;

        if (escolha == 0) {
            break;
        }

        if (permitirSair && escolha == -1) {
            return false;
        }

        if (escolha < 1 || escolha > 11) {
            std::cout << "Opção inválida.\n";
            continue;
        }

        Parametros filtro;

        if (
            !LerParametros(
                processador,
                escolha,
                filtro,
                largura,
                altura
            )
        ) {
            continue;
        }

        sequencia.push_back(escolha);
        filtros.push_back(filtro);
    }

    return !sequencia.empty();
}

bool AbrirArquivo(
    Render& processador,
    const std::string& arquivo,
    int& largura,
    int& altura
) {
    processador.midia(arquivo);

    if (!processador.leitor.isOpened()) {
        return false;
    }

    cv::Mat primeiro;
    processador.leitor >> primeiro;

    if (primeiro.empty()) {
        processador.leitor.release();
        return false;
    }

    largura = primeiro.cols;
    altura = primeiro.rows;

    processador.leitor.set(
        cv::CAP_PROP_POS_FRAMES,
        0
    );

    return true;
}

bool AbrirWebcam(
    Render& processador,
    int& largura,
    int& altura
) {
    processador.camera(0);

    if (!processador.leitor.isOpened()) {
        return false;
    }

    cv::Mat quadro;

    for (int i = 0; i < 10; ++i) {
        processador.leitor >> quadro;

        if (!quadro.empty()) {
            largura = quadro.cols;
            altura = quadro.rows;
            return true;
        }

        cv::waitKey(30);
    }

    return false;
}

bool ProcessarArquivoFree(
    Render& processador,
    const std::string& arquivo,
    int escolha,
    Parametros filtro,
    const DadosRecorte& dadosRecorte,
    const std::string& pastaDestino
) {
    int largura = 0;
    int altura = 0;

    if (
        !AbrirArquivo(
            processador,
            arquivo,
            largura,
            altura
        )
    ) {
        std::cerr
            << "[ERRO] Não foi possível abrir: "
            << arquivo << "\n";

        return false;
    }

    if (escolha == 2) {
        filtro.gama =
            largura -
            dadosRecorte.esquerda -
            dadosRecorte.direita;

        filtro.delta =
            altura -
            dadosRecorte.topo -
            dadosRecorte.base;

        if (
            filtro.gama <= 1 ||
            filtro.delta <= 1
        ) {
            std::cerr
                << "[ERRO] O recorte não cabe na mídia: "
                << largura << "x" << altura << ".\n";

            return false;
        }

        processador.recorte(
            dadosRecorte.esquerda,
            dadosRecorte.topo
        );
    }

    const std::string extensao =
        fs::path(arquivo).extension().string();

    if (!ValidarImagem(extensao)) {
        std::cerr
            << "[ERRO] No modo Free, selecione imagens "
            << "para processamento em arquivo.\n";

        return false;
    }

    cv::Mat resultado =
        processador.render(
            escolha,
            filtro
        );

    if (resultado.empty()) {
        std::cerr
            << "[ERRO] O processamento gerou uma imagem vazia.\n";

        return false;
    }

    const std::string destino =
        Renomear(
            pastaDestino,
            "resultado_free_" + NomeFiltro(escolha),
            extensao
        );

    if (!cv::imwrite(destino, resultado)) {
        std::cerr
            << "[ERRO] Não foi possível salvar: "
            << destino << "\n";

        return false;
    }

    std::cout
        << "[SUCESSO] Imagem salva: "
        << destino << "\n";

    return true;
}

bool ProcessarArquivo(
    Render& processador,
    const std::string& arquivo,
    const std::vector<int>& escolhas,
    const std::vector<Parametros>& filtros,
    const std::string& pastaDestino,
    const std::string& prefixo
) {
    int largura = 0;
    int altura = 0;

    if (
        !AbrirArquivo(
            processador,
            arquivo,
            largura,
            altura
        )
    ) {
        std::cerr
            << "[ERRO] Não foi possível abrir: "
            << arquivo << "\n";

        return false;
    }

    const std::string extensao =
        fs::path(arquivo).extension().string();

    if (ValidarImagem(extensao)) {
        cv::Mat resultado =
            processador.render(
                escolhas,
                filtros
            );

        if (resultado.empty()) {
            std::cerr
                << "[ERRO] O processamento gerou "
                << "uma imagem vazia.\n";

            return false;
        }

        const std::string destino =
            Renomear(
                pastaDestino,
                prefixo,
                extensao
            );

        if (!cv::imwrite(destino, resultado)) {
            std::cerr
                << "[ERRO] Não foi possível salvar: "
                << destino << "\n";

            return false;
        }

        std::cout
            << "[SUCESSO] Imagem salva: "
            << destino << "\n";

        return true;
    }

    const std::string destino =
        Renomear(
            pastaDestino,
            prefixo,
            ".mp4"
        );

    cv::VideoWriter gravador;

    const int codec =
        cv::VideoWriter::fourcc(
            'm',
            'p',
            '4',
            'v'
        );

    bool pausado = false;

    cv::namedWindow(
        "Resultado - Stream",
        cv::WINDOW_NORMAL
    );

    while (true) {
        if (!pausado) {
            cv::Mat resultado =
                processador.render(
                    escolhas,
                    filtros
                );

            if (resultado.empty()) {
                break;
            }

            if (!gravador.isOpened()) {
                if (
                    !gravador.open(
                        destino,
                        codec,
                        30.0,
                        resultado.size()
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível criar "
                        << "o vídeo de saída.\n";

                    cv::destroyWindow(
                        "Resultado - Stream"
                    );

                    return false;
                }
            }

            gravador.write(resultado);

            cv::imshow(
                "Resultado - Stream",
                resultado
            );
        }

        const int tecla =
            cv::waitKey(30) & 0xFF;

        if (
            tecla == 'c' ||
            tecla == 'C' ||
            tecla == 27
        ) {
            break;
        }

        if (
            tecla == 'p' ||
            tecla == 'P'
        ) {
            pausado = !pausado;
        }
        else if (
            tecla == 'r' ||
            tecla == 'R'
        ) {
            processador.leitor.set(
                cv::CAP_PROP_POS_FRAMES,
                0
            );

            pausado = false;
        }
    }

    if (gravador.isOpened()) {
        gravador.release();

        std::cout
            << "[SUCESSO] Vídeo salvo: "
            << destino << "\n";
    }

    cv::destroyWindow(
        "Resultado - Stream"
    );

    return true;
}

bool ProcessarWebcam(
    Render& processador,
    const std::vector<int>& escolhas,
    const std::vector<Parametros>& filtros,
    const std::string& pastaDestino,
    const std::string& prefixo
) {
    const std::string destino =
        Renomear(
            pastaDestino,
            prefixo,
            ".mp4"
        );

    cv::VideoWriter gravador;

    const int codec =
        cv::VideoWriter::fourcc(
            'm',
            'p',
            '4',
            'v'
        );

    bool pausado = false;

    cv::namedWindow(
        "Resultado - Webcam",
        cv::WINDOW_NORMAL
    );

    while (true) {
        if (!pausado) {
            cv::Mat resultado =
                processador.render(
                    escolhas,
                    filtros
                );

            if (resultado.empty()) {
                cv::waitKey(10);
                continue;
            }

            if (!gravador.isOpened()) {
                if (
                    !gravador.open(
                        destino,
                        codec,
                        30.0,
                        resultado.size()
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível criar "
                        << "o vídeo da webcam.\n";

                    cv::destroyWindow(
                        "Resultado - Webcam"
                    );

                    return false;
                }
            }

            gravador.write(resultado);

            cv::imshow(
                "Resultado - Webcam",
                resultado
            );
        }

        const int tecla =
            cv::waitKey(30) & 0xFF;

        if (
            tecla == 'c' ||
            tecla == 'C' ||
            tecla == 27
        ) {
            break;
        }

        if (
            tecla == 'p' ||
            tecla == 'P'
        ) {
            pausado = !pausado;
        }
        else if (
            tecla == 'r' ||
            tecla == 'R'
        ) {
            processador.camera(0);
            pausado = false;
        }
    }

    if (gravador.isOpened()) {
        gravador.release();

        std::cout
            << "[SUCESSO] Vídeo da webcam salvo: "
            << destino << "\n";
    }

    cv::destroyWindow(
        "Resultado - Webcam"
    );

    return true;
}

void RelatorioFiltros(
    const std::string& modo,
    const std::string& arquivo,
    const std::vector<int>& escolhas,
    const std::vector<Parametros>& filtros,
    const std::string& pastaDestino
) {
    if (escolhas.size() != filtros.size()) {
        std::cerr
            << "[ERRO] Não foi possível gerar "
            << "o relatório: dados incompatíveis.\n";

        return;
    }

    const std::string destino =
        Renomear(
            pastaDestino,
            "relatorio_" + modo,
            ".txt"
        );

    std::ofstream relatorio(destino);

    if (!relatorio.is_open()) {
        std::cerr
            << "[ERRO] Não foi possível criar "
            << "o relatório.\n";

        return;
    }

    relatorio
        << "RELEMBRE - RELATÓRIO DE FILTROS\n";

    relatorio
        << "Modo: "
        << modo
        << "\n";

    relatorio
        << "Arquivo: "
        << arquivo
        << "\n\n";

    for (size_t i = 0; i < escolhas.size(); ++i) {
        relatorio
            << (i + 1)
            << ". "
            << NomeFiltro(escolhas[i])
            << " ["
            << escolhas[i]
            << "]\n";

        relatorio
            << "   alfa = "
            << filtros[i].alfa
            << "\n";

        relatorio
            << "   beta = "
            << filtros[i].beta
            << "\n";

        relatorio
            << "   gama = "
            << filtros[i].gama
            << "\n";

        relatorio
            << "   delta = "
            << filtros[i].delta
            << "\n";
    }

    relatorio.close();

    std::cout
        << "Relatório salvo em: "
        << destino
        << "\n";
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const std::string pastaDestino =
        "../output/";

    if (
        !fs::exists(pastaDestino) &&
        !fs::create_directories(pastaDestino)
    ) {
        std::cerr
            << "[ERRO] Não foi possível criar "
            << "a pasta output/.\n";

        return 1;
    }

    std::string modo;

    if (argc >= 2) {
        modo = argv[1];
    }
    else {
        Manual();

        std::cout
            << "\nEscolha a versão: ";

        std::cin >> modo;
    }

    if (modo == "free") {
        Render processador;

        while (true) {
            std::cout
                << "\nMODO FREE\n";

            std::cout
                << "[1] Selecionar arquivo(s)\n";

            std::cout
                << "[2] Usar webcam\n";

            std::cout
                << "[0] Sair\n";

            std::cout
                << "Escolha: ";

            int opcao;
            std::cin >> opcao;

            if (opcao == 0) {
                break;
            }

            if (opcao == 2) {
                int largura = 0;
                int altura = 0;

                if (
                    !AbrirWebcam(
                        processador,
                        largura,
                        altura
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível "
                        << "acessar a webcam.\n";

                    continue;
                }

                std::cout
                    << "Webcam aberta.\n";

                processador.janela();
                continue;
            }

            if (opcao != 1) {
                std::cout
                    << "Opção inválida.\n";

                continue;
            }

            const std::vector<std::string> arquivos =
                SelecionarArquivos(true);

            if (arquivos.empty()) {
                std::cout
                    << "Nenhum arquivo selecionado.\n";

                continue;
            }

            int largura = 0;
            int altura = 0;

            if (
                !AbrirArquivo(
                    processador,
                    arquivos.front(),
                    largura,
                    altura
                )
            ) {
                std::cerr
                    << "[ERRO] Não foi possível ler "
                    << "o primeiro arquivo selecionado.\n";

                continue;
            }

            Filtros();

            int escolha;

            std::cout
                << ">> Escolha um filtro: ";

            std::cin >> escolha;

            if (
                escolha < 1 ||
                escolha > 11
            ) {
                std::cout
                    << "Opção inválida.\n";

                continue;
            }

            Parametros filtro;
            DadosRecorte dadosRecorte;

            if (
                !LerParametros(
                    processador,
                    escolha,
                    filtro,
                    largura,
                    altura,
                    &dadosRecorte
                )
            ) {
                continue;
            }

            for (const std::string& arquivo : arquivos) {
                ProcessarArquivoFree(
                    processador,
                    arquivo,
                    escolha,
                    filtro,
                    dadosRecorte,
                    pastaDestino
                );
            }

            std::cout
                << "\nProcesso concluído. "
                << "Deseja processar novamente? (s/n): ";

            char repetir;
            std::cin >> repetir;

            if (
                repetir != 's' &&
                repetir != 'S'
            ) {
                break;
            }
        }
    }
    else if (modo == "pro") {
        Render processador;

        while (true) {
            std::cout
                << "\nMODO PRO\n";

            std::cout
                << "[1] Selecionar arquivo\n";

            std::cout
                << "[2] Usar webcam\n";

            std::cout
                << "[0] Sair\n";

            std::cout
                << "Escolha: ";

            int fonte;
            std::cin >> fonte;

            if (fonte == 0) {
                break;
            }

            const bool camera =
                fonte == 2;

            std::vector<std::string> arquivos;

            int largura = 0;
            int altura = 0;

            if (camera) {
                if (
                    !AbrirWebcam(
                        processador,
                        largura,
                        altura
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível "
                        << "acessar a webcam.\n";

                    continue;
                }
            }
            else if (fonte == 1) {
                arquivos =
                    SelecionarArquivos(false);

                if (arquivos.empty()) {
                    std::cout
                        << "Nenhum arquivo selecionado.\n";

                    continue;
                }

                if (
                    !AbrirArquivo(
                        processador,
                        arquivos.front(),
                        largura,
                        altura
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível "
                        << "abrir a mídia.\n";

                    continue;
                }
            }
            else {
                std::cout
                    << "Opção inválida.\n";

                continue;
            }

            std::vector<int> sequencia;
            std::vector<Parametros> filtros;

            if (
                !MontarSequencia(
                    processador,
                    largura,
                    altura,
                    sequencia,
                    filtros,
                    true
                )
            ) {
                break;
            }

            if (camera) {
                RelatorioFiltros(
                    "pro",
                    "webcam",
                    sequencia,
                    filtros,
                    pastaDestino
                );

                ProcessarWebcam(
                    processador,
                    sequencia,
                    filtros,
                    pastaDestino,
                    "resultado_pro_webcam"
                );
            }
            else {
                RelatorioFiltros(
                    "pro",
                    arquivos.front(),
                    sequencia,
                    filtros,
                    pastaDestino
                );

                ProcessarArquivo(
                    processador,
                    arquivos.front(),
                    sequencia,
                    filtros,
                    pastaDestino,
                    "resultado_pro"
                );
            }

            std::cout
                << "\nProcesso concluído. "
                << "Deseja iniciar outro processo? (s/n): ";

            char repetir;
            std::cin >> repetir;

            if (
                repetir != 's' &&
                repetir != 'S'
            ) {
                break;
            }
        }
    }
    else if (modo == "demo") {
        Render processador;

        while (true) {
            std::cout
                << "\nMODO DEMO\n";

            std::cout
                << "[1] Selecionar arquivo\n";

            std::cout
                << "[2] Usar webcam\n";

            std::cout
                << "[0] Sair\n";

            std::cout
                << "Escolha: ";

            int fonte;
            std::cin >> fonte;

            if (fonte == 0) {
                break;
            }

            const bool camera =
                fonte == 2;

            std::vector<std::string> arquivos;

            int largura = 0;
            int altura = 0;

            if (camera) {
                if (
                    !AbrirWebcam(
                        processador,
                        largura,
                        altura
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível "
                        << "acessar a webcam.\n";

                    continue;
                }
            }
            else if (fonte == 1) {
                arquivos =
                    SelecionarArquivos(false);

                if (arquivos.empty()) {
                    std::cout
                        << "Nenhum arquivo selecionado.\n";

                    continue;
                }

                if (
                    !AbrirArquivo(
                        processador,
                        arquivos.front(),
                        largura,
                        altura
                    )
                ) {
                    std::cerr
                        << "[ERRO] Não foi possível "
                        << "abrir a mídia.\n";

                    continue;
                }
            }
            else {
                std::cout
                    << "Opção inválida.\n";

                continue;
            }

            std::vector<int> sequencia;
            std::vector<Parametros> filtros;

            if (
                !MontarSequencia(
                    processador,
                    largura,
                    altura,
                    sequencia,
                    filtros,
                    true
                )
            ) {
                break;
            }

            processador.comparar(
                sequencia,
                filtros
            );

            RelatorioFiltros(
                "demo",
                camera
                    ? "webcam"
                    : arquivos.front(),
                processador.filtrosUsados(),
                processador.parametrosUsados(),
                pastaDestino
            );

            std::cout
                << "\nDemonstração concluída. "
                << "Deseja iniciar outra? (s/n): ";

            char repetir;
            std::cin >> repetir;

            if (
                repetir != 's' &&
                repetir != 'S'
            ) {
                break;
            }
        }
    }
    else {
        Manual();
        return 1;
    }

    return 0;
}
