#include "../include/render.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include <unordered_map>
#include <windows.h>

#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>

namespace {
enum class OrigemRender { Nenhuma, Webcam, Imagem, Video };

std::unordered_map<const Render*, OrigemRender>& origensRender() {
    static std::unordered_map<const Render*, OrigemRender> origens;
    return origens;
}

OrigemRender obterOrigem(const Render* render) {
    const auto& origens = origensRender();
    const auto it = origens.find(render);
    return it == origens.end() ? OrigemRender::Nenhuma : it->second;
}

void definirOrigem(const Render* render, OrigemRender origem) {
    origensRender()[render] = origem;
}
}

Render::Render() = default;

cv::Mat Render::render(int escolha, const Parametros& filtro) {
    if (escolha < 1 || escolha > 11) {
        std::cout << "Opção invalida!\n";
        resultado.release();
        return {};
    }

    if (!(obterOrigem(this) == OrigemRender::Imagem && !leitor.isOpened() && !imagem.empty())) {
        if (!leitor.isOpened()) {
            resultado.release();
            return {};
        }
        leitor >> imagem;
    }

    if (imagem.empty()) {
        resultado.release();
        return {};
    }

    switch (escolha) {
    case 1:
        resultado = girar(imagem, filtro.alfa, filtro.gama);
        break;
    case 2:
        resultado = recortar(imagem, filtro.gama, filtro.delta);
        break;
    case 3:
        resultado = granular(imagem, filtro.alfa);
        break;
    case 4:
        resultado = nitidez(imagem, filtro.alfa);
        break;
    case 5:
        resultado = desfocar(imagem, filtro.gama);
        break;
    case 6:
        resultado = remover(imagem, filtro.alfa);
        break;
    case 7:
        resultado = limpar(imagem, filtro.beta);
        break;
    case 8:
        resultado = brilho(imagem, filtro.alfa);
        break;
    case 9:
        resultado = contraste(imagem, filtro.alfa);
        break;
    case 10:
        resultado = cores(imagem, filtro.alfa, filtro.gama);
        break;
    case 11:
        resultado = cinzas(imagem, filtro.alfa);
        break;
    default:
        std::cout << "Opção invalida!\n";
        resultado.release();
        return {};
    }

    if (gravador.isOpened() && !resultado.empty()) {
        gravador.write(resultado);
    }

    return resultado;
}

cv::Mat Render::render(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    if (escolhas.size() != filtro.size()) {
        resultado.release();
        return {};
    }

    for (int escolha : escolhas) {
        if (escolha < 1 || escolha > 11) {
            std::cout << "Opção invalida!\n";
            resultado.release();
            return {};
        }
    }

    if (!(obterOrigem(this) == OrigemRender::Imagem && !leitor.isOpened() && !imagem.empty())) {
        if (!leitor.isOpened()) {
            resultado.release();
            return {};
        }
        leitor >> imagem;
    }

    if (imagem.empty()) {
        resultado.release();
        return {};
    }

    resultado = imagem.clone();

    for (size_t i = 0; i < escolhas.size(); ++i) {
        switch (escolhas[i]) {
        case 1:
            resultado = girar(resultado, filtro[i].alfa, filtro[i].gama);
            break;
        case 2:
            resultado = recortar(resultado, filtro[i].gama, filtro[i].delta);
            break;
        case 3:
            resultado = granular(resultado, filtro[i].alfa);
            break;
        case 4:
            resultado = nitidez(resultado, filtro[i].alfa);
            break;
        case 5:
            resultado = desfocar(resultado, filtro[i].gama);
            break;
        case 6:
            resultado = remover(resultado, filtro[i].alfa);
            break;
        case 7:
            resultado = limpar(resultado, filtro[i].beta);
            break;
        case 8:
            resultado = brilho(resultado, filtro[i].alfa);
            break;
        case 9:
            resultado = contraste(resultado, filtro[i].alfa);
            break;
        case 10:
            resultado = cores(resultado, filtro[i].alfa, filtro[i].gama);
            break;
        case 11:
            resultado = cinzas(resultado, filtro[i].alfa);
            break;
        default:
            std::cout << "Opção invalida!\n";
            resultado.release();
            return {};
        }

        if (resultado.empty()) {
            resultado.release();
            return {};
        }
    }

    if (gravador.isOpened() && !resultado.empty()) {
        gravador.write(resultado);
    }

    return resultado;
}

