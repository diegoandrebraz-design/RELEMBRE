#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <cmath>
#include <limits>
#include <system_error>
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

template <typename T>
bool LerValor(const std::string& mensagem, T& valor) {
    std::cout << mensagem;
    if (std::cin >> valor) {
        return true;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cerr << "[ERRO] Entrada inválida. Tente novamente.\n";
    return false;
}

std::string Renomear(const std::string& pasta,
                     const std::string& prefixo,
                     const std::string& extensao) {
    int contador = 1;
    std::string nome;

    do {
        nome = pasta + prefixo + "_" + std::to_string(contador) + extensao;
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
    std::cout << "[11] Ajustar Saturação\n";
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
    case 11: return "Ajustar Saturação";
    default: return "Desconhecido";
    }
}

std::vector<std::string> SelecionarArquivos(bool multiplos) {
    char buffer[65536] = {};
    OPENFILENAMEA dialogo{};

    dialogo.lStructSize = sizeof(dialogo);
    dialogo.lpstrFile = buffer;
    dialogo.nMaxFile = sizeof(buffer);
    dialogo.lpstrFilter =
        "Mídias suportadas\0*.png;*.jpg;*.jpeg;*.bmp;*.webp;*.mp4;*.avi;*.mov;*.mkv;*.wmv\0"
        "Todos os arquivos\0*.*\0";
    dialogo.nFilterIndex = 1;
    dialogo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (multiplos) {
        dialogo.Flags |= OFN_ALLOWMULTISELECT;
    }

    if (!GetOpenFileNameA(&dialogo)) {
        return {};
    }

    std::vector<std::string> arquivos;
    char* cursor = buffer;

    if (!multiplos || cursor[std::strlen(cursor) + 1] == '\0') {
        arquivos.emplace_back(cursor);
        return arquivos;
    }

    std::string pasta = cursor;
    cursor += pasta.size() + 1;

    while (*cursor) {
        std::string nome = cursor;
        arquivos.push_back(pasta + "\\" + nome);
        cursor += nome.size() + 1;
    }

    return arquivos;
}

bool LerParametros(Render& processador,
                   int escolha,
                   Parametros& filtro,
                   int largura,
                   int altura,
                   DadosRecorte& dadosRecorte) {
    filtro = {1.0, 0.0f, 0, 0};

    switch (escolha) {
    case 1: {
        if (!LerValor(" [Girar] Ângulo em graus: ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) {
            std::cout << "[ERRO] O ângulo precisa ser um número finito.\n";
            return false;
        }
        if (!LerValor(" [Girar] Espelhamento (1 Horizontal | 0 Vertical | -1 Ambos | 2 Nenhum): ", filtro.gama)) return false;
        if (filtro.gama != 1 && filtro.gama != 0 && filtro.gama != -1 && filtro.gama != 2) {
            std::cout << "[ERRO] Escolha de espelhamento inválida.\n";
            return false;
        }
        break;
    }
    case 2: {
        if (dadosRecorte.ativo) {
            std::cout << "[ERRO] O Render atual mantém um único ponto de origem para o recorte. Use apenas um filtro de recorte por sequência.\n";
            return false;
        }

        int esquerda = 0, direita = 0, topo = 0, base = 0;
        if (!LerValor(" [Recortar] Margem esquerda em pixels: ", esquerda) ||
            !LerValor(" [Recortar] Margem direita em pixels: ", direita) ||
            !LerValor(" [Recortar] Margem superior em pixels: ", topo) ||
            !LerValor(" [Recortar] Margem inferior em pixels: ", base)) {
            return false;
        }

        if (esquerda < 0 || topo < 0 || direita < 0 || base < 0 ||
            esquerda >= largura || topo >= altura ||
            direita >= largura - esquerda || base >= altura - topo) {
            std::cout << "[ERRO] As margens devem ser não negativas e deixar uma área de imagem válida.\n";
            return false;
        }

        dadosRecorte = {esquerda, direita, topo, base, true};
        filtro.gama = largura - esquerda - direita;
        filtro.delta = altura - topo - base;
        processador.recorte(esquerda, topo);
        break;
    }
    case 3:
        if (!LerValor(" [Granulação] Intensidade (0 a 100): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] A intensidade precisa ser finita.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Granulação ajustada para o intervalo 0 a 100.\n"; }
        break;
    case 4:
        if (!LerValor(" [Nitidez] Intensidade (0 a 100): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] A intensidade precisa ser finita.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Nitidez ajustada para o intervalo 0 a 100.\n"; }
        break;
    case 5:
        if (!LerValor(" [Desfocar] Kernel (inteiro positivo; valores pares são ajustados pelo filtro): ", filtro.gama)) return false;
        if (filtro.gama < 1) {
            std::cout << "[ERRO] O kernel precisa ser positivo.\n";
            return false;
        }
        break;
    case 6:
        if (!LerValor(" [Remover Falhas] Sensibilidade (0 usa o padrão 40; 1 a 254 ajusta a sensibilidade): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] A sensibilidade precisa ser finita.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 254.0); if (original != filtro.alfa) std::cout << "[AVISO] Sensibilidade ajustada para o intervalo 0 a 254.\n"; }
        break;
    case 7: {
        if (!LerValor(" [Reduzir Ruídos] Intensidade (0 a 10; 0 mantém a imagem): ", filtro.beta)) return false;
        if (!std::isfinite(filtro.beta)) {
            std::cout << "[ERRO] A intensidade precisa ser finita.\n";
            return false;
        }
        const float ruidoOriginal = filtro.beta;
        filtro.beta = std::clamp(filtro.beta, 0.0f, 10.0f);
        if (filtro.beta != ruidoOriginal) std::cout << "[AVISO] Intensidade ajustada para o intervalo 0 a 10.\n";
        break;
    }
    case 8:
        if (!LerValor(" [Brilho] Valor (0 a 100; 50 mantém o brilho): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] O valor precisa ser finito.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Brilho ajustado para o intervalo 0 a 100.\n"; }
        break;
    case 9:
        if (!LerValor(" [Contraste] Valor (0 a 100; 50 é o valor neutro): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] O valor precisa ser finito.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Contraste ajustado para o intervalo 0 a 100.\n"; }
        break;
    case 10:
        if (!LerValor(" [Cores] Intensidade (0 a 100; 50 mantém o canal): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] A intensidade precisa ser finita.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Intensidade ajustada para o intervalo 0 a 100.\n"; }
        if (!LerValor(" [Cores] Canal (1 Azul | 2 Verde | 3 Vermelho): ", filtro.gama)) return false;
        if (filtro.gama < 1 || filtro.gama > 3) {
            std::cout << "[ERRO] O canal deve ser 1, 2 ou 3.\n";
            return false;
        }
        break;
    case 11:
        if (!LerValor(" [Saturação] Intensidade (0 a 100; 0 = sem saturação, 100 = saturação original): ", filtro.alfa)) return false;
        if (!std::isfinite(filtro.alfa)) { std::cout << "[ERRO] A intensidade precisa ser finita.\n"; return false; }
        { const double original = filtro.alfa; filtro.alfa = std::clamp(filtro.alfa, 0.0, 100.0); if (original != filtro.alfa) std::cout << "[AVISO] Saturação ajustada para o intervalo 0 a 100.\n"; }
        break;
    default:
        std::cout << "Opção inválida.\n";
        return false;
    }

    return true;
}

// Reproduz o cálculo geométrico usado por Render::girar para que um recorte
// adicionado depois de uma rotação use as dimensões realmente resultantes.
void DimensoesRotacionadas(int largura, int altura, double angulo,
                           int& novaLargura, int& novaAltura) {
    if (largura <= 0 || altura <= 0 || !std::isfinite(angulo)) {
        novaLargura = largura;
        novaAltura = altura;
        return;
    }

    const double radianos = std::remainder(angulo, 360.0) * CV_PI / 180.0;
    const double c = std::cos(radianos);
    const double sn = std::sin(radianos);
    const double cx = (largura - 1) / 2.0;
    const double cy = (altura - 1) / 2.0;
    const double tx = (1.0 - c) * cx - sn * cy;
    const double ty = sn * cx + (1.0 - c) * cy;
    const double xs[] = {-0.5, largura - 0.5, -0.5, largura - 0.5};
    const double ys[] = {-0.5, -0.5, altura - 0.5, altura - 0.5};

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    for (int i = 0; i < 4; ++i) {
        const double x = c * xs[i] + sn * ys[i] + tx;
        const double y = -sn * xs[i] + c * ys[i] + ty;
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }
    novaLargura = std::max(1, static_cast<int>(std::ceil(maxX - minX)));
    novaAltura = std::max(1, static_cast<int>(std::ceil(maxY - minY)));
}

bool PrepararFiltrosParaFonte(Render& processador,
                              int largura,
                              int altura,
                              const std::vector<int>& escolhas,
                              const std::vector<Parametros>& filtros,
                              const DadosRecorte& dadosRecorte,
                              std::vector<Parametros>& filtrosFonte) {
    if (largura <= 0 || altura <= 0 || escolhas.size() != filtros.size()) {
        return false;
    }

    filtrosFonte = filtros;
    processador.recorte(0, 0);
    int larguraAtual = largura;
    int alturaAtual = altura;

    for (std::size_t i = 0; i < escolhas.size(); ++i) {
        if (escolhas[i] == 1) {
            DimensoesRotacionadas(larguraAtual, alturaAtual,
                                  filtrosFonte[i].alfa,
                                  larguraAtual, alturaAtual);
        } else if (escolhas[i] == 2) {
            if (!dadosRecorte.ativo) {
                std::cerr << "[ERRO] O filtro de recorte não possui margens configuradas.\n";
                return false;
            }
            const long long larguraCalculada = static_cast<long long>(larguraAtual) -
                dadosRecorte.esquerda - dadosRecorte.direita;
            const long long alturaCalculada = static_cast<long long>(alturaAtual) -
                dadosRecorte.topo - dadosRecorte.base;
            if (dadosRecorte.esquerda < 0 || dadosRecorte.topo < 0 ||
                dadosRecorte.direita < 0 || dadosRecorte.base < 0 ||
                dadosRecorte.esquerda >= larguraAtual || dadosRecorte.topo >= alturaAtual ||
                larguraCalculada <= 0 || alturaCalculada <= 0 ||
                larguraCalculada > std::numeric_limits<int>::max() ||
                alturaCalculada > std::numeric_limits<int>::max()) {
                std::cerr << "[ERRO] As margens de recorte não cabem nesta mídia ("
                          << larguraAtual << "x" << alturaAtual << ").\n";
                return false;
            }
            const int novaLargura = static_cast<int>(larguraCalculada);
            const int novaAltura = static_cast<int>(alturaCalculada);
            filtrosFonte[i].gama = novaLargura;
            filtrosFonte[i].delta = novaAltura;
            processador.recorte(dadosRecorte.esquerda, dadosRecorte.topo);
            larguraAtual = novaLargura;
            alturaAtual = novaAltura;
        }
    }
    return true;
}

void RelatorioFiltros(const std::string& modo,
                      const std::string& arquivo,
                      const std::vector<int>& escolhas,
                      const std::vector<Parametros>& filtros,
                      const std::string& pastaDestino) {
    const std::string destino = Renomear(
        pastaDestino,
        "relatorio_" + modo,
        ".txt"
    );

    std::ofstream relatorio(destino);
    if (!relatorio.is_open()) {
        std::cerr << "[ERRO] Não foi possível criar o relatório.\n";
        return;
    }

    relatorio << "RELEMBRE - RELATÓRIO DE FILTROS\n";
    relatorio << "Modo: " << modo << "\n";
    relatorio << "Arquivo: " << arquivo << "\n\n";

    for (size_t i = 0; i < escolhas.size(); ++i) {
        relatorio << (i + 1) << ". "
                  << NomeFiltro(escolhas[i]) << " ["
                  << escolhas[i] << "]\n";
        relatorio << "   alfa = " << filtros[i].alfa << "\n";
        relatorio << "   beta = " << filtros[i].beta << "\n";
        relatorio << "   gama = " << filtros[i].gama << "\n";
        relatorio << "   delta = " << filtros[i].delta << "\n";
    }

    relatorio.close();
    std::cout << "Relatório salvo em: " << destino << "\n";
}

bool AbrirArquivo(Render& processador,
                  const std::string& arquivo,
                  int& largura,
                  int& altura,
                  int* canais = nullptr) {
    try {
        processador.midia(arquivo);

        // Contrato do Render atual: imagem estática carregada em imagem interna,
        // vídeo aberto em leitor. Não classificar a mídia só pela extensão.
        if (!processador.leitor.isOpened()) {
            const cv::Mat imagem = cv::imread(arquivo, cv::IMREAD_UNCHANGED);
            if (imagem.empty()) {
                std::cerr << "[ERRO] O Render não abriu a mídia como imagem nem como vídeo: " << arquivo << "\n";
                return false;
            }
            largura = imagem.cols;
            altura = imagem.rows;
            if (canais) *canais = imagem.channels();
            return true;
        }

        cv::Mat primeiro;
        processador.leitor >> primeiro;
        if (primeiro.empty()) {
            processador.leitor.release();
            std::cerr << "[ERRO] O arquivo foi reconhecido como vídeo, mas não forneceu um primeiro quadro válido: " << arquivo << "\n";
            return false;
        }

        largura = primeiro.cols;
        altura = primeiro.rows;
        if (canais) *canais = primeiro.channels();
        if (!processador.leitor.set(cv::CAP_PROP_POS_FRAMES, 0)) {
            std::cerr << "[AVISO] Não foi possível confirmar o reposicionamento no início do vídeo; a leitura será iniciada da posição atual.\n";
        }
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "[ERRO] OpenCV falhou ao abrir a mídia '" << arquivo << "': " << e.what() << "\n";
        return false;
    }
}

bool AbrirWebcam(Render& processador,
                 int& largura,
                 int& altura,
                 int* canais = nullptr) {
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
            if (canais) *canais = quadro.channels();
            return true;
        }
        cv::waitKey(30);
    }

    processador.leitor.release();
    std::cerr << "[ERRO] A câmera abriu, mas não forneceu quadros válidos após 10 tentativas. Verifique permissões, uso por outro aplicativo e conexão.\n";
    return false;
}

std::string ExtensaoImagemSaida(const std::string& arquivo) {
    std::string extensao = fs::path(arquivo).extension().string();
    std::transform(extensao.begin(), extensao.end(), extensao.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (extensao == ".png" || extensao == ".jpg" || extensao == ".jpeg" ||
        extensao == ".bmp" || extensao == ".webp" || extensao == ".tif" ||
        extensao == ".tiff" || extensao == ".jp2" || extensao == ".pbm" ||
        extensao == ".pgm" || extensao == ".ppm" || extensao == ".pnm") {
        return extensao;
    }
    return ".png";
}

bool ValidarCompatibilidadeFiltros(const std::vector<int>& escolhas,
                                   int canais,
                                   const std::string& fonte) {
    const bool exigeCor = std::find(escolhas.begin(), escolhas.end(), 10) != escolhas.end() ||
                          std::find(escolhas.begin(), escolhas.end(), 11) != escolhas.end();
    if (exigeCor && canais != 3 && canais != 4) {
        std::cerr << "[ERRO] A sequência inclui Alterar Cores ou Ajustar Saturação, mas '"
                  << fonte << "' fornece " << canais
                  << " canal(is). O Render atual só aplica esses dois filtros a imagens de 3 ou 4 canais; a sequência foi interrompida para não salvar um resultado sem o efeito solicitado.\n";
        return false;
    }
    return true;
}

bool PrepararQuadroVideo(const cv::Mat& entrada, cv::Mat& saida) {
    if (entrada.empty()) return false;
    if (entrada.channels() == 3) {
        saida = entrada;
    } else if (entrada.channels() == 1) {
        cv::cvtColor(entrada, saida, cv::COLOR_GRAY2BGR);
    } else if (entrada.channels() == 4) {
        cv::cvtColor(entrada, saida, cv::COLOR_BGRA2BGR);
    } else {
        return false;
    }
    return !saida.empty();
}

bool ProcessarArquivo(Render& processador,
                      const std::string& arquivo,
                      const std::vector<int>& escolhas,
                      const std::vector<Parametros>& filtros,
                      const std::string& pastaDestino,
                      const std::string& prefixo,
                      const DadosRecorte& dadosRecorte) {
    int largura = 0;
    int altura = 0;
    int canais = 0;

    if (!AbrirArquivo(processador, arquivo, largura, altura, &canais)) {
        std::cerr << "[ERRO] Não foi possível abrir: " << arquivo << "\n";
        return false;
    }

    const bool imagemEstatica = !processador.leitor.isOpened();
    const std::string extensao = imagemEstatica ? ExtensaoImagemSaida(arquivo) : ".mp4";

    if (!ValidarCompatibilidadeFiltros(escolhas, canais, arquivo)) return false;

    std::vector<Parametros> filtrosFonte;
    if (!PrepararFiltrosParaFonte(processador, largura, altura, escolhas,
                                  filtros, dadosRecorte, filtrosFonte)) {
        std::cerr << "[ERRO] A sequência de filtros não é válida para esta mídia.\n";
        return false;
    }

    if (imagemEstatica) {
        cv::Mat resultado;
        try {
            resultado = processador.render(escolhas, filtrosFonte);
        } catch (const cv::Exception& e) {
            std::cerr << "[ERRO] OpenCV falhou ao aplicar os filtros à imagem '" << arquivo << "': " << e.what() << "\n";
            return false;
        }

        if (resultado.empty()) {
            std::cerr << "[ERRO] O processamento gerou uma imagem vazia.\n";
            return false;
        }

        const std::string destino = Renomear(
            pastaDestino,
            prefixo,
            extensao
        );

        try {
            if (!cv::imwrite(destino, resultado)) {
                std::cerr << "[ERRO] Não foi possível salvar: " << destino << "\n";
                return false;
            }
        } catch (const cv::Exception& e) {
            std::cerr << "[ERRO] Falha ao salvar a imagem: " << e.what() << "\n";
            return false;
        }

        std::cout << "[SUCESSO] Imagem salva: " << destino << "\n";
        return true;
    }

    const std::string destino = Renomear(
        pastaDestino,
        prefixo,
        ".mp4"
    );

    double fps = processador.leitor.get(cv::CAP_PROP_FPS);
    if (!std::isfinite(fps) || fps < 1.0 || fps > 240.0) fps = 30.0;
    cv::VideoWriter gravador;
    const int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    bool pausado = false;
    std::size_t quadrosGravados = 0;
    cv::Size tamanhoGravacao;

    cv::namedWindow("Resultado - Stream", cv::WINDOW_NORMAL);

    while (true) {
        if (!pausado) {
            cv::Mat resultado;
            try {
                resultado = processador.render(escolhas, filtrosFonte);
            } catch (const cv::Exception& e) {
                std::cerr << "[ERRO] OpenCV falhou durante o processamento do vídeo: " << e.what() << "\n";
                break;
            }

            if (resultado.empty()) {
                std::cout << "[INFO] A leitura do vídeo terminou ou deixou de fornecer quadros.\n";
                break;
            }

            cv::Mat quadroGravacao;
            if (!PrepararQuadroVideo(resultado, quadroGravacao)) {
                std::cerr << "[ERRO] O filtro gerou um formato de quadro não suportado para vídeo.\n";
                break;
            }
            if (!gravador.isOpened()) {
                if (!gravador.open(destino, codec, fps, quadroGravacao.size(), true)) {
                    std::cerr << "[ERRO] Não foi possível criar o vídeo de saída.\n";
                    cv::destroyWindow("Resultado - Stream");
                    return false;
                }
                tamanhoGravacao = quadroGravacao.size();
            } else if (quadroGravacao.size() != tamanhoGravacao) {
                std::cerr << "[ERRO] O tamanho dos quadros mudou durante a gravação.\n";
                break;
            }

            gravador.write(quadroGravacao);
            ++quadrosGravados;
            cv::imshow("Resultado - Stream", resultado);
        }

        const int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 'C' || tecla == 27) {
            break;
        }
        if (tecla == 'p' || tecla == 'P') {
            pausado = !pausado;
        }
        else if (tecla == 'r' || tecla == 'R') {
            if (!AbrirArquivo(processador, arquivo, largura, altura)) {
                std::cerr << "[ERRO] Não foi possível reiniciar a mídia.\n";
                break;
            }
            pausado = false;
        }
    }

    if (gravador.isOpened()) {
        gravador.release();
    }

    cv::destroyWindow("Resultado - Stream");

    if (quadrosGravados == 0) {
        std::error_code erro;
        fs::remove(destino, erro);
        std::cerr << "[ERRO] Nenhum quadro foi gravado; o vídeo não foi salvo.\n";
        return false;
    }

    std::cout << "[SUCESSO] Arquivo de vídeo gerado: " << destino
              << " (" << quadrosGravados << " quadros gravados).\n";
    std::cout << "[INFO] Use C ou Esc para encerrar; se a leitura parou antes do fim esperado, verifique a integridade e o codec do vídeo de origem.\n";
    return true;
}

bool ProcessarArquivoFree(Render& processador,
                          const std::string& arquivo,
                          int escolha,
                          const Parametros& filtro,
                          const DadosRecorte& dadosRecorte,
                          const std::string& pastaDestino) {
    // O modo Free mantém o fluxo original de um filtro por lote de arquivos.
    // O processamento real é compartilhado para não divergir no tratamento
    // de imagens estáticas, vídeos, recorte, gravação e mensagens de erro.
    return ProcessarArquivo(
        processador,
        arquivo,
        std::vector<int>{escolha},
        std::vector<Parametros>{filtro},
        pastaDestino,
        "resultado_free_" + NomeFiltro(escolha),
        dadosRecorte
    );
}

bool ProcessarWebcam(Render& processador,
                      const std::vector<int>& escolhas,
                      const std::vector<Parametros>& filtros,
                      const std::string& pastaDestino,
                      const std::string& prefixo,
                      int largura,
                      int altura,
                      int canais,
                      const DadosRecorte& dadosRecorte) {
    if (!ValidarCompatibilidadeFiltros(escolhas, canais, "webcam")) return false;
    std::vector<Parametros> filtrosFonte;
    if (!PrepararFiltrosParaFonte(processador, largura, altura, escolhas,
                                  filtros, dadosRecorte, filtrosFonte)) {
        std::cerr << "[ERRO] A sequência de filtros não é válida para a webcam.\n";
        return false;
    }
    const std::string destino = Renomear(
        pastaDestino,
        prefixo,
        ".mp4"
    );

    double fps = processador.leitor.get(cv::CAP_PROP_FPS);
    if (!std::isfinite(fps) || fps < 1.0 || fps > 240.0) fps = 30.0;
    cv::VideoWriter gravador;
    const int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    bool pausado = false;
    std::size_t quadrosGravados = 0;
    int leiturasVazias = 0;
    cv::Size tamanhoGravacao;

    cv::namedWindow("Resultado - Webcam", cv::WINDOW_NORMAL);

    while (true) {
        if (!pausado) {
            cv::Mat resultado;
            try {
                resultado = processador.render(escolhas, filtrosFonte);
            } catch (const cv::Exception& e) {
                std::cerr << "[ERRO] OpenCV falhou durante o processamento da webcam: " << e.what() << "\n";
                break;
            }

            if (resultado.empty()) {
                ++leiturasVazias;
                const int teclaVazia = cv::waitKey(10) & 0xFF;

                if (teclaVazia == 'c' || teclaVazia == 'C' || teclaVazia == 27) {
                    break;
                }
                if (teclaVazia == 'p' || teclaVazia == 'P') {
                    pausado = !pausado;
                } else if (teclaVazia == 'r' || teclaVazia == 'R') {
                    processador.camera(0);
                    if (!processador.leitor.isOpened()) {
                        std::cerr << "[ERRO] Não foi possível reabrir a webcam. Confira a conexão, as permissões e se outro programa está usando a câmera.\n";
                        break;
                    }
                    pausado = false;
                    leiturasVazias = 0;
                }
                if (leiturasVazias >= 100) {
                    std::cerr << "[ERRO] A webcam deixou de fornecer quadros.\n";
                    break;
                }
                continue;
            }

            leiturasVazias = 0;

            cv::Mat quadroGravacao;
            if (!PrepararQuadroVideo(resultado, quadroGravacao)) {
                std::cerr << "[ERRO] O filtro gerou um formato de quadro não suportado para vídeo.\n";
                break;
            }
            if (!gravador.isOpened()) {
                if (!gravador.open(destino, codec, fps, quadroGravacao.size(), true)) {
                    std::cerr << "[ERRO] Não foi possível criar o vídeo da webcam.\n";
                    cv::destroyWindow("Resultado - Webcam");
                    return false;
                }
                tamanhoGravacao = quadroGravacao.size();
            } else if (quadroGravacao.size() != tamanhoGravacao) {
                std::cerr << "[ERRO] O tamanho dos quadros mudou durante a gravação.\n";
                break;
            }

            gravador.write(quadroGravacao);
            ++quadrosGravados;
            cv::imshow("Resultado - Webcam", resultado);
        }

        const int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 'C' || tecla == 27) {
            break;
        }
        if (tecla == 'p' || tecla == 'P') {
            pausado = !pausado;
        }
        else if (tecla == 'r' || tecla == 'R') {
            processador.camera(0);
            if (!processador.leitor.isOpened()) {
                std::cerr << "[ERRO] Não foi possível reabrir a webcam. Confira a conexão, as permissões e se outro programa está usando a câmera.\n";
                break;
            }
            pausado = false;
            leiturasVazias = 0;
        }
    }

    if (gravador.isOpened()) {
        gravador.release();
    }

    cv::destroyWindow("Resultado - Webcam");

    if (quadrosGravados == 0) {
        std::error_code erro;
        fs::remove(destino, erro);
        std::cerr << "[ERRO] Nenhum quadro da webcam foi gravado.\n";
        return false;
    }

    std::cout << "[SUCESSO] Vídeo da webcam salvo: " << destino
              << " (" << quadrosGravados << " quadros)\n";
    return true;
}

bool MontarSequencia(Render& processador,
                     int largura,
                     int altura,
                     std::vector<int>& sequencia,
                     std::vector<Parametros>& filtros,
                     bool permitirSair,
                     DadosRecorte& dadosRecorte) {
    sequencia.clear();
    filtros.clear();
    dadosRecorte = {};
    processador.recorte(0, 0);

    int larguraAtual = largura;
    int alturaAtual = altura;

    Filtros();
    std::cout << "Digite 0 para iniciar";
    if (permitirSair) std::cout << " ou -1 para sair";
    std::cout << ".\n";

    while (true) {
        int escolha = 0;
        if (!LerValor(">> Adicionar filtro: ", escolha)) continue;

        if (escolha == 0) break;
        if (permitirSair && escolha == -1) return false;
        if (escolha < 1 || escolha > 11) {
            std::cout << "Opção inválida.\n";
            continue;
        }

        Parametros filtro;
        if (!LerParametros(processador, escolha, filtro,
                           larguraAtual, alturaAtual, dadosRecorte)) {
            continue;
        }

        sequencia.push_back(escolha);
        filtros.push_back(filtro);

        if (escolha == 1) {
            DimensoesRotacionadas(larguraAtual, alturaAtual, filtro.alfa,
                                  larguraAtual, alturaAtual);
        } else if (escolha == 2) {
            larguraAtual = filtro.gama;
            alturaAtual = filtro.delta;
        }
    }

    // O Render atual aceita vetores vazios: a mídia original é devolvida sem filtros.
    if (sequencia.empty()) {
        std::cout << "[INFO] Nenhum filtro selecionado. A mídia será processada sem alterações de filtro.\n";
    }
    return true;
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const std::string pastaDestino = "../output/";

    std::error_code erroPasta;
    if (!fs::exists(pastaDestino) && !fs::create_directories(pastaDestino, erroPasta)) {
        std::cerr << "[ERRO] Não foi possível criar a pasta output/.\n";
        return 1;
    }
    if (!fs::is_directory(pastaDestino)) {
        std::cerr << "[ERRO] O caminho output/ existe, mas não é uma pasta.\n";
        return 1;
    }

    std::string modo;
    if (argc >= 2) {
        modo = argv[1];
    }
    else {
        Manual();
        std::cout << "\nEscolha a versão: ";
        if (!(std::cin >> modo)) return 1;
    }

    std::transform(modo.begin(), modo.end(), modo.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (modo == "free") {
        Render processador;

        while (true) {
            std::cout << "\nMODO FREE\n";
            std::cout << "[1] Selecionar arquivo(s)\n";
            std::cout << "[2] Scanner de documentos pela webcam\n";
            std::cout << "[3] Aplicar filtros à webcam e salvar vídeo\n";
            std::cout << "[0] Sair\n";
            int opcao = -1;
            if (!LerValor("Escolha: ", opcao)) continue;

            if (opcao == 0) {
                break;
            }

            if (opcao == 2) {
                int largura = 0;
                int altura = 0;
                int canais = 0;

                if (!AbrirWebcam(processador, largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível acessar a webcam pelo backend CAP_DSHOW usado por Render::camera(). Verifique se a câmera está conectada, autorizada no Windows e não está ocupada por outro programa.\n";
                    continue;
                }

                std::cout << "Webcam aberta.\n";
                processador.janela();
                continue;
            }

            if (opcao == 3) {
                int largura = 0;
                int altura = 0;
                int canais = 0;
                if (!AbrirWebcam(processador, largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível acessar a webcam pelo backend CAP_DSHOW usado por Render::camera(). Verifique se a câmera está conectada, autorizada no Windows e não está ocupada por outro programa.\n";
                    continue;
                }

                std::vector<int> sequencia;
                std::vector<Parametros> filtros;
                DadosRecorte dadosRecorte;
                if (!MontarSequencia(processador, largura, altura, sequencia,
                                    filtros, false, dadosRecorte)) {
                    continue;
                }
                ProcessarWebcam(processador, sequencia, filtros, pastaDestino,
                                "resultado_free_webcam", largura, altura, canais, dadosRecorte);
                continue;
            }

            if (opcao != 1) {
                std::cout << "Opção inválida.\n";
                continue;
            }

            const std::vector<std::string> arquivos = SelecionarArquivos(true);
            if (arquivos.empty()) {
                std::cout << "Nenhum arquivo selecionado.\n";
                continue;
            }

            int largura = 0;
            int altura = 0;
            int canais = 0;

            if (!AbrirArquivo(processador, arquivos.front(), largura, altura, &canais)) {
                std::cerr << "[ERRO] Não foi possível ler o primeiro arquivo selecionado.\n";
                continue;
            }

            Filtros();
            int escolha = 0;
            if (!LerValor(">> Escolha um filtro: ", escolha)) continue;
            if (escolha < 1 || escolha > 11) {
                std::cout << "[ERRO] Opção de filtro inválida. Escolha um valor entre 1 e 11.\n";
                continue;
            }

            Parametros filtro;
            DadosRecorte dadosRecorte;
            if (!LerParametros(processador, escolha, filtro, largura, altura, dadosRecorte)) {
                std::cout << "[INFO] Parâmetros inválidos; nenhum arquivo deste lote foi processado.\n";
                continue;
            }

            std::size_t sucessos = 0;
            std::size_t falhas = 0;
            for (const std::string& arquivo : arquivos) {
                std::cout << "\n[INFO] Aplicando '" << NomeFiltro(escolha)
                          << "' a: " << arquivo << "\n";
                if (ProcessarArquivoFree(processador, arquivo, escolha, filtro,
                                         dadosRecorte, pastaDestino)) {
                    ++sucessos;
                } else {
                    ++falhas;
                    std::cerr << "[FALHA] Não foi possível concluir o processamento de: " << arquivo << "\n";
                }
            }
            std::cout << "\n[RESUMO] Lote concluído: " << sucessos
                      << " arquivo(s) processado(s) com sucesso e " << falhas
                      << " com falha.\n";

            std::cout << "\nProcesso concluído. Deseja processar novamente? (s/n): ";
            char repetir = 'n';
            if (!LerValor("", repetir)) repetir = 'n';
            if (repetir != 's' && repetir != 'S') {
                break;
            }
        }
    }
    else if (modo == "pro") {
        Render processador;

        while (true) {
            std::cout << "\nMODO PRO\n";
            std::cout << "[1] Selecionar arquivo\n";
            std::cout << "[2] Usar webcam\n";
            std::cout << "[0] Voltar/sair\n";

            int fonte = -1;
            if (!LerValor("Escolha: ", fonte)) continue;
            if (fonte == 0) break;

            bool camera = fonte == 2;
            std::vector<std::string> arquivos;
            int largura = 0;
            int altura = 0;
            int canais = 0;

            if (camera) {
                if (!AbrirWebcam(processador, largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível acessar a webcam pelo backend CAP_DSHOW usado por Render::camera(). Verifique se a câmera está conectada, autorizada no Windows e não está ocupada por outro programa.\n";
                    continue;
                }
            }
            else if (fonte == 1) {
                arquivos = SelecionarArquivos(false);
                if (arquivos.empty()) {
                    std::cout << "Nenhum arquivo selecionado.\n";
                    continue;
                }

                if (!AbrirArquivo(processador, arquivos.front(), largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível abrir a mídia.\n";
                    continue;
                }
            }
            else {
                std::cout << "Opção inválida.\n";
                continue;
            }

            std::vector<int> sequencia;
            std::vector<Parametros> filtros;
            DadosRecorte dadosRecorte;

            if (!MontarSequencia(processador, largura, altura, sequencia, filtros, false, dadosRecorte)) {
                continue;
            }

            if (!ValidarCompatibilidadeFiltros(sequencia, canais, camera ? "webcam" : arquivos.front())) {
                continue;
            }

            RelatorioFiltros(
                "pro",
                camera ? "webcam" : arquivos.front(),
                sequencia,
                filtros,
                pastaDestino
            );

            if (camera) {
                ProcessarWebcam(
                    processador,
                    sequencia,
                    filtros,
                    pastaDestino,
                    "resultado_pro_webcam",
                    largura,
                    altura,
                    canais,
                    dadosRecorte
                );
            }
            else {
                ProcessarArquivo(
                    processador,
                    arquivos.front(),
                    sequencia,
                    filtros,
                    pastaDestino,
                    "resultado_pro",
                    dadosRecorte
                );
            }

            std::cout << "\nProcesso concluído. Deseja iniciar outro processo? (s/n): ";
            char repetir = 'n';
            if (!LerValor("", repetir)) repetir = 'n';
            if (repetir != 's' && repetir != 'S') {
                break;
            }
        }
    }
    else if (modo == "demo") {
        Render processador;

        while (true) {
            std::cout << "\nMODO DEMO\n";
            std::cout << "[1] Selecionar arquivo\n";
            std::cout << "[2] Usar webcam\n";
            std::cout << "[0] Voltar/sair\n";

            int fonte = -1;
            if (!LerValor("Escolha: ", fonte)) continue;
            if (fonte == 0) break;

            bool camera = fonte == 2;
            std::vector<std::string> arquivos;
            int largura = 0;
            int altura = 0;
            int canais = 0;

            if (camera) {
                if (!AbrirWebcam(processador, largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível acessar a webcam pelo backend CAP_DSHOW usado por Render::camera(). Verifique se a câmera está conectada, autorizada no Windows e não está ocupada por outro programa.\n";
                    continue;
                }
            }
            else if (fonte == 1) {
                arquivos = SelecionarArquivos(false);
                if (arquivos.empty()) {
                    std::cout << "Nenhum arquivo selecionado.\n";
                    continue;
                }

                if (!AbrirArquivo(processador, arquivos.front(), largura, altura, &canais)) {
                    std::cerr << "[ERRO] Não foi possível abrir a mídia.\n";
                    continue;
                }
            }
            else {
                std::cout << "Opção inválida.\n";
                continue;
            }

            std::vector<int> sequencia;
            std::vector<Parametros> filtros;
            DadosRecorte dadosRecorte;

            if (!MontarSequencia(processador, largura, altura, sequencia, filtros, true, dadosRecorte)) {
                break;
            }

            if (!ValidarCompatibilidadeFiltros(sequencia, canais, camera ? "webcam" : arquivos.front())) {
                continue;
            }

            std::vector<Parametros> filtrosFonte;
            if (!PrepararFiltrosParaFonte(processador, largura, altura, sequencia,
                                          filtros, dadosRecorte, filtrosFonte)) {
                std::cerr << "[ERRO] A sequência de filtros não é válida para a fonte escolhida.\n";
                continue;
            }

            const cv::Mat resultadoComparacao = processador.comparar(sequencia, filtrosFonte);
            if (resultadoComparacao.empty()) {
                std::cerr << "[ERRO] A comparação não produziu um resultado válido; relatório não gerado.\n";
                continue;
            }

            RelatorioFiltros(
                "demo",
                camera ? "webcam" : arquivos.front(),
                processador.filtrosUsados(),
                processador.parametrosUsados(),
                pastaDestino
            );

            std::cout << "\nDemonstração concluída. Deseja iniciar outra? (s/n): ";
            char repetir = 'n';
            if (!LerValor("", repetir)) repetir = 'n';
            if (repetir != 's' && repetir != 'S') {
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