cv::Mat Render::comparar(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    resultado.release();

    const OrigemRender origem = obterOrigem(this);
    const bool fonteImagemEstatica = origem == OrigemRender::Imagem;
    const bool fonteVideo = origem == OrigemRender::Video;

    if ((!leitor.isOpened() && !(fonteImagemEstatica && !imagem.empty())) ||
        escolhas.size() != filtro.size()) {
        return {};
    }

    std::vector<int> sequencia = escolhas;
    std::vector<Parametros> filtros = filtro;

    bool pausado = false;
    size_t filtroAtual = 0;
    int parametroAtual = 0;
    int quadrosVaziosConsecutivos = 0;
    int errosExibicaoConsecutivos = 0;
    int larguraOriginalComposicao = 0;
    int larguraOriginalUltimoValidoComposicao = 0;

    bool falhaProcessamentoAtual = false;
    bool falhaLeituraAtual = false;
    bool fimNormalVideo = false;
    bool encerrarPorLimiteLeitura = false;

    bool composicaoDesatualizadaPorFalha = false;

    cv::Mat quadroReinicializacaoPendente;
    cv::Mat quadroOriginalFonte;
    cv::Mat quadroBrutoAtual;
    cv::Mat historicoRemoverAntesDoQuadro;
    bool imagemEstaticaConsumida = false;

    const bool fontefilevideo = fonteVideo;
    const bool fontefileimagem = fonteImagemEstatica;

    frameAnterior.release();

    try {
        cv::namedWindow("Comparacao Lado a Lado", cv::WINDOW_NORMAL);
    } catch (const cv::Exception& e) {
        std::cerr << "Erro OpenCV ao criar janela: " << e.what() << std::endl;
        return {};
    } catch (...) {
        std::cerr << "Erro desconhecido ao criar janela." << std::endl;
        return {};
    }

    auto aplicarFiltro = [this](const cv::Mat& entrada, int escolha, const Parametros& parametros) -> cv::Mat {
        switch (escolha) {
        case 1:  return girar(entrada, parametros.alfa, parametros.gama);
        case 2:  return recortar(entrada, parametros.gama, parametros.delta);
        case 3:  return granular(entrada, parametros.alfa);
        case 4:  return nitidez(entrada, parametros.alfa);
        case 5:  return desfocar(entrada, parametros.gama);
        case 6:  return remover(entrada, parametros.alfa);
        case 7:  return limpar(entrada, parametros.beta);
        case 8:  return brilho(entrada, parametros.alfa);
        case 9:  return contraste(entrada, parametros.alfa);
        case 10: return cores(entrada, parametros.alfa, parametros.gama);
        case 11: return cinzas(entrada, parametros.alfa);
        default: return {};
        }
    };

    auto reprocessarQuadroAtual = [&]() {
        cv::Mat imagemAlvo;
        if (!quadroBrutoAtual.empty()) {
            imagemAlvo = quadroBrutoAtual;
        } else if (!quadroOriginalFonte.empty()) {
            imagemAlvo = quadroOriginalFonte;
        }

        if (imagemAlvo.empty()) {
            falhaProcessamentoAtual = true;
            composicaoDesatualizadaPorFalha = true;
            return;
        }

        cv::Mat processado = imagemAlvo.clone();
        bool falhaProc = false;

        // Reprocessar o mesmo quadro deve usar o histórico do quadro anterior,
        // sem substituir o histórico já estabelecido pelo quadro atual.
        cv::Mat historicoAtual = frameAnterior.clone();
        frameAnterior = historicoRemoverAntesDoQuadro.clone();

        for (size_t i = 0; i < sequencia.size(); ++i) {
            try {
                processado = aplicarFiltro(processado, sequencia[i], filtros[i]);
            } catch (const cv::Exception& e) {
                std::cerr << "Erro OpenCV no reprocessamento (filtro " << sequencia[i] << "): " << e.what() << std::endl;
                processado.release();
            } catch (...) {
                std::cerr << "Erro desconhecido no reprocessamento (filtro " << sequencia[i] << ")" << std::endl;
                processado.release();
            }

            if (processado.empty()) {
                falhaProc = true;
                break;
            }
        }

        frameAnterior = historicoAtual;

        if (!falhaProc && !processado.empty()) {
            int procChannels = processado.channels();
            if (procChannels != 1 && procChannels != 3 && procChannels != 4) {
                falhaProc = true;
            }

            if (!falhaProc) {
                try {
                    cv::Mat origTemp = imagemAlvo.clone();
                    if (origTemp.channels() == 4) cv::cvtColor(origTemp, origTemp, cv::COLOR_BGRA2BGR);
                    else if (origTemp.channels() == 1) cv::cvtColor(origTemp, origTemp, cv::COLOR_GRAY2BGR);

                    if (processado.channels() == 4) cv::cvtColor(processado, processado, cv::COLOR_BGRA2BGR);
                    else if (processado.channels() == 1) cv::cvtColor(processado, processado, cv::COLOR_GRAY2BGR);

                    if (origTemp.rows > 0 && processado.rows > 0 && origTemp.size() != processado.size()) {
                        int novaLargura = std::max(1, static_cast<int>(origTemp.cols * (static_cast<double>(processado.rows) / origTemp.rows)));
                        cv::resize(origTemp, origTemp, cv::Size(novaLargura, processado.rows), 0, 0, cv::INTER_LINEAR);
                    }

                    if (origTemp.rows == processado.rows && origTemp.channels() == processado.channels() && origTemp.depth() == processado.depth()) {
                        const int larguraCandidata = origTemp.cols;
                        if (origTemp.empty() || processado.empty() ||
                            origTemp.cols <= 0 || origTemp.rows <= 0 ||
                            processado.cols <= 0 || processado.rows <= 0) {
                            falhaProc = true;
                        } else {
                            cv::Mat resultadoTemp;
                            cv::hconcat(origTemp, processado, resultadoTemp);

                            if (!resultadoTemp.empty()) {
                                resultado = resultadoTemp;
                                larguraOriginalComposicao = larguraCandidata;
                                larguraOriginalUltimoValidoComposicao = larguraCandidata;
                                falhaProcessamentoAtual = false;
                                composicaoDesatualizadaPorFalha = false;

                                falhaLeituraAtual = false;
                                if (fontefileimagem) {
                                    encerrarPorLimiteLeitura = false;
                                }
                            } else {
                                falhaProc = true;
                            }
                        }
                    } else {
                        falhaProc = true;
                    }
                } catch (const cv::Exception& e) {
                    std::cerr << "Erro OpenCV na composicao do reprocessamento: " << e.what() << std::endl;
                    falhaProc = true;
                } catch (...) {
                    std::cerr << "Erro desconhecido na composicao do reprocessamento." << std::endl;
                    falhaProc = true;
                }
            }
        } else {
            falhaProc = true;
        }

        if (falhaProc) {
            falhaProcessamentoAtual = true;
            composicaoDesatualizadaPorFalha = true;

            if (!resultado.empty()) {
                larguraOriginalComposicao =
                    larguraOriginalUltimoValidoComposicao;
            }
        }
    };

    while (true) {
        const bool estadoCongeladoPrincipal =
            pausado ||
            encerrarPorLimiteLeitura ||
            (fontefileimagem && !resultado.empty());

        if (!estadoCongeladoPrincipal) {
            cv::Mat ultimoResultadoValido;
            if (!resultado.empty()) {
                ultimoResultadoValido = resultado.clone();
            }

            fimNormalVideo = false;

            cv::Mat original;
            if (!quadroReinicializacaoPendente.empty()) {
                original = quadroReinicializacaoPendente.clone();
                quadroReinicializacaoPendente.release();
            } else {
                try {
                    if (fontefileimagem && !leitor.isOpened()) {
                        if (!imagemEstaticaConsumida && !imagem.empty()) {
                            original = imagem.clone();
                            imagemEstaticaConsumida = true;
                        } else {
                            original.release();
                        }
                    } else {
                        leitor >> original;
                    }
                } catch (const cv::Exception& e) {
                    std::cerr << "Erro OpenCV ao ler quadro: " << e.what() << std::endl;
                    original.release();
                } catch (...) {
                    original.release();
                }
            }

            if (original.empty()) {
                quadrosVaziosConsecutivos++;

                if (fontefileimagem) {
                    falhaLeituraAtual = false;
                    encerrarPorLimiteLeitura = true;

                    if (!ultimoResultadoValido.empty()) {
                        resultado = ultimoResultadoValido;
                        larguraOriginalComposicao =
                            larguraOriginalUltimoValidoComposicao;
                    }
                }

                double totalFrames = 0;
                double currentFrame = 0;

                try {
                    totalFrames = leitor.get(cv::CAP_PROP_FRAME_COUNT);
                    currentFrame = leitor.get(cv::CAP_PROP_POS_FRAMES);
                } catch (...) {
                }

                const bool metadadosValidos =
                    std::isfinite(totalFrames) &&
                    std::isfinite(currentFrame) &&
                    totalFrames > 0 &&
                    currentFrame > 0;

                const bool fimPorMetadados =
                    fontefilevideo &&
                    metadadosValidos &&
                    currentFrame >= totalFrames;

                if (fimPorMetadados) {
                    fimNormalVideo = true;
                    encerrarPorLimiteLeitura = true;

                    if (!ultimoResultadoValido.empty()) {
                        resultado = ultimoResultadoValido;
                        larguraOriginalComposicao =
                            larguraOriginalUltimoValidoComposicao;
                    }
                } else if (!fontefileimagem) {
                    falhaLeituraAtual = true;

                    if (!ultimoResultadoValido.empty()) {
                        resultado = ultimoResultadoValido;
                        larguraOriginalComposicao = larguraOriginalUltimoValidoComposicao;
                        composicaoDesatualizadaPorFalha = true;
                    }

                    if (quadrosVaziosConsecutivos >= 30) {
                        encerrarPorLimiteLeitura = true;
                    }
                }
            } else {
                bool falhaProcessamentoNestaIteracao = false;
                quadrosVaziosConsecutivos = 0;
                falhaLeituraAtual = false;
                encerrarPorLimiteLeitura = false;

                cv::Mat quadroBrutoOriginal = original.clone();
                historicoRemoverAntesDoQuadro = frameAnterior.clone();
                if (quadroOriginalFonte.empty()) {
                    quadroOriginalFonte = quadroBrutoOriginal.clone();
                }

                int origChannels = original.channels();
                if (origChannels != 1 && origChannels != 3 && origChannels != 4) {
                    falhaProcessamentoNestaIteracao = true;
                }

                if (!falhaProcessamentoNestaIteracao) {
                    cv::Mat processado = original.clone();

                    for (size_t i = 0; i < sequencia.size(); ++i) {
                        try {
                            processado = aplicarFiltro(processado, sequencia[i], filtros[i]);
                        } catch (const cv::Exception& e) {
                            std::cerr << "Erro OpenCV no filtro " << sequencia[i] << "): " << e.what() << std::endl;
                            processado.release();
                        } catch (...) {
                            std::cerr << "Erro desconhecido no filtro " << sequencia[i] << std::endl;
                            processado.release();
                        }

                        if (processado.empty()) {
                            falhaProcessamentoNestaIteracao = true;
                            break;
                        }
                    }

                    if (!processado.empty()) {
                        int procChannels = processado.channels();
                        if (procChannels != 1 && procChannels != 3 && procChannels != 4) {
                            falhaProcessamentoNestaIteracao = true;
                        }

                        if (!falhaProcessamentoNestaIteracao) {
                            try {
                                if (original.channels() == 4) cv::cvtColor(original, original, cv::COLOR_BGRA2BGR);
                                else if (original.channels() == 1) cv::cvtColor(original, original, cv::COLOR_GRAY2BGR);

                                if (processado.channels() == 4) cv::cvtColor(processado, processado, cv::COLOR_BGRA2BGR);
                                else if (processado.channels() == 1) cv::cvtColor(processado, processado, cv::COLOR_GRAY2BGR);

                                if (original.rows > 0 && processado.rows > 0 && original.size() != processado.size()) {
                                    int novaLargura = std::max(1, static_cast<int>(original.cols * (static_cast<double>(processado.rows) / original.rows)));
                                    cv::resize(original, original, cv::Size(novaLargura, processado.rows), 0, 0, cv::INTER_LINEAR);
                                }

                                if (original.rows == processado.rows && original.channels() == processado.channels() && original.depth() == processado.depth()) {
                                    const int larguraCandidata = original.cols;

                                    if (original.empty() || processado.empty() ||
                                        original.cols <= 0 || original.rows <= 0 ||
                                        processado.cols <= 0 || processado.rows <= 0) {
                                        falhaProcessamentoNestaIteracao = true;
                                    } else {
                                        cv::Mat resultadoTemp;
                                        cv::hconcat(original, processado, resultadoTemp);

                                        if (!resultadoTemp.empty()) {
                                            resultado = resultadoTemp;
                                            larguraOriginalComposicao = larguraCandidata;
                                            larguraOriginalUltimoValidoComposicao = larguraCandidata;

                                            quadroOriginalFonte = quadroBrutoOriginal.clone();
                                            quadroBrutoAtual = quadroBrutoOriginal.clone();
                                            imagem = quadroBrutoOriginal.clone();
                                            falhaProcessamentoAtual = false;
                                            composicaoDesatualizadaPorFalha = false;
                                        } else {
                                            falhaProcessamentoNestaIteracao = true;
                                        }
                                    }
                                } else {
                                    falhaProcessamentoNestaIteracao = true;
                                }
                            } catch (const cv::Exception& e) {
                                std::cerr << "Erro OpenCV na composicao: " << e.what() << std::endl;
                                falhaProcessamentoNestaIteracao = true;
                            } catch (...) {
                                std::cerr << "Erro desconhecido na composicao." << std::endl;
                                falhaProcessamentoNestaIteracao = true;
                            }
                        }
                    } else {
                        falhaProcessamentoNestaIteracao = true;
                    }
                }

                if (falhaProcessamentoNestaIteracao) {
                    falhaProcessamentoAtual = true;
                    composicaoDesatualizadaPorFalha = true;

                    if (!ultimoResultadoValido.empty()) {
                        resultado = ultimoResultadoValido;
                        larguraOriginalComposicao = larguraOriginalUltimoValidoComposicao;
                    }
                }
            }
        }

        cv::Mat painelExibicao;
        bool erroGeracaoPainelNestaIteracao = false;
        if (resultado.empty()) {
            try {
                constexpr int altPl = 150;
                constexpr int largPl = 640;
                cv::Mat placeholder(altPl, largPl, CV_8UC3, cv::Scalar(35, 35, 35));

                std::string textoPlaceholder = "AGUARDANDO DADOS DA CAPTURA...";

                if (fimNormalVideo) {
                    textoPlaceholder = "FIM NORMAL DO VIDEO";
                } else if (fontefileimagem && encerrarPorLimiteLeitura) {
                    textoPlaceholder = "FALHA NA LEITURA DA IMAGEM";
                } else if (encerrarPorLimiteLeitura) {
                    textoPlaceholder = "FALHA: LIMITE DE LEITURA ATINGIDO";
                } else if (falhaProcessamentoAtual) {
                    textoPlaceholder = "FALHA NO PROCESSAMENTO DOS FILTROS...";
                } else if (quadrosVaziosConsecutivos > 0) {
                    textoPlaceholder = "FALHA NA LEITURA DA CAPTURA...";
                }

                cv::putText(placeholder, textoPlaceholder, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(0, 165, 255), 2);
                painelExibicao = placeholder;
            } catch (const cv::Exception& e) {
                std::cerr << "Erro OpenCV ao gerar placeholder: " << e.what() << std::endl;
                erroGeracaoPainelNestaIteracao = true;
            } catch (...) {
                std::cerr << "Erro desconhecido na geracao do placeholder." << std::endl;
                erroGeracaoPainelNestaIteracao = true;
            }
        } else {
            const char* nomeFiltro = "Filtro: nenhum";
            const char* descricaoFiltro = "Sem filtros aplicados.";

            if (!sequencia.empty() && filtroAtual < sequencia.size()) {
                switch (sequencia[filtroAtual]) {
                case 1: nomeFiltro = "Filtro: Girar"; descricaoFiltro = "Rotacao e espelhamento da imagem."; break;
                case 2: nomeFiltro = "Filtro: Recortar"; descricaoFiltro = "Remove areas das bordas da imagem."; break;
                case 3: nomeFiltro = "Filtro: Granular"; descricaoFiltro = "Adiciona granulos preservando bordas detectadas."; break;
                case 4: nomeFiltro = "Filtro: Nitidez"; descricaoFiltro = "Realca detalhes sem aplicar nitidez nas bordas protegidas."; break;
                case 5: nomeFiltro = "Filtro: Desfocar"; descricaoFiltro = "Suaviza a imagem preservando bordas protegidas."; break;
                case 6: nomeFiltro = "Filtro: Remover"; descricaoFiltro = "Detecta e tenta remover pequenas falhas."; break;
                case 7: nomeFiltro = "Filtro: Limpar"; descricaoFiltro = "Reduz ruido preservando detalhes."; break;
                case 8: nomeFiltro = "Filtro: Brilho"; descricaoFiltro = "Ajusta a luminosidade da imagem."; break;
                case 9: nomeFiltro = "Filtro: Contraste"; descricaoFiltro = "Ajusta a diferenca entre tons claros e escuros."; break;
                case 10: nomeFiltro = "Filtro: Cores"; descricaoFiltro = "Ajusta a intensidade de um canal de cor."; break;
                case 11: nomeFiltro = "Filtro: Cinzas"; descricaoFiltro = "Converte a imagem para tons de cinza."; break;
                default: nomeFiltro = "Filtro desconhecido"; descricaoFiltro = "Operacao nao identificada."; break;
                }
            }

            try {
                constexpr int alturaPainel = 150;
                cv::Mat painel(resultado.rows + alturaPainel, resultado.cols, resultado.type(), cv::Scalar(35, 35, 35));
                resultado.copyTo(painel(cv::Rect(0, 0, resultado.cols, resultado.rows)));

                int larguraProcEfetiva = resultado.cols - larguraOriginalComposicao;
                int centroOriginal = larguraOriginalComposicao > 0 ? larguraOriginalComposicao / 2 : resultado.cols / 4;
                int centroProcessada = larguraOriginalComposicao > 0 ? larguraOriginalComposicao + (larguraProcEfetiva / 2) : (resultado.cols * 3) / 4;

                cv::putText(painel, "ORIGINAL", cv::Point(std::max(10, centroOriginal - 50), resultado.rows + 28), cv::FONT_HERSHEY_SIMPLEX, 0.60, cv::Scalar(255, 255, 255), 2);

                cv::Scalar corStatusProcessada = (falhaProcessamentoAtual || falhaLeituraAtual || composicaoDesatualizadaPorFalha) ? cv::Scalar(0, 165, 255) : cv::Scalar(255, 255, 255);
                std::string labelProcessada = "PROCESSADO";

                if (fimNormalVideo) {
                    labelProcessada = "FIM DO VIDEO [CONGELADO - AJUSTES SUSPENSOS]";
                    corStatusProcessada = cv::Scalar(200, 200, 200);
                } else if (encerrarPorLimiteLeitura) {
                    labelProcessada = "CONGELADO (LIMITE - AJUSTES SUSPENSOS)";
                    corStatusProcessada = cv::Scalar(0, 165, 255);
                } else if (falhaLeituraAtual) {
                    labelProcessada = "TENTANDO RECONECTAR LEITURA...";
                    corStatusProcessada = cv::Scalar(255, 165, 0);
                } else if (falhaProcessamentoAtual) {
                    labelProcessada = "ATENCAO: FALHA NO FILTRO ATUAL (TENTANDO RECUPERAR)";
                    corStatusProcessada = cv::Scalar(0, 165, 255);
                } else if (composicaoDesatualizadaPorFalha) {
                    labelProcessada = "ATENCAO: PARAMETROS DIVERGENTES (AGUARDANDO NOVO QUADRO)";
                    corStatusProcessada = cv::Scalar(0, 165, 255);
                }
                cv::putText(painel, labelProcessada, cv::Point(std::max(larguraOriginalComposicao + 10, centroProcessada - 280), resultado.rows + 28), cv::FONT_HERSHEY_SIMPLEX, 0.32, corStatusProcessada, 2);

                cv::putText(painel, nomeFiltro, cv::Point(15, resultado.rows + 58), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(255, 255, 255), 1);
                cv::putText(painel, descricaoFiltro, cv::Point(15, resultado.rows + 82), cv::FONT_HERSHEY_SIMPLEX, 0.48, cv::Scalar(220, 220, 220), 1);
                cv::putText(painel, "[/] [*] filtro   [T] parametro   [+] [-] ajustar", cv::Point(15, resultado.rows + 106), cv::FONT_HERSHEY_SIMPLEX, 0.48, cv::Scalar(220, 220, 220), 1);
                cv::putText(painel, "[Z] remover selecionado   [P] pausar   [R] reiniciar   [C] ou [ESC] sair", cv::Point(15, resultado.rows + 130), cv::FONT_HERSHEY_SIMPLEX, 0.48, cv::Scalar(220, 220, 220), 1);

                const int telaLargura = GetSystemMetrics(SM_CXSCREEN);
                const int telaAltura = GetSystemMetrics(SM_CYSCREEN);
                const int larguraMaxima = std::max(1, static_cast<int>(telaLargura * 0.90));
                const int alturaMaxima = std::max(1, static_cast<int>(telaAltura * 0.80));

                const double escalaLargura = static_cast<double>(larguraMaxima) / static_cast<double>(painel.cols);
                const double escalaAltura = static_cast<double>(alturaMaxima) / static_cast<double>(painel.rows);
                const double escala = std::min(escalaLargura, escalaAltura);

                if (escala < 1.0) {
                    cv::resize(painel, painelExibicao, cv::Size(), escala, escala, cv::INTER_AREA);
                } else {
                    painelExibicao = painel;
                }
            } catch (const cv::Exception& e) {
                std::cerr << "Erro OpenCV na composicao/exibicao do painel: " << e.what() << std::endl;
                erroGeracaoPainelNestaIteracao = true;
            } catch (...) {
                std::cerr << "Erro desconhecido na composicao ou exibicao do painel." << std::endl;
                erroGeracaoPainelNestaIteracao = true;
            }
        }

        bool erroExibicaoNestaIteracao = erroGeracaoPainelNestaIteracao;
        if (!painelExibicao.empty()) {
            try {
                cv::resizeWindow("Comparacao Lado a Lado", painelExibicao.cols, painelExibicao.rows);
                cv::imshow("Comparacao Lado a Lado", painelExibicao);
            } catch (const cv::Exception& e) {
                std::cerr << "Erro OpenCV em imshow/resizeWindow: " << e.what() << std::endl;
                erroExibicaoNestaIteracao = true;
            } catch (...) {
                std::cerr << "Erro desconhecido em imshow/resizeWindow." << std::endl;
                erroExibicaoNestaIteracao = true;
            }
        }

        int tecla = -1;
        bool erroWaitKeyNestaIteracao = false;
        try {
            tecla = cv::waitKey(encerrarPorLimiteLeitura ? 100 : 30) & 0xFF;
        } catch (const cv::Exception& e) {
            std::cerr << "Erro OpenCV em waitKey: " << e.what() << std::endl;
            erroWaitKeyNestaIteracao = true;
            tecla = -1;
        } catch (...) {
            std::cerr << "Erro desconhecido em waitKey." << std::endl;
            erroWaitKeyNestaIteracao = true;
            tecla = -1;
        }

        if (erroExibicaoNestaIteracao || erroWaitKeyNestaIteracao) {
            errosExibicaoConsecutivos++;
        } else {
            errosExibicaoConsecutivos = 0;
        }

        if (errosExibicaoConsecutivos >= 15) {
            std::cerr << "Erro crítico: 15 falhas consecutivas no subsistema gráfico/waitKey. Encerrando comparação." << std::endl;
            break;
        }

        if (tecla == 'c' || tecla == 'C' || tecla == 27) {
            break;
        }
        else if (tecla == 'p' || tecla == 'P') {
            if (!encerrarPorLimiteLeitura && !fontefileimagem) {
                pausado = !pausado;
            }
        }
        else if ((tecla == 'r' || tecla == 'R') && fontefilevideo) {
            try {
                quadroReinicializacaoPendente.release();
                bool reiniciadoComSucesso = false;

                try {
                    if (leitor.set(cv::CAP_PROP_POS_FRAMES, 0)) {
                        cv::Mat testeQuadro;
                        if (leitor.read(testeQuadro) && !testeQuadro.empty()) {
                            quadroReinicializacaoPendente = testeQuadro.clone();
                            reiniciadoComSucesso = true;
                        }
                    }
                } catch (...) {
                    reiniciadoComSucesso = false;
                    quadroReinicializacaoPendente.release();
                }

                if (reiniciadoComSucesso) {
                    frameAnterior.release();
                    pausado = false;
                    quadrosVaziosConsecutivos = 0;
                    errosExibicaoConsecutivos = 0;
                    falhaProcessamentoAtual = false;
                    falhaLeituraAtual = false;
                    fimNormalVideo = false;
                    encerrarPorLimiteLeitura = false;
                    composicaoDesatualizadaPorFalha = true;

                    quadroBrutoAtual = quadroReinicializacaoPendente.clone();
                    quadroOriginalFonte = quadroBrutoAtual.clone();
                    imagem = quadroBrutoAtual.clone();
                } else {
                    quadroReinicializacaoPendente.release();

                    falhaLeituraAtual = true;
                    fimNormalVideo = false;
                    encerrarPorLimiteLeitura = false;
                    quadrosVaziosConsecutivos = 0;

                    std::cerr << "Falha ao reiniciar ou obter um novo quadro da fonte."
                              << std::endl;
                }
            } catch (const cv::Exception& e) {
                quadroReinicializacaoPendente.release();
                falhaLeituraAtual = true;
                fimNormalVideo = false;
                encerrarPorLimiteLeitura = false;
                quadrosVaziosConsecutivos = 0;

                std::cerr << "Erro OpenCV ao reiniciar: " << e.what() << std::endl;
            } catch (...) {
                quadroReinicializacaoPendente.release();
                falhaLeituraAtual = true;
                fimNormalVideo = false;
                encerrarPorLimiteLeitura = false;
                quadrosVaziosConsecutivos = 0;

                std::cerr << "Erro desconhecido ao reiniciar a midia." << std::endl;
            }
        }
        else {
            const bool estadoCongeladoInteracao = (encerrarPorLimiteLeitura && !fontefileimagem) || fimNormalVideo;

            if (!estadoCongeladoInteracao) {
                bool alterouConfiguracao = false;

                if (tecla == 'z' || tecla == 'Z') {
                    if (!sequencia.empty() && filtroAtual < sequencia.size()) {
                        sequencia.erase(sequencia.begin() + static_cast<std::vector<int>::difference_type>(filtroAtual));
                        filtros.erase(filtros.begin() + static_cast<std::vector<Parametros>::difference_type>(filtroAtual));

                        parametroAtual = 0;

                        if (sequencia.empty()) {
                            filtroAtual = 0;
                        } else if (filtroAtual >= sequencia.size()) {
                            filtroAtual = sequencia.size() - 1;
                        }
                        alterouConfiguracao = true;
                    }
                }
                else if (tecla == '/' || tecla == '*') {
                    if (!sequencia.empty()) {
                        if (tecla == '/') {
                            if (filtroAtual > 0) --filtroAtual;
                        } else {
                            if (filtroAtual + 1 < sequencia.size()) ++filtroAtual;
                        }
                        parametroAtual = 0;
                    }
                }
                else if (tecla == 't' || tecla == 'T') {
                    if (!sequencia.empty()) {
                        const int escolha = sequencia[filtroAtual];
                        int quantidadeParametros = 0;
                        switch (escolha) {
                        case 1: case 2: case 10: quantidadeParametros = 2; break;
                        case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 11: quantidadeParametros = 1; break;
                        default: quantidadeParametros = 0; break;
                        }
                        if (quantidadeParametros > 0) {
                            ++parametroAtual;
                            if (parametroAtual >= quantidadeParametros) parametroAtual = 0;
                        }
                    }
                }
                else if (tecla == '+' || tecla == '=' || tecla == '-') {
                    if (!sequencia.empty() && filtroAtual < sequencia.size()) {
                        const int escolha = sequencia[filtroAtual];
                        int parametro = -1;
                        switch (escolha) {
                        case 1: parametro = parametroAtual == 0 ? 0 : 2; break;
                        case 2: parametro = parametroAtual == 0 ? 2 : 3; break;
                        case 3: case 4: case 6: case 8: case 9: case 11: parametro = parametroAtual == 0 ? 0 : -1; break;
                        case 5: parametro = 2; break;
                        case 7: parametro = 1; break;
                        case 10: parametro = parametroAtual == 0 ? 0 : 2; break;
                        default: parametro = -1; break;
                        }

                        if (parametro != -1) {
                            const double passo = tecla == '-' ? -1.0 : 1.0;
                            if (parametro == 0) {
                                double valorAntigo = filtros[filtroAtual].alfa;
                                filtros[filtroAtual].alfa = std::clamp(valorAntigo + passo, -255.0, 255.0);
                                if (filtros[filtroAtual].alfa != valorAntigo) alterouConfiguracao = true;
                            } else if (parametro == 1) {
                                float valorAntigo = filtros[filtroAtual].beta;
                                filtros[filtroAtual].beta = std::clamp(valorAntigo + static_cast<float>(passo), -100.0f, 100.0f);
                                if (filtros[filtroAtual].beta != valorAntigo) alterouConfiguracao = true;
                            } else if (parametro == 2) {
                                int valorAntigo = filtros[filtroAtual].gama;
                                int passoInt = static_cast<int>(passo);

                                long long valorNovo = std::clamp(
                                    static_cast<long long>(valorAntigo) + passoInt,
                                    -1000LL,
                                    1000LL
                                );

                                filtros[filtroAtual].gama = static_cast<int>(valorNovo);

                                if (filtros[filtroAtual].gama != valorAntigo)
                                    alterouConfiguracao = true;
                            } else if (parametro == 3) {
                                int valorAntigo = filtros[filtroAtual].delta;
                                int passoInt = static_cast<int>(passo);

                                long long valorNovo = std::clamp(
                                    static_cast<long long>(valorAntigo) + passoInt,
                                    -1000LL,
                                    1000LL
                                );

                                filtros[filtroAtual].delta = static_cast<int>(valorNovo);

                                if (filtros[filtroAtual].delta != valorAntigo)
                                    alterouConfiguracao = true;
                            }
                        }
                    }
                }

                if (alterouConfiguracao) {
                    if (pausado || fontefileimagem || resultado.empty()) {
                        reprocessarQuadroAtual();
                    } else {
                        composicaoDesatualizadaPorFalha = true;
                    }
                }
            }
        }
    }

    try {
        cv::destroyWindow("Comparacao Lado a Lado");
    } catch (...) {
    }

    ultimaSequencia = sequencia;
    ultimosFiltros = filtros;

    return resultado;
}

cv::Mat Render::girar(const cv::Mat& arquivo, double alfa, int gama) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa)) {
        return arquivo.clone();
    }

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    alfa = std::remainder(alfa, 360.0);

    cv::Mat mocao = cv::getRotationMatrix2D(
        cv::Point2f(static_cast<float>(arquivo.cols - 1) / 2.0f,
                    static_cast<float>(arquivo.rows - 1) / 2.0f),
        alfa,
        1.0
    );

    const auto w = static_cast<double>(arquivo.cols);
    const auto h = static_cast<double>(arquivo.rows);
    cv::Point2d cantos[] = {
        {-0.5, -0.5},
        {w - 0.5, -0.5},
        {-0.5, h - 0.5},
        {w - 0.5, h - 0.5}
    };

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();

    for (const cv::Point2d& canto : cantos) {
        const double x = mocao.at<double>(0, 0) * canto.x +
                         mocao.at<double>(0, 1) * canto.y +
                         mocao.at<double>(0, 2);
        const double y = mocao.at<double>(1, 0) * canto.x +
                         mocao.at<double>(1, 1) * canto.y +
                         mocao.at<double>(1, 2);

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }

    const double largura = std::ceil(maxX - minX);
    const double altura = std::ceil(maxY - minY);

    if (!std::isfinite(largura) || !std::isfinite(altura) ||
        largura < 1.0 || altura < 1.0 ||
        largura > static_cast<double>(std::numeric_limits<int>::max()) ||
        altura > static_cast<double>(std::numeric_limits<int>::max())) {
        return arquivo.clone();
    }

    const int novaLargura = static_cast<int>(largura);
    const int novaAltura = static_cast<int>(altura);

    mocao.at<double>(0, 2) -= minX;
    mocao.at<double>(1, 2) -= minY;

    cv::Scalar corBorda;
    if (canaisOriginais == 1) {
        corBorda = cv::Scalar(255);
    } else if (canaisOriginais == 3) {
        corBorda = cv::Scalar(255, 255, 255);
    } else {
        corBorda = cv::Scalar(255, 255, 255, 255);
    }

    cv::Mat processada;

    cv::warpAffine(
        arquivo,
        processada,
        mocao,
        cv::Size(novaLargura, novaAltura),
        cv::INTER_LINEAR,
        cv::BORDER_CONSTANT,
        corBorda
    );

    if (gama == 1 || gama == 0 || gama == -1) {
        cv::flip(processada, processada, gama);
    }

    return processada;
}

void Render::recorte(int x, int y) {
    esquerda = x;
    topo = y;
}

cv::Mat Render::recortar(const cv::Mat& arquivo, int gama, int delta) {
    if (arquivo.empty() || gama <= 0 || delta <= 0) {
        return arquivo.clone();
    }

    if (esquerda < 0 || topo < 0 ||
        esquerda >= arquivo.cols || topo >= arquivo.rows) {
        return arquivo.clone();
    }

    const int largura = std::min(gama, arquivo.cols - esquerda);
    const int altura = std::min(delta, arquivo.rows - topo);

    if (largura <= 0 || altura <= 0) {
        return arquivo.clone();
    }

    return arquivo(cv::Rect(esquerda, topo, largura, altura)).clone();
}

cv::Mat Render::granular(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || !std::isfinite(alfa) ||
        arquivo.depth() != CV_8U ||
        arquivo.channels() < 1 || arquivo.channels() > 4) {
        return arquivo.clone();
    }

    alfa = std::clamp(alfa, 0.0, 100.0);

    if (alfa == 0.0) {
        return arquivo.clone();
    }

    const int canaisCor =
        (arquivo.channels() == 2 || arquivo.channels() == 4)
        ? arquivo.channels() - 1
        : arquivo.channels();

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(arquivo.size(), CV_8UC1);

    if (borda && limite > 0 &&
        std::isfinite(desvio) && desvio >= 0.0) {

        auto estatisticas = [&](const cv::Mat& regiao,
                                std::vector<double>& medias,
                                double& maiorDesvio) {
            std::vector<cv::Mat> canais;
            cv::split(regiao, canais);

            medias.resize(canaisCor);
            maiorDesvio = 0.0;

            for (int i = 0; i < canaisCor; ++i) {
                cv::Scalar media;
                cv::Scalar desvioCanal;

                cv::meanStdDev(canais[i], media, desvioCanal);

                medias[i] = media[0];
                maiorDesvio = std::max(
                    maiorDesvio, desvioCanal[0]
                );
            }
        };

        auto diferenca = [](const std::vector<double>& a,
                            const std::vector<double>& b) {
            double maior = 0.0;

            if (a.size() != b.size()) {
                return maior;
            }

            for (size_t i = 0; i < a.size(); ++i) {
                maior = std::max(
                    maior, std::abs(a[i] - b[i])
                );
            }

            return maior;
        };

        auto detectar = [&](bool horizontal, bool inicio) {
            const int tamanho = horizontal
                ? arquivo.rows
                : arquivo.cols;

            const int tamanhoMaximo =
                std::min(limite, tamanho / 2);

            if (tamanhoMaximo < 1) {
                return;
            }

            int espessura = 0;
            std::vector<double> mediasAnteriores;

            for (int i = 0; i < tamanhoMaximo; ++i) {
                const int posicao =
                    inicio ? i : tamanho - 1 - i;

                const cv::Rect faixa = horizontal
                    ? cv::Rect(0, posicao, arquivo.cols, 1)
                    : cv::Rect(posicao, 0, 1, arquivo.rows);

                std::vector<double> medias;
                double maiorDesvio = 0.0;

                estatisticas(
                    arquivo(faixa), medias, maiorDesvio
                );

                if (maiorDesvio > desvio ||
                    (i > 0 &&
                     diferenca(medias, mediasAnteriores) > desvio)) {
                    break;
                }

                mediasAnteriores = medias;
                ++espessura;
            }

            if (espessura == 0) {
                return;
            }

            const int posicaoInterior =
                inicio ? espessura : tamanho - espessura - 1;

            if (posicaoInterior < 0 || posicaoInterior >= tamanho) {
                return;
            }

            const cv::Rect regiaoBorda = horizontal
                ? (inicio
                    ? cv::Rect(0, 0, arquivo.cols, espessura)
                    : cv::Rect(0, arquivo.rows - espessura,
                               arquivo.cols, espessura))
                : (inicio
                    ? cv::Rect(0, 0, espessura, arquivo.rows)
                    : cv::Rect(arquivo.cols - espessura, 0,
                               espessura, arquivo.rows));

            const cv::Rect faixaInterna = horizontal
                ? cv::Rect(0, posicaoInterior, arquivo.cols, 1)
                : cv::Rect(posicaoInterior, 0, 1, arquivo.rows);

            const int posicaoBorda =
                inicio ? espessura - 1 : tamanho - espessura;

            const cv::Rect ultimaFaixa = horizontal
                ? cv::Rect(0, posicaoBorda, arquivo.cols, 1)
                : cv::Rect(posicaoBorda, 0, 1, arquivo.rows);

            std::vector<double> mediasBorda;
            std::vector<double> mediasInternas;
            double desvioBorda = 0.0;
            double desvioInterno = 0.0;

            estatisticas(
                arquivo(ultimaFaixa), mediasBorda, desvioBorda
            );

            estatisticas(
                arquivo(faixaInterna), mediasInternas, desvioInterno
            );

            if (diferenca(mediasBorda, mediasInternas) > desvio ||
                desvioInterno > desvioBorda + desvio) {
                mascaraProtecao(regiaoBorda).setTo(255);
            }
        };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat mascaraGranulado;
    cv::bitwise_not(mascaraProtecao, mascaraGranulado);

    std::vector<cv::Mat> canais;
    cv::split(arquivo, canais);

    for (int i = 0; i < canaisCor; ++i) {
        cv::Mat ruido(arquivo.size(), CV_16SC1);
        cv::randn(ruido, cv::Scalar(0), cv::Scalar(alfa));

        cv::Mat ruidoAplicavel =
            cv::Mat::zeros(arquivo.size(), CV_16SC1);

        ruido.copyTo(ruidoAplicavel, mascaraGranulado);

        cv::Mat canal16;
        canais[i].convertTo(canal16, CV_16S);

        cv::add(canal16, ruidoAplicavel, canal16);
        canal16.convertTo(canais[i], CV_8U);
    }

    cv::Mat resultadoGranulado;
    cv::merge(canais, resultadoGranulado);

    return resultadoGranulado;
}

cv::Mat Render::nitidez(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa)) {
        return arquivo.clone();
    }

    alfa = std::max(0.0, std::min(100.0, alfa));
    alfa /= 12.5;

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    if (alfa == 0.0) {
        return arquivo.clone();
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> canaisBGRA;
        cv::split(arquivo, canaisBGRA);
        canalAlpha = canaisBGRA[3].clone();

        cv::Mat bgrTemp;
        std::vector<cv::Mat> bgrIniciais = { canaisBGRA[0], canaisBGRA[1], canaisBGRA[2] };
        cv::merge(bgrIniciais, bgrTemp);

        cv::Mat mascaraTransparente = (canalAlpha == 0);
        if (cv::countNonZero(mascaraTransparente) > 0 && cv::countNonZero(~mascaraTransparente) > 0) {
            cv::inpaint(bgrTemp, mascaraTransparente, bgrTemp, 3.0, cv::INPAINT_TELEA);
        }

        imagemProcessar = bgrTemp;
    }

    cv::Mat floatMat;
    std::vector<cv::Mat> canaisYCrCb;
    cv::Mat ycrcb;

    if (imagemProcessar.channels() == 3) {
        cv::cvtColor(imagemProcessar, ycrcb, cv::COLOR_BGR2YCrCb);
        cv::split(ycrcb, canaisYCrCb);
        canaisYCrCb[0].convertTo(floatMat, CV_32F);
    } else {
        imagemProcessar.convertTo(floatMat, CV_32F);
    }

    cv::Mat epsilonFloat, nitidezFloat;
    cv::GaussianBlur(floatMat, epsilonFloat, cv::Size(3, 3), 0, 0, cv::BORDER_REFLECT);
    cv::addWeighted(floatMat, 1.0 + alfa, epsilonFloat, -alfa, 0, nitidezFloat);

    cv::Mat minLocal, maxLocal, kernelMorph;
    kernelMorph = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::erode(floatMat, minLocal, kernelMorph, cv::Point(-1,-1), 1, cv::BORDER_REFLECT);
    cv::dilate(floatMat, maxLocal, kernelMorph, cv::Point(-1,-1), 1, cv::BORDER_REFLECT);

    for (int r = 0; r < nitidezFloat.rows; ++r) {
        const float* pMin = minLocal.ptr<float>(r);
        const float* pMax = maxLocal.ptr<float>(r);
        auto* pNit = nitidezFloat.ptr<float>(r);

        for (int c = 0; c < nitidezFloat.cols; ++c) {
            float amplitudeLocal = pMax[c] - pMin[c];
            float margem = std::max(1.0f, std::min(15.0f, amplitudeLocal * 0.1f));
            pNit[c] = std::min(pMax[c] + margem, std::max(pMin[c] - margem, pNit[c]));
        }
    }

    cv::Mat nitidezResultado;
    if (imagemProcessar.channels() == 3) {
        nitidezFloat.convertTo(canaisYCrCb[0], CV_8U);
        cv::merge(canaisYCrCb, ycrcb);
        cv::Mat bgrTemp;
        cv::cvtColor(ycrcb, bgrTemp, cv::COLOR_YCrCb2BGR);
        bgrTemp.copyTo(nitidezResultado);
    } else {
        nitidezFloat.convertTo(nitidezResultado, CV_8U);
    }

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> saidaCanais;
        cv::split(nitidezResultado, saidaCanais);

        std::vector<cv::Mat> bgraOriginal;
        cv::split(arquivo, bgraOriginal);
        for (int i = 0; i < 3; ++i) {
            bgraOriginal[i].copyTo(saidaCanais[i], canalAlpha == 0);
        }
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, nitidezResultado);
    }

    return nitidezResultado;
}

cv::Mat Render::desfocar(const cv::Mat& arquivo, int gama) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    int maxDim = std::min(arquivo.rows, arquivo.cols);
    int ksize = std::min(gama, maxDim);
    if (ksize <= 0) {
        return arquivo.clone();
    }

    if (ksize % 2 == 0) {
        if (ksize + 1 <= maxDim) {
            ++ksize;
        } else if (ksize > 1) {
            --ksize;
        } else {
            ksize = 1;
        }
    }

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> canaisBGRA;
        cv::split(arquivo, canaisBGRA);
        canalAlpha = canaisBGRA[3].clone();

        cv::Mat bgrTemp;
        std::vector<cv::Mat> bgrIniciais = { canaisBGRA[0], canaisBGRA[1], canaisBGRA[2] };
        cv::merge(bgrIniciais, bgrTemp);

        cv::Mat mascaraTransparente = (canalAlpha == 0);
        if (cv::countNonZero(mascaraTransparente) > 0 && cv::countNonZero(~mascaraTransparente) > 0) {
            cv::inpaint(bgrTemp, mascaraTransparente, bgrTemp, 3.0, cv::INPAINT_TELEA);
        }

        imagemProcessar = bgrTemp;
    }

    cv::Mat resultadoDesfocado;
    cv::GaussianBlur(imagemProcessar, resultadoDesfocado, cv::Size(ksize, ksize), 0);

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> saidaCanais;
        cv::split(resultadoDesfocado, saidaCanais);

        std::vector<cv::Mat> bgraOriginal;
        cv::split(arquivo, bgraOriginal);
        for (int i = 0; i < 3; ++i) {
            bgraOriginal[i].copyTo(saidaCanais[i], canalAlpha == 0);
        }
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, resultadoDesfocado);
    }

    return resultadoDesfocado;
}

cv::Mat Render::remover(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa) || !std::isfinite(desvio) || desvio < 0.0) {
        return arquivo.clone();
    }

    double sensibilidade = alfa;
    if (sensibilidade <= 0.0) {
        sensibilidade = 40.0;
    } else if (sensibilidade > 254.0) {
        sensibilidade = 254.0;
    }

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    cv::Mat mascaraProtecao = cv::Mat::zeros(arquivo.size(), CV_8UC1);

    if (borda && limite > 0) {
        auto estatisticas = [&](const cv::Mat& regiao, std::vector<double>& medias, double& maiorDesvio, bool& valido, bool& totalmenteTransparente) {
            valido = true;
            totalmenteTransparente = false;
            std::vector<cv::Mat> canais;
            cv::Mat mascaraValida;

            if (regiao.channels() == 4) {
                std::vector<cv::Mat> bgra;
                cv::split(regiao, bgra);

                mascaraValida = (bgra[3] > 0);

                if (cv::countNonZero(mascaraValida) == 0) {
                    totalmenteTransparente = true;
                    medias.assign(3, 0.0);
                    maiorDesvio = 0.0;
                    valido = true;
                    return;
                }

                std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
                cv::Mat baseRegiao;
                cv::merge(bgr, baseRegiao);
                cv::split(baseRegiao, canais);
            } else {
                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                } else {
                    cv::split(regiao, canais);
                }
            }

            medias.resize(canais.size());
            maiorDesvio = 0.0;

            for (size_t i = 0; i < canais.size(); ++i) {
                cv::Scalar media;
                cv::Scalar desvioCanal;

                if (!mascaraValida.empty()) {
                    cv::meanStdDev(canais[i], media, desvioCanal, mascaraValida);
                } else {
                    cv::meanStdDev(canais[i], media, desvioCanal);
                }

                medias[i] = media[0];
                maiorDesvio = std::max(maiorDesvio, desvioCanal[0]);
            }
        };

        auto diferenca = [](const std::vector<double>& a, const std::vector<double>& b) {
            if (a.size() != b.size()) {
                return 0.0;
            }

            double maior = 0.0;

            for (size_t i = 0; i < a.size(); ++i) {
                maior = std::max(maior, std::abs(a[i] - b[i]));
            }

            return maior;
        };

        auto detectar = [&](bool horizontal, bool inicio) {
            const int tamanho = horizontal ? arquivo.rows : arquivo.cols;
            const int tamanhoMaximo = std::min(limite, tamanho / 2);

            if (tamanhoMaximo < 1) {
                return;
            }

            int espessura = 0;
            bool encontrouConteudo = false;
            int paddingExterno = 0;

            for (int i = 0; i < tamanhoMaximo; ++i) {
                cv::Rect faixa;

                if (horizontal) {
                    const int y = inicio ? i : arquivo.rows - 1 - i;
                    faixa = cv::Rect(0, y, arquivo.cols, 1);
                }
                else {
                    const int x = inicio ? i : arquivo.cols - 1 - i;
                    faixa = cv::Rect(x, 0, 1, arquivo.rows);
                }

                std::vector<double> medias;
                double maiorDesvio = 0.0;
                bool valido = true;
                bool totalmenteTransparente = false;

                estatisticas(arquivo(faixa), medias, maiorDesvio, valido, totalmenteTransparente);

                if (!valido) {
                    break;
                }

                if (totalmenteTransparente) {
                    if (!encontrouConteudo) {
                        paddingExterno = i + 1;
                        continue;
                    } else {
                        break;
                    }
                }

                encontrouConteudo = true;

                if (maiorDesvio > desvio) {
                    break;
                }

                ++espessura;
            }

            if (espessura < 1) {
                return;
            }

            const int inicioBordaCoord = inicio ? paddingExterno : tamanho - paddingExterno - espessura;
            const int faixaBordaCoord = inicio ? inicioBordaCoord + espessura - 1 : inicioBordaCoord;
            const int posicaoInterior = inicio ? inicioBordaCoord + espessura : inicioBordaCoord - 1;

            if (posicaoInterior < 0 || posicaoInterior >= tamanho) {
                return;
            }

            cv::Rect regiaoBorda;
            cv::Rect faixaBorda;
            cv::Rect faixaInterna;

            if (horizontal) {
                regiaoBorda = cv::Rect(0, inicioBordaCoord, arquivo.cols, espessura);
                faixaBorda = cv::Rect(0, faixaBordaCoord, arquivo.cols, 1);
                faixaInterna = cv::Rect(0, posicaoInterior, arquivo.cols, 1);
            }
            else {
                regiaoBorda = cv::Rect(inicioBordaCoord, 0, espessura, arquivo.rows);
                faixaBorda = cv::Rect(faixaBordaCoord, 0, 1, arquivo.rows);
                faixaInterna = cv::Rect(posicaoInterior, 0, 1, arquivo.rows);
            }

            std::vector<double> mediasBorda;
            std::vector<double> mediasInternas;
            double desvioBorda = 0.0;
            double desvioInterno = 0.0;
            bool validoBorda = true;
            bool validoInterno = true;
            bool transDummy1 = false;
            bool transDummy2 = false;

            estatisticas(arquivo(faixaBorda), mediasBorda, desvioBorda, validoBorda, transDummy1);
            estatisticas(arquivo(faixaInterna), mediasInternas, desvioInterno, validoInterno, transDummy2);

            if (!validoBorda || !validoInterno || transDummy1 || transDummy2) {
                return;
            }

            const double diferencaCor = diferenca(mediasBorda, mediasInternas);
            const bool transicao = (diferencaCor > desvio);

            if (desvioBorda <= desvio && transicao) {
                mascaraProtecao(regiaoBorda).setTo(255);
            }
        };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat cinza;
    if (canaisOriginais == 3) {
        cv::cvtColor(arquivo, cinza, cv::COLOR_BGR2GRAY);
    }
    else if (canaisOriginais == 4) {
        cv::Mat bgrTemp;
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, bgrTemp);
        cv::cvtColor(bgrTemp, cinza, cv::COLOR_BGR2GRAY);
    }
    else {
        cinza = arquivo.clone();
    }

    cv::Mat gradY;
    cv::Sobel(cinza, gradY, CV_16S, 0, 1, 3);
    cv::convertScaleAbs(gradY, gradY);

    cv::Mat mascara;
    cv::threshold(gradY, mascara, sensibilidade, 255, cv::THRESH_BINARY);

    cv::morphologyEx(
        mascara,
        mascara,
        cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(40, 1))
    );

    if (!frameAnterior.empty() && frameAnterior.size() == cinza.size() && frameAnterior.type() == cinza.type()) {
        cv::Mat diferenca;
        cv::absdiff(cinza, frameAnterior, diferenca);

        cv::threshold(
            diferenca,
            diferenca,
            std::max(8.0, sensibilidade * 0.35),
            255,
            cv::THRESH_BINARY
        );

        cv::morphologyEx(
            diferenca,
            diferenca,
            cv::MORPH_OPEN,
            cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 1))
        );

        cv::Mat mascaraGradienteSuave;
        cv::threshold(gradY, mascaraGradienteSuave, std::max(10.0, sensibilidade * 0.4), 255, cv::THRESH_BINARY);
        cv::Mat diferencaFiltrada;
        cv::bitwise_and(diferenca, mascaraGradienteSuave, diferencaFiltrada);

        cv::bitwise_or(mascara, diferencaFiltrada, mascara);
    }

    int espessura = 3;
    if (sensibilidade < 30.0) {
        espessura = 5;
    }

    cv::dilate(
        mascara,
        mascara,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(espessura, espessura))
    );

    if (borda && limite > 0) {
        mascara.setTo(0, mascaraProtecao);
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        canalAlpha = bgra[3].clone();

        cv::Mat bgrTemp;
        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, bgrTemp);

        cv::Mat mascaraTransparente = (canalAlpha == 0);
        if (cv::countNonZero(mascaraTransparente) > 0 && cv::countNonZero(~mascaraTransparente) > 0) {
            cv::inpaint(bgrTemp, mascaraTransparente, bgrTemp, 3.0, cv::INPAINT_TELEA);
        }

        imagemProcessar = bgrTemp;
    }

    cv::Mat resultadoAtual;
    cv::inpaint(imagemProcessar, mascara, resultadoAtual, static_cast<double>(espessura), cv::INPAINT_NS);

    cv::Mat resultadoRemocao = arquivo.clone();
    if (canaisOriginais == 4) {
        resultadoAtual.setTo(cv::Scalar(0, 0, 0), canalAlpha == 0);
        std::vector<cv::Mat> saidaCanais;
        cv::split(resultadoAtual, saidaCanais);
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, resultadoAtual);
        resultadoAtual.copyTo(resultadoRemocao, mascara);
    } else {
        resultadoAtual.copyTo(resultadoRemocao, mascara);
    }

    frameAnterior = cinza.clone();

    return resultadoRemocao;
}
cv::Mat Render::limpar(const cv::Mat& arquivo, float beta) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(beta) || !std::isfinite(desvio) || desvio < 0.0) {
        return arquivo.clone();
    }

    float intensidade = beta;
    if (intensidade < 0.0f) {
        intensidade = 0.0f;
    } else if (intensidade > 10.0f) {
        intensidade = 10.0f;
    }

    if (intensidade == 0.0f) {
        return arquivo.clone();
    }

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    cv::Mat mascaraProtecao = cv::Mat::zeros(arquivo.size(), CV_8UC1);

    if (borda && limite > 0) {
        auto estatisticas = [&](const cv::Mat& regiao, std::vector<double>& medias, double& maiorDesvio, bool& valido, bool& totalmenteTransparente) {
            valido = true;
            totalmenteTransparente = false;
            std::vector<cv::Mat> canais;
            cv::Mat mascaraValida;

            if (regiao.channels() == 4) {
                std::vector<cv::Mat> bgra;
                cv::split(regiao, bgra);

                mascaraValida = (bgra[3] > 0);

                if (cv::countNonZero(mascaraValida) == 0) {
                    totalmenteTransparente = true;
                    medias.assign(3, 0.0);
                    maiorDesvio = 0.0;
                    valido = true;
                    return;
                }

                std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
                cv::Mat baseRegiao;
                cv::merge(bgr, baseRegiao);
                cv::split(baseRegiao, canais);
            } else {
                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                } else {
                    cv::split(regiao, canais);
                }
            }

            medias.resize(canais.size());
            maiorDesvio = 0.0;

            for (size_t i = 0; i < canais.size(); ++i) {
                cv::Scalar media;
                cv::Scalar desvioCanal;

                if (!mascaraValida.empty()) {
                    cv::meanStdDev(canais[i], media, desvioCanal, mascaraValida);
                } else {
                    cv::meanStdDev(canais[i], media, desvioCanal);
                }

                medias[i] = media[0];
                maiorDesvio = std::max(maiorDesvio, desvioCanal[0]);
            }
        };

        auto diferenca = [](const std::vector<double>& a, const std::vector<double>& b) {
            if (a.size() != b.size()) {
                return 0.0;
            }

            double maior = 0.0;

            for (size_t i = 0; i < a.size(); ++i) {
                maior = std::max(maior, std::abs(a[i] - b[i]));
            }

            return maior;
        };

        auto detectar = [&](bool horizontal, bool inicio) {
            const int tamanho = horizontal ? arquivo.rows : arquivo.cols;
            const int tamanhoMaximo = std::min(limite, tamanho / 2);

            if (tamanhoMaximo < 1) {
                return;
            }

            int espessura = 0;
            bool encontrouConteudo = false;
            int paddingExterno = 0;

            for (int i = 0; i < tamanhoMaximo; ++i) {
                cv::Rect faixa;

                if (horizontal) {
                    const int y = inicio ? i : arquivo.rows - 1 - i;
                    faixa = cv::Rect(0, y, arquivo.cols, 1);
                }
                else {
                    const int x = inicio ? i : arquivo.cols - 1 - i;
                    faixa = cv::Rect(x, 0, 1, arquivo.rows);
                }

                std::vector<double> medias;
                double maiorDesvio = 0.0;
                bool valido = true;
                bool totalmenteTransparente = false;

                estatisticas(arquivo(faixa), medias, maiorDesvio, valido, totalmenteTransparente);

                if (!valido) {
                    break;
                }

                if (totalmenteTransparente) {
                    if (!encontrouConteudo) {
                        paddingExterno = i + 1;
                        continue;
                    } else {
                        break;
                    }
                }

                encontrouConteudo = true;

                if (maiorDesvio > desvio) {
                    break;
                }

                ++espessura;
            }

            if (espessura < 1) {
                return;
            }

            const int inicioBordaCoord = inicio ? paddingExterno : tamanho - paddingExterno - espessura;
            const int faixaBordaCoord = inicio ? inicioBordaCoord + espessura - 1 : inicioBordaCoord;
            const int posicaoInterior = inicio ? inicioBordaCoord + espessura : inicioBordaCoord - 1;

            if (posicaoInterior < 0 || posicaoInterior >= tamanho) {
                return;
            }

            cv::Rect regiaoBorda;
            cv::Rect faixaBorda;
            cv::Rect faixaInterna;

            if (horizontal) {
                regiaoBorda = cv::Rect(0, inicioBordaCoord, arquivo.cols, espessura);
                faixaBorda = cv::Rect(0, faixaBordaCoord, arquivo.cols, 1);
                faixaInterna = cv::Rect(0, posicaoInterior, arquivo.cols, 1);
            }
            else {
                regiaoBorda = cv::Rect(inicioBordaCoord, 0, espessura, arquivo.rows);
                faixaBorda = cv::Rect(faixaBordaCoord, 0, 1, arquivo.rows);
                faixaInterna = cv::Rect(posicaoInterior, 0, 1, arquivo.rows);
            }

            std::vector<double> mediasBorda;
            std::vector<double> mediasInternas;
            double desvioBorda = 0.0;
            double desvioInterno = 0.0;
            bool validoBorda = true;
            bool validoInterno = true;
            bool transDummy1 = false;
            bool transDummy2 = false;

            estatisticas(arquivo(faixaBorda), mediasBorda, desvioBorda, validoBorda, transDummy1);
            estatisticas(arquivo(faixaInterna), mediasInternas, desvioInterno, validoInterno, transDummy2);

            if (!validoBorda || !validoInterno || transDummy1 || transDummy2) {
                return;
            }

            const double diferencaCor = diferenca(mediasBorda, mediasInternas);
            const bool transicao = (diferencaCor > desvio);

            if (desvioBorda <= desvio && transicao) {
                mascaraProtecao(regiaoBorda).setTo(255);
            }
        };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        canalAlpha = bgra[3].clone();

        cv::Mat bgrTemp;
        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, bgrTemp);

        cv::Mat mascaraTransparente = (canalAlpha == 0);
        if (cv::countNonZero(mascaraTransparente) > 0 && cv::countNonZero(~mascaraTransparente) > 0) {
            cv::inpaint(bgrTemp, mascaraTransparente, bgrTemp, 3.0, cv::INPAINT_TELEA);
        }

        imagemProcessar = bgrTemp;
    } else if (canaisOriginais == 1) {
        cv::cvtColor(arquivo, imagemProcessar, cv::COLOR_GRAY2BGR);
    }

    cv::Mat filtrado;
    cv::fastNlMeansDenoisingColored(imagemProcessar, filtrado, intensidade, intensidade, 7, 21);

    cv::Mat cinza;
    if (canaisOriginais == 1) {
        cinza = arquivo.clone();
    } else {
        cv::cvtColor(imagemProcessar, cinza, cv::COLOR_BGR2GRAY);
    }

    cv::Mat gradX, gradY, gradiente;
    cv::Sobel(cinza, gradX, CV_32F, 1, 0, 3);
    cv::Sobel(cinza, gradY, CV_32F, 0, 1, 3);
    cv::magnitude(gradX, gradY, gradiente);
    cv::threshold(gradiente, gradiente, 20.0, 255.0, cv::THRESH_BINARY);
    gradiente.convertTo(gradiente, CV_8UC1);

    cv::dilate(
        gradiente,
        gradiente,
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))
    );

    cv::Mat mascaraProtecaoTotal = gradiente;
    if (borda && limite > 0) {
        cv::bitwise_or(mascaraProtecaoTotal, mascaraProtecao, mascaraProtecaoTotal);
    }

    cv::Mat resultadoLimpeza;
    if (canaisOriginais == 1) {
        cv::Mat filtradoCinza;
        cv::cvtColor(filtrado, filtradoCinza, cv::COLOR_BGR2GRAY);
        resultadoLimpeza = filtradoCinza.clone();
        arquivo.copyTo(resultadoLimpeza, mascaraProtecaoTotal);
    } else if (canaisOriginais == 4) {
        resultadoLimpeza = filtrado.clone();
        cv::Mat bgrOriginal;
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, bgrOriginal);
        bgrOriginal.copyTo(resultadoLimpeza, mascaraProtecaoTotal);

        resultadoLimpeza.setTo(cv::Scalar(0, 0, 0), canalAlpha == 0);
        std::vector<cv::Mat> saidaCanais;
        cv::split(resultadoLimpeza, saidaCanais);
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, resultadoLimpeza);
    } else {
        resultadoLimpeza = filtrado.clone();
        arquivo.copyTo(resultadoLimpeza, mascaraProtecaoTotal);
    }

    return resultadoLimpeza;
}

cv::Mat Render::brilho(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa)) {
        return arquivo.clone();
    }

    double valorBrilho = alfa;
    if (valorBrilho < 0.0) {
        valorBrilho = 0.0;
    } else if (valorBrilho > 100.0) {
        valorBrilho = 100.0;
    }

    valorBrilho = (valorBrilho - 50.0) * 2.54;

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        canalAlpha = bgra[3].clone();

        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, imagemProcessar);
    }

    cv::Mat resultadoBrilho;
    imagemProcessar.convertTo(resultadoBrilho, -1, 1.0, valorBrilho);

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> saidaCanais;
        cv::split(resultadoBrilho, saidaCanais);

        std::vector<cv::Mat> bgraOriginal;
        cv::split(arquivo, bgraOriginal);
        for (int i = 0; i < 3; ++i) {
            bgraOriginal[i].copyTo(saidaCanais[i], canalAlpha == 0);
        }
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, resultadoBrilho);
    }

    return resultadoBrilho;
}

cv::Mat Render::contraste(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa)) {
        return arquivo.clone();
    }

    double valorContraste = alfa;
    if (valorContraste < 0.0) {
        valorContraste = 0.0;
    } else if (valorContraste > 100.0) {
        valorContraste = 100.0;
    }

    valorContraste /= 50.0;

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 1 && canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        canalAlpha = bgra[3].clone();

        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, imagemProcessar);
    }

    cv::Mat resultadoContraste;
    imagemProcessar.convertTo(resultadoContraste, -1, valorContraste, 0);

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> saidaCanais;
        cv::split(resultadoContraste, saidaCanais);

        std::vector<cv::Mat> bgraOriginal;
        cv::split(arquivo, bgraOriginal);
        for (int i = 0; i < 3; ++i) {
            bgraOriginal[i].copyTo(saidaCanais[i], canalAlpha == 0);
        }
        saidaCanais.push_back(canalAlpha);
        cv::merge(saidaCanais, resultadoContraste);
    }

    return resultadoContraste;
}

cv::Mat Render::cores(const cv::Mat& arquivo, double alfa, int gama) {
    if (arquivo.empty() || arquivo.depth() != CV_8U || (arquivo.channels() != 3 && arquivo.channels() != 4) ||
        !std::isfinite(alfa)) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa /= 50.0;

    const int canalAlvo = gama - 1;

    if (canalAlvo < 0 || canalAlvo > 2) {
        return arquivo.clone();
    }

    cv::Mat hsv;

    if (arquivo.channels() == 4) {
        cv::Mat bgr;
        cv::cvtColor(arquivo, bgr, cv::COLOR_BGRA2BGR);
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    }
    else {
        cv::cvtColor(arquivo, hsv, cv::COLOR_BGR2HSV);
    }

    cv::Mat processada = arquivo.clone();

    for (int i = 0; i < processada.rows; ++i) {
        for (int j = 0; j < processada.cols; ++j) {
            const cv::Vec3b& cor = hsv.at<cv::Vec3b>(i, j);
            cv::Vec3b original;

            if (arquivo.channels() == 3) {
                original = arquivo.at<cv::Vec3b>(i, j);
            }
            else {
                const auto& pixel = arquivo.at<cv::Vec4b>(i, j);
                original = cv::Vec3b(pixel[0], pixel[1], pixel[2]);
            }

            const uchar max_val = std::max({original[0], original[1], original[2]});
            const uchar min_val = std::min({original[0], original[1], original[2]});

            if (cor[1] <= 25) {
                continue;
            }

            if ((max_val - min_val) < 20) {
                continue;
            }

            if (max_val >= 245 && (max_val - min_val) < 35) {
                continue;
            }

            if (arquivo.channels() == 3) {
                auto& pixel = processada.at<cv::Vec3b>(i, j);
                pixel[canalAlvo] = cv::saturate_cast<uchar>(pixel[canalAlvo] * alfa);
            }
            else {
                auto& pixel = processada.at<cv::Vec4b>(i, j);
                pixel[canalAlvo] = cv::saturate_cast<uchar>(pixel[canalAlvo] * alfa);
            }
        }
    }

    return processada;
}

cv::Mat Render::cinzas(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty() || arquivo.rows <= 0 || arquivo.cols <= 0) {
        return arquivo.clone();
    }

    if (!std::isfinite(alfa)) {
        return arquivo.clone();
    }

    int canaisOriginais = arquivo.channels();
    if (arquivo.depth() != CV_8U || (canaisOriginais != 3 && canaisOriginais != 4)) {
        return arquivo.clone();
    }

    alfa = std::clamp(alfa, 0.0, 100.0);

    double fator = alfa / 100.0;

    cv::Mat imagemProcessar = arquivo;
    cv::Mat canalAlpha;

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> bgra;
        cv::split(arquivo, bgra);
        canalAlpha = bgra[3].clone();

        std::vector<cv::Mat> bgr = { bgra[0], bgra[1], bgra[2] };
        cv::merge(bgr, imagemProcessar);
    }

    cv::Mat hsv;
    cv::cvtColor(imagemProcessar, hsv, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsvCanais;
    cv::split(hsv, hsvCanais);

    hsvCanais[1].convertTo(hsvCanais[1], CV_32F);
    hsvCanais[1] *= fator;
    hsvCanais[1].convertTo(hsvCanais[1], CV_8U);

    cv::Mat hsvModificado, bgrResultado;
    cv::merge(hsvCanais, hsvModificado);
    cv::cvtColor(hsvModificado, bgrResultado, cv::COLOR_HSV2BGR);

    if (canaisOriginais == 4) {
        std::vector<cv::Mat> saidaCanais;
        cv::split(bgrResultado, saidaCanais);

        std::vector<cv::Mat> bgraOriginal;
        cv::split(arquivo, bgraOriginal);
        for (int i = 0; i < 3; ++i) {
            bgraOriginal[i].copyTo(saidaCanais[i], canalAlpha == 0);
        }
        saidaCanais.push_back(canalAlpha);

        cv::Mat resultadoBGRA;
        cv::merge(saidaCanais, resultadoBGRA);
        return resultadoBGRA;
    }

    return bgrResultado;
}

void Render::camera(int dispositivo) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    frameAnterior.release();
    regiaoAnterior = cv::Rect();
    imagem.release();
    resultado.release();

    // A origem só é marcada como webcam depois que a abertura funciona.
    definirOrigem(this, OrigemRender::Nenhuma);

    if (!leitor.open(dispositivo, cv::CAP_DSHOW)) {
        std::cerr << "Não foi possível acessar a câmera." << std::endl;
        return;
    }

    definirOrigem(this, OrigemRender::Webcam);
}

void Render::midia(const std::string& arquivo) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    frameAnterior.release();
    regiaoAnterior = cv::Rect();
    imagem.release();
    resultado.release();

    definirOrigem(this, OrigemRender::Nenhuma);

    // Primeiro identifica uma imagem estática. Ela não depende de que
    // VideoCapture consiga abrir imagens como se fossem vídeos.
    cv::Mat imagemEstatica = cv::imread(arquivo, cv::IMREAD_UNCHANGED);
    if (!imagemEstatica.empty()) {
        imagem = imagemEstatica.clone();
        definirOrigem(this, OrigemRender::Imagem);
        return;
    }

    // Se não for imagem, tenta abrir como vídeo.
    if (!leitor.open(arquivo)) {
        std::cerr << "Não foi possível abrir o arquivo como imagem ou vídeo: "
                  << arquivo << std::endl;
        return;
    }

    definirOrigem(this, OrigemRender::Video);
}

void Render::janela() {
    if (!leitor.isOpened()) {
        std::cerr << "O scanner precisa de uma fonte aberta pela webcam." << std::endl;
        return;
    }

    if (obterOrigem(this) != OrigemRender::Webcam) {
        std::cerr << "O scanner funciona com a webcam. Para arquivos de imagem ou vídeo, use midia()." << std::endl;
        return;
    }

    cv::Mat frame;
    bool localPausar = false;

    cv::Mat ultimaDigitalizacaoValida;
    bool temDigitalizacaoValida = false;
    const std::string janelaDigitalizacaoNome = "Digitalizacao do Documento";
    bool janelaDigitalizacaoAberta = false;

    cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);

    cv::HOGDescriptor detectorPessoa;
    detectorPessoa.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    bool hogAtivo = true;
    int falhasHogConsecutivas = 0;

    int errosConsecutivos = 0;

    int falhasLeituraConsecutivas = 0;

    while (true) {
        constexpr int limiteErrosCriticos = 10;
        bool cicloBemSucedido = false;

        try {
            int tecla = cv::waitKey(30) & 0xFF;

            if (tecla == 'c' || tecla == 'C' || tecla == 27) {
                break;
            }
            else if (tecla == 'p' || tecla == 'P') {
                localPausar = !localPausar;
            }
            else if (tecla == 'r' || tecla == 'R') {
                if (obterOrigem(this) != OrigemRender::Webcam) {
                    bool seekSucesso = leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                    double posAtual = leitor.get(cv::CAP_PROP_POS_FRAMES);

                    if (seekSucesso && std::isfinite(posAtual) &&
                        posAtual >= 0.0 && posAtual <= 1.0) {
                        std::cout << "[INFO] Backend informou posicionamento proximo ao inicio do video." << std::endl;
                    } else {
                        std::cerr << "[AVISO] Nao foi possivel confirmar o posicionamento proximo ao inicio do video." << std::endl;
                    }
                } else {
                    std::cout << "[INFO] Captura ao vivo (webcam) mantida; historico de movimento e scanner reiniciados." << std::endl;
                }

                frameAnterior.release();
                regiaoAnterior = cv::Rect();
                ultimaDigitalizacaoValida.release();
                temDigitalizacaoValida = false;
                localPausar = false;
                errosConsecutivos = 0;
                falhasHogConsecutivas = 0;
                falhasLeituraConsecutivas = 0;
                hogAtivo = true;
            }
            else if (tecla == 's' || tecla == 'S') {
                if (temDigitalizacaoValida && !ultimaDigitalizacaoValida.empty()) {
                    if (const bool sucessoGravacao = cv::imwrite("digitalizacao.png", ultimaDigitalizacaoValida); sucessoGravacao) {
                        std::cout << "[SUCESSO] Arquivo salvo com sucesso em 'digitalizacao.png'." << std::endl;
                    } else {
                        std::cerr << "[ERRO] Falha ao gravar o arquivo 'digitalizacao.png'." << std::endl;
                    }
                } else {
                    std::cout << "[AVISO] Nenhuma digitalizacao valida disponivel para salvamento." << std::endl;
                }
            }

            if (!localPausar) {
                bool sucesso = leitor.read(frame);
                if (!sucesso || frame.empty()) {
                    if (obterOrigem(this) != OrigemRender::Webcam) {
                        std::cerr << "[AVISO] Fim do fluxo de video detectado." << std::endl;
                        break;
                    } else {
                        constexpr int limiteFalhasLeituraWebcam = 10;
                        falhasLeituraConsecutivas++;
                        std::cerr << "[AVISO] Falha temporaria de leitura na webcam (" << falhasLeituraConsecutivas << "/" << limiteFalhasLeituraWebcam << ")." << std::endl;
                        if (falhasLeituraConsecutivas >= limiteFalhasLeituraWebcam) {
                            std::cerr << "[ERRO CRITICO] Perda persistente de conexao com a webcam. Encerrando." << std::endl;
                            break;
                        }
                        continue;
                    }
                }

                falhasLeituraConsecutivas = 0;

                if (frame.channels() == 4) {
                    cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
                } else if (frame.channels() == 1) {
                    cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
                }

                if (frame.channels() != 3) {
                    std::cerr << "[ERRO] Formato de quadro incompativel (canais != 3)." << std::endl;
                    errosConsecutivos++;
                    if (errosConsecutivos >= limiteErrosCriticos) {
                        std::cerr << "[ERRO CRITICO] Limite de quadros invalidos excedido. Encerrando." << std::endl;
                        break;
                    }
                    continue;
                }

                cv::Mat frameParaDocumento = frame.clone();

                std::vector<cv::Rect> pessoasDetectadas;
                if (hogAtivo) {
                    constexpr int limiteFalhasHog = 5;
                    try {
                        detectorPessoa.detectMultiScale(frame, pessoasDetectadas, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2, false);
                        falhasHogConsecutivas = 0;
                    } catch (const cv::Exception& e) {
                        falhasHogConsecutivas++;
                        std::cerr << "[AVISO] Erro na deteccao HOG (" << falhasHogConsecutivas << "/" << limiteFalhasHog << "): " << e.what() << std::endl;
                        if (falhasHogConsecutivas >= limiteFalhasHog) {
                            hogAtivo = false;
                            std::cerr << "[ALERTA] Deteccao HOG desativada temporariamente devido a falhas consecutivas." << std::endl;
                        }
                    }
                }

                cv::Mat cinza;
                cv::cvtColor(frame, cinza, cv::COLOR_BGR2GRAY);
                cv::Mat cinzaBlur;
                cv::GaussianBlur(cinza, cinzaBlur, cv::Size(21, 21), 0);

                if (frameAnterior.empty() || frameAnterior.size() != cinzaBlur.size() || frameAnterior.type() != cinzaBlur.type()) {
                    frameAnterior = cinzaBlur.clone();
                } else {
                    cv::Mat diferenca;
                    cv::absdiff(frameAnterior, cinzaBlur, diferenca);
                    cv::Mat limiteMovimento;
                    cv::threshold(diferenca, limiteMovimento, 25, 255, cv::THRESH_BINARY);
                    cv::dilate(limiteMovimento, limiteMovimento, cv::Mat(), cv::Point(-1, -1), 2);

                    std::vector<std::vector<cv::Point>> contornos;
                    cv::findContours(limiteMovimento, contornos, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                    for (const auto& contorno : contornos) {
                        if (cv::contourArea(contorno) < 500) {
                            continue;
                        }
                        cv::Rect regiaoMovimento = cv::boundingRect(contorno);

                        bool movimentoDePessoa = false;

                        if (hogAtivo) {
                            for (const auto& pessoa : pessoasDetectadas) {
                                if ((regiaoMovimento & pessoa).area() > 0) {
                                    movimentoDePessoa = true;
                                    break;
                                }
                            }
                        }

                        bool desenharRegiao = hogAtivo
                            ? movimentoDePessoa
                            : true;

                        if (desenharRegiao) {
                            cv::Scalar cor = hogAtivo
                                ? cv::Scalar(0, 255, 0)
                                : cv::Scalar(0, 165, 255);

                            cv::rectangle(frame, regiaoMovimento, cor, 2);
                        }
                    }
                    frameAnterior = cinzaBlur.clone();
                }

                cv::Mat imagemCinza, imagemBinarizada;
                cv::cvtColor(frameParaDocumento, imagemCinza, cv::COLOR_BGR2GRAY);
                cv::GaussianBlur(imagemCinza, imagemCinza, cv::Size(5, 5), 0);
                cv::adaptiveThreshold(imagemCinza, imagemBinarizada, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 21, 2);

                std::vector<std::vector<cv::Point>> contornosDocs;
                cv::findContours(imagemBinarizada, contornosDocs, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

                double maiorArea = 0;
                std::vector<cv::Point> maiorContornoValido;
                int frameW = frame.cols;
                int frameH = frame.rows;
                double areaTotalFrame = static_cast<double>(frameW) * static_cast<double>(frameH);

                for (const auto& contorno : contornosDocs) {
                    double area = cv::contourArea(contorno);
                    if (area > areaTotalFrame * 0.10 && area < areaTotalFrame * 0.90) {
                        double perimetro = cv::arcLength(contorno, true);
                        std::vector<cv::Point> aprox;
                        cv::approxPolyDP(contorno, aprox, 0.02 * perimetro, true);

                        if (aprox.size() == 4 && cv::isContourConvex(aprox) && area > maiorArea) {
                            cv::Rect boundingBox = cv::boundingRect(aprox);
                            if (boundingBox.x > 10 && boundingBox.y > 10 &&
                                (boundingBox.x + boundingBox.width) < frameW - 10 &&
                                (boundingBox.y + boundingBox.height) < frameH - 10) {
                                maiorArea = area;
                                maiorContornoValido = aprox;
                            }
                        }
                    }
                }

                if (maiorContornoValido.size() == 4) {
                    std::vector<cv::Point2f> pontos(4);
                    for (int i = 0; i < 4; ++i) {
                        pontos[i] = static_cast<cv::Point2f>(maiorContornoValido[i]);
                    }

                    cv::Point2f centro(0.0f, 0.0f);
                    for (const auto& p : pontos) {
                        centro += p;
                    }
                    centro *= 0.25f;

                    std::vector<cv::Point2f> pontosOrdenados = pontos;
                    std::sort(
                        pontosOrdenados.begin(),
                        pontosOrdenados.end(),
                        [centro](const cv::Point2f& a, const cv::Point2f& b) {
                            return std::atan2(a.y - centro.y, a.x - centro.x) <
                                   std::atan2(b.y - centro.y, b.x - centro.x);
                        }
                    );

                    float minX = pontosOrdenados[0].x;
                    float minY = pontosOrdenados[0].y;
                    for (const auto& p : pontosOrdenados) {
                        if (p.x < minX) minX = p.x;
                        if (p.y < minY) minY = p.y;
                    }

                    int idxTL = 0;
                    double menorDistSq = std::numeric_limits<double>::max();
                    for (int i = 0; i < 4; ++i) {
                        const double distSq = std::pow(pontosOrdenados[i].x - minX, 2) +
                                              std::pow(pontosOrdenados[i].y - minY, 2);
                        if (distSq < menorDistSq) {
                            menorDistSq = distSq;
                            idxTL = i;
                        }
                    }

                    std::vector<cv::Point2f> pontosRotacionados(4);
                    for (int i = 0; i < 4; ++i) {
                        pontosRotacionados[i] = pontosOrdenados[(idxTL + i) % 4];
                    }

                    bool geometriaValida = true;
                    for (size_t i = 0; i < 4; ++i) {
                        const cv::Point2f& atual = pontosRotacionados[i];
                        const cv::Point2f& proximo = pontosRotacionados[(i + 1) % 4];

                        if (cv::norm(proximo - atual) < 10.0f) {
                            geometriaValida = false;
                            break;
                        }
                    }

                    if (geometriaValida) {
                        std::vector<cv::Point> pontosValidacao;
                        pontosValidacao.reserve(4);

                        for (const auto& p : pontosRotacionados) {
                            pontosValidacao.emplace_back(
                                cvRound(p.x),
                                cvRound(p.y)
                            );
                        }

                        if (cv::isContourConvex(pontosValidacao)) {
                            float larguraTopo = std::hypot(pontosRotacionados[1].x - pontosRotacionados[0].x, pontosRotacionados[1].y - pontosRotacionados[0].y);
                            float larguraBase = std::hypot(pontosRotacionados[2].x - pontosRotacionados[3].x, pontosRotacionados[2].y - pontosRotacionados[3].y);
                            int larguraMax = static_cast<int>(std::max(larguraTopo, larguraBase));

                            float alturaEsq = std::hypot(pontosRotacionados[3].x - pontosRotacionados[0].x, pontosRotacionados[3].y - pontosRotacionados[0].y);
                            float alturaDir = std::hypot(pontosRotacionados[2].x - pontosRotacionados[1].x, pontosRotacionados[2].y - pontosRotacionados[1].y);
                            int alturaMax = static_cast<int>(std::max(alturaEsq, alturaDir));

                            if (larguraMax > 50 && alturaMax > 50) {
                                std::vector<cv::Point2f> destino = {
                                    cv::Point2f(0, 0),
                                    cv::Point2f(static_cast<float>(larguraMax - 1), 0),
                                    cv::Point2f(static_cast<float>(larguraMax - 1), static_cast<float>(alturaMax - 1)),
                                    cv::Point2f(0, static_cast<float>(alturaMax - 1))
                                };

                                cv::Mat matrizPerspectiva = cv::getPerspectiveTransform(pontosRotacionados, destino);
                                cv::Mat scannerCinza, digitalizacaoTemp;

                                cv::warpPerspective(frameParaDocumento, scannerCinza, matrizPerspectiva, cv::Size(larguraMax, alturaMax));
                                if (scannerCinza.channels() == 3) {
                                    cv::cvtColor(scannerCinza, scannerCinza, cv::COLOR_BGR2GRAY);
                                } else if (scannerCinza.channels() == 4) {
                                    cv::cvtColor(scannerCinza, scannerCinza, cv::COLOR_BGRA2GRAY);
                                }

                                cv::adaptiveThreshold(
                                    scannerCinza,
                                    digitalizacaoTemp,
                                    255,
                                    cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                    cv::THRESH_BINARY,
                                    21,
                                    5
                                );

                                digitalizacaoTemp.copyTo(ultimaDigitalizacaoValida);
                                temDigitalizacaoValida = true;

                                for (size_t i = 0; i < 4; ++i) {
                                    cv::line(frame, maiorContornoValido[i], maiorContornoValido[(i + 1) % 4], cv::Scalar(0, 0, 255), 2);
                                }
                            }
                        }
                    }
                }
            }

            if (!frame.empty()) {
                cv::imshow("Reprodutor", frame);
            }

            if (temDigitalizacaoValida && !ultimaDigitalizacaoValida.empty()) {
                if (!janelaDigitalizacaoAberta) {
                    cv::namedWindow(janelaDigitalizacaoNome, cv::WINDOW_AUTOSIZE);
                    janelaDigitalizacaoAberta = true;
                }
                cv::imshow(janelaDigitalizacaoNome, ultimaDigitalizacaoValida);
            }

            cicloBemSucedido = true;

        } catch (const cv::Exception& e) {
            std::cerr << "[ERRO OpenCV] " << e.what() << std::endl;
            errosConsecutivos++;
            if (errosConsecutivos >= limiteErrosCriticos) {
                std::cerr << "[ERRO CRITICO] Muitas excecoes do OpenCV consecutivas. Encerrando." << std::endl;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERRO STD] " << e.what() << std::endl;
            errosConsecutivos++;
            if (errosConsecutivos >= limiteErrosCriticos) {
                std::cerr << "[ERRO CRITICO] Muitas excecoes padrao consecutivas. Encerrando." << std::endl;
                break;
            }
        } catch (...) {
            std::cerr << "[ERRO DESCONHECIDO] Excecao nao tratada capturada." << std::endl;
            errosConsecutivos++;
            if (errosConsecutivos >= limiteErrosCriticos) {
                break;
            }
        }

        if (cicloBemSucedido) {
            errosConsecutivos = 0;
        }
    }

    try {
        cv::destroyWindow("Reprodutor");
    } catch (...) {
    }

    if (janelaDigitalizacaoAberta) {
        cv::destroyWindow(janelaDigitalizacaoNome);
    }
}

const std::vector<int>& Render::filtrosUsados() const {
    return ultimaSequencia;
}

const std::vector<Parametros>& Render::parametrosUsados() const {
    return ultimosFiltros;
}

Render::~Render() {
    origensRender().erase(this);
    if (leitor.isOpened()) {
        leitor.release();
    }

    if (gravador.isOpened()) {
        gravador.release();
    }
}
