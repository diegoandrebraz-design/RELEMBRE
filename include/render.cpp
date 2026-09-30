#include "../include/render.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <windows.h>

Render::Render() = default;

cv::Mat Render::render(
    int escolha,
    const Parametros& filtro
) {
    leitor >> imagem;

    if (imagem.empty()) {
        resultado.release();
        return {};
    }

    switch (escolha) {
    case 1:
        resultado = girar(
            imagem,
            filtro.alfa,
            filtro.gama
        );
        break;

    case 2:
        resultado = recortar(
            imagem,
            filtro.gama,
            filtro.delta
        );
        break;

    case 3:
        resultado = granular(
            imagem,
            filtro.alfa
        );
        break;

    case 4:
        resultado = nitidez(
            imagem,
            filtro.alfa
        );
        break;

    case 5:
        resultado = desfocar(
            imagem,
            filtro.gama
        );
        break;

    case 6:
        resultado = remover(
            imagem,
            filtro.alfa
        );
        break;

    case 7:
        resultado = limpar(
            imagem,
            filtro.beta
        );
        break;

    case 8:
        resultado = brilho(
            imagem,
            filtro.alfa
        );
        break;

    case 9:
        resultado = contraste(
            imagem,
            filtro.alfa
        );
        break;

    case 10:
        resultado = cores(
            imagem,
            filtro.alfa,
            filtro.gama
        );
        break;

    case 11:
        resultado = cinzas(
            imagem,
            filtro.alfa
        );
        break;

    default:
        std::cout << "Opção invalida!\n";
        resultado.release();
        return {};
    }

    if (
        gravador.isOpened() &&
        !resultado.empty()
    ) {
        gravador.write(resultado);
    }

    return resultado;
}

cv::Mat Render::render(
    const std::vector<int>& escolhas,
    const std::vector<Parametros>& filtro
) {
    if (
        escolhas.empty() ||
        escolhas.size() != filtro.size()
    ) {
        resultado.release();
        return {};
    }

    leitor >> imagem;

    if (imagem.empty()) {
        resultado.release();
        return {};
    }

    resultado = imagem.clone();

    for (size_t i = 0; i < escolhas.size(); ++i) {
        switch (escolhas[i]) {
        case 1:
            resultado = girar(
                resultado,
                filtro[i].alfa,
                filtro[i].gama
            );
            break;

        case 2:
            resultado = recortar(
                resultado,
                filtro[i].gama,
                filtro[i].delta
            );
            break;

        case 3:
            resultado = granular(
                resultado,
                filtro[i].alfa
            );
            break;

        case 4:
            resultado = nitidez(
                resultado,
                filtro[i].alfa
            );
            break;

        case 5:
            resultado = desfocar(
                resultado,
                filtro[i].gama
            );
            break;

        case 6:
            resultado = remover(
                resultado,
                filtro[i].alfa
            );
            break;

        case 7:
            resultado = limpar(
                resultado,
                filtro[i].beta
            );
            break;

        case 8:
            resultado = brilho(
                resultado,
                filtro[i].alfa
            );
            break;

        case 9:
            resultado = contraste(
                resultado,
                filtro[i].alfa
            );
            break;

        case 10:
            resultado = cores(
                resultado,
                filtro[i].alfa,
                filtro[i].gama
            );
            break;

        case 11:
            resultado = cinzas(
                resultado,
                filtro[i].alfa
            );
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

    if (
        gravador.isOpened() &&
        !resultado.empty()
    ) {
        gravador.write(resultado);
    }

    return resultado;
}

cv::Mat Render::comparar(
    const std::vector<int>& escolhas,
    const std::vector<Parametros>& filtro
) {
    if (
        !leitor.isOpened() ||
        escolhas.size() != filtro.size()
    ) {
        resultado.release();
        return {};
    }

    std::vector<int> sequencia = escolhas;
    std::vector<Parametros> filtros = filtro;

    const bool camera =
        leitor.get(cv::CAP_PROP_FRAME_COUNT) <= 0;

    bool pausado = false;

    size_t filtroAtual = 0;
    int parametroAtual = 0;

    frameAnterior.release();

    cv::namedWindow(
        "Comparacao Lado a Lado",
        cv::WINDOW_NORMAL
    );

    /*
     * Aplica a sequência diretamente sobre o frame atual.
     *
     * Importante:
     * comparar() não chama render(), pois render() possui
     * responsabilidade própria de leitura e gravação.
     *
     * Aqui o objetivo é exclusivamente:
     *
     * frame original
     *       |
     *       +----> original
     *       |
     *       +----> filtros ----> processado
     *                              |
     *                              v
     *                    original | processado
     */
    auto aplicarFiltro =
        [this](
            cv::Mat entrada,
            int escolha,
            const Parametros& parametros
        ) -> cv::Mat {
            switch (escolha) {
            case 1:
                return girar(
                    entrada,
                    parametros.alfa,
                    parametros.gama
                );

            case 2:
                return recortar(
                    entrada,
                    parametros.gama,
                    parametros.delta
                );

            case 3:
                return granular(
                    entrada,
                    parametros.alfa
                );

            case 4:
                return nitidez(
                    entrada,
                    parametros.alfa
                );

            case 5:
                return desfocar(
                    entrada,
                    parametros.gama
                );

            case 6:
                return remover(
                    entrada,
                    parametros.alfa
                );

            case 7:
                return limpar(
                    entrada,
                    parametros.beta
                );

            case 8:
                return brilho(
                    entrada,
                    parametros.alfa
                );

            case 9:
                return contraste(
                    entrada,
                    parametros.alfa
                );

            case 10:
                return cores(
                    entrada,
                    parametros.alfa,
                    parametros.gama
                );

            case 11:
                return cinzas(
                    entrada,
                    parametros.alfa
                );

            default:
                return {};
            }
        };

    while (true) {
        if (!pausado) {
            cv::Mat original;

            leitor >> original;

            if (original.empty()) {
                if (camera) {
                    cv::waitKey(10);
                    continue;
                }

                break;
            }

            imagem = original.clone();

            cv::Mat processado =
                original.clone();

            for (size_t i = 0; i < sequencia.size(); ++i) {
                processado =
                    aplicarFiltro(
                        processado,
                        sequencia[i],
                        filtros[i]
                    );

                if (processado.empty()) {
                    break;
                }
            }

            if (processado.empty()) {
                break;
            }

            /*
             * Para exibição lado a lado, todos os resultados
             * são normalizados para BGR quando necessário.
             */
            if (original.channels() == 4) {
                cv::cvtColor(
                    original,
                    original,
                    cv::COLOR_BGRA2BGR
                );
            }
            else if (original.channels() == 1) {
                cv::cvtColor(
                    original,
                    original,
                    cv::COLOR_GRAY2BGR
                );
            }

            if (processado.channels() == 4) {
                cv::cvtColor(
                    processado,
                    processado,
                    cv::COLOR_BGRA2BGR
                );
            }
            else if (processado.channels() == 1) {
                cv::cvtColor(
                    processado,
                    processado,
                    cv::COLOR_GRAY2BGR
                );
            }

            if (original.empty() || processado.empty()) {
                break;
            }

            if (
                original.size() !=
                processado.size()
            ) {
                cv::resize(
                    original,
                    original,
                    processado.size(),
                    0,
                    0,
                    cv::INTER_LINEAR
                );
            }

            if (
                original.channels() !=
                processado.channels()
            ) {
                break;
            }

            if (
                original.depth() !=
                processado.depth()
            ) {
                processado.convertTo(
                    processado,
                    original.type()
                );
            }

            cv::hconcat(
                original,
                processado,
                resultado
            );

            if (resultado.empty()) {
                break;
            }

            const int telaLargura =
                GetSystemMetrics(
                    SM_CXSCREEN
                );

            const int telaAltura =
                GetSystemMetrics(
                    SM_CYSCREEN
                );

            const int larguraMaxima =
                std::max(
                    1,
                    static_cast<int>(
                        telaLargura * 0.90
                    )
                );

            const int alturaMaxima =
                std::max(
                    1,
                    static_cast<int>(
                        telaAltura * 0.80
                    )
                );

            const double escalaLargura =
                static_cast<double>(
                    larguraMaxima
                ) /
                static_cast<double>(
                    resultado.cols
                );

            const double escalaAltura =
                static_cast<double>(
                    alturaMaxima
                ) /
                static_cast<double>(
                    resultado.rows
                );

            const double escala =
                std::min(
                    escalaLargura,
                    escalaAltura
                );

            cv::Mat exibicao;

            if (escala < 1.0) {
                cv::resize(
                    resultado,
                    exibicao,
                    cv::Size(),
                    escala,
                    escala,
                    cv::INTER_AREA
                );
            }
            else {
                exibicao = resultado;
            }

            cv::resizeWindow(
                "Comparacao Lado a Lado",
                exibicao.cols,
                exibicao.rows
            );

            cv::imshow(
                "Comparacao Lado a Lado",
                exibicao
            );
        }

        int tecla =
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
            if (!camera) {
                leitor.set(
                    cv::CAP_PROP_POS_FRAMES,
                    0
                );

                frameAnterior.release();
                pausado = false;
            }
        }
        else if (
            tecla == 'z' ||
            tecla == 'Z'
        ) {
            if (!sequencia.empty()) {
                sequencia.pop_back();
                filtros.pop_back();

                if (sequencia.empty()) {
                    filtroAtual = 0;
                    parametroAtual = 0;
                }
                else if (
                    filtroAtual >=
                    sequencia.size()
                ) {
                    filtroAtual =
                        sequencia.size() - 1;

                    parametroAtual = 0;
                }
            }
        }
        else if (
            tecla == '/' ||
            tecla == '*'
        ) {
            if (!sequencia.empty()) {
                if (tecla == '/') {
                    if (filtroAtual > 0) {
                        --filtroAtual;
                    }
                }
                else {
                    if (
                        filtroAtual + 1 <
                        sequencia.size()
                    ) {
                        ++filtroAtual;
                    }
                }

                parametroAtual = 0;
            }
        }
        else if (
            tecla == 't' ||
            tecla == 'T'
        ) {
            if (!sequencia.empty()) {
                const int escolha =
                    sequencia[filtroAtual];

                int quantidadeParametros = 0;

                switch (escolha) {
                case 1:
                case 2:
                case 10:
                    quantidadeParametros = 2;
                    break;

                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 11:
                    quantidadeParametros = 1;
                    break;

                default:
                    quantidadeParametros = 0;
                    break;
                }

                if (quantidadeParametros > 0) {
                    ++parametroAtual;

                    if (
                        parametroAtual >=
                        quantidadeParametros
                    ) {
                        parametroAtual = 0;
                    }
                }
            }
        }
        else if (
            tecla == '+' ||
            tecla == '=' ||
            tecla == '-'
        ) {
            if (!sequencia.empty()) {
                const int escolha =
                    sequencia[filtroAtual];

                int parametro = -1;

                switch (escolha) {
                case 1:
                    parametro =
                        parametroAtual == 0
                            ? 0
                            : 2;
                    break;

                case 2:
                    parametro =
                        parametroAtual == 0
                            ? 2
                            : 3;
                    break;

                case 3:
                case 4:
                case 6:
                case 8:
                case 9:
                case 11:
                    parametro = 0;
                    break;

                case 5:
                    parametro = 2;
                    break;

                case 7:
                    parametro = 1;
                    break;

                case 10:
                    parametro =
                        parametroAtual == 0
                            ? 0
                            : 2;
                    break;

                default:
                    parametro = -1;
                    break;
                }

                if (parametro != -1) {
                    const double passo =
                        tecla == '-'
                            ? -1.0
                            : 1.0;

                    if (parametro == 0) {
                        filtros[filtroAtual].alfa +=
                            passo;
                    }
                    else if (parametro == 1) {
                        filtros[filtroAtual].beta +=
                            static_cast<float>(
                                passo
                            );
                    }
                    else if (parametro == 2) {
                        filtros[filtroAtual].gama +=
                            static_cast<int>(
                                passo
                            );
                    }
                    else if (parametro == 3) {
                        filtros[filtroAtual].delta +=
                            static_cast<int>(
                                passo
                            );
                    }
                }
            }
        }
    }

    cv::destroyWindow(
        "Comparacao Lado a Lado"
    );

    return resultado;
}

cv::Mat Render::girar(
    const cv::Mat& arquivo,
    double alfa,
    int gama
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    cv::Point2f centro(
        static_cast<float>(arquivo.cols) / 2.0f,
        static_cast<float>(arquivo.rows) / 2.0f
    );

    cv::Mat mocao =
        cv::getRotationMatrix2D(
            centro,
            alfa,
            1.0
        );

    const double seno =
        std::abs(
            mocao.at<double>(0, 1)
        );

    const double cosseno =
        std::abs(
            mocao.at<double>(0, 0)
        );

    const int novaLargura =
        static_cast<int>(
            arquivo.rows * seno +
            arquivo.cols * cosseno
        );

    const int novaAltura =
        static_cast<int>(
            arquivo.rows * cosseno +
            arquivo.cols * seno
        );

    if (
        novaLargura <= 0 ||
        novaAltura <= 0
    ) {
        return {};
    }

    mocao.at<double>(0, 2) +=
        (novaLargura - arquivo.cols) / 2.0;

    mocao.at<double>(1, 2) +=
        (novaAltura - arquivo.rows) / 2.0;

    cv::Mat processada;

    cv::warpAffine(
        arquivo,
        processada,
        mocao,
        cv::Size(
            novaLargura,
            novaAltura
        ),
        cv::INTER_LINEAR,
        cv::BORDER_CONSTANT,
        cv::Scalar(255, 255, 255, 255)
    );

    if (
        gama == 1 ||
        gama == 0 ||
        gama == -1
    ) {
        cv::flip(
            processada,
            processada,
            gama
        );
    }

    return processada;
}

cv::Mat Render::recortar(
    const cv::Mat& arquivo,
    int gama,
    int delta
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (
        gama <= 1 ||
        delta <= 1
    ) {
        return arquivo.clone();
    }

    if (esquerda < 0) {
        esquerda = 0;
    }

    if (topo < 0) {
        topo = 0;
    }

    if (esquerda >= arquivo.cols) {
        esquerda = arquivo.cols - 1;
    }

    if (topo >= arquivo.rows) {
        topo = arquivo.rows - 1;
    }

    if (
        esquerda + gama >
        arquivo.cols
    ) {
        gama =
            arquivo.cols - esquerda;
    }

    if (
        topo + delta >
        arquivo.rows
    ) {
        delta =
            arquivo.rows - topo;
    }

    if (
        gama <= 0 ||
        delta <= 0
    ) {
        return arquivo.clone();
    }

    const cv::Rect areaCorte(
        esquerda,
        topo,
        gama,
        delta
    );

    return arquivo(areaCorte).clone();
}

cv::Mat Render::granular(
    const cv::Mat& arquivo,
    double alfa
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    alfa =
        std::max(
            0.0,
            std::min(
                100.0,
                alfa
            )
        );

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(
            arquivo.size(),
            CV_8UC1
        );

    if (
        borda &&
        limite > 0 &&
        desvio >= 0.0
    ) {
        auto estatisticas =
            [&](
                const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio
            ) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                }
                else {
                    cv::split(
                        regiao,
                        canais
                    );
                }

                medias.resize(
                    canais.size()
                );

                maiorDesvio = 0.0;

                for (
                    size_t i = 0;
                    i < canais.size();
                    ++i
                ) {
                    cv::Scalar media;
                    cv::Scalar desvioCanal;

                    cv::meanStdDev(
                        canais[i],
                        media,
                        desvioCanal
                    );

                    medias[i] = media[0];

                    maiorDesvio =
                        std::max(
                            maiorDesvio,
                            desvioCanal[0]
                        );
                }
            };

        auto diferenca =
            [](
                const std::vector<double>& a,
                const std::vector<double>& b
            ) {
                if (a.size() != b.size()) {
                    return 0.0;
                }

                double maior = 0.0;

                for (
                    size_t i = 0;
                    i < a.size();
                    ++i
                ) {
                    maior =
                        std::max(
                            maior,
                            std::abs(
                                a[i] - b[i]
                            )
                        );
                }

                return maior;
            };

        auto detectar =
            [&](
                bool horizontal,
                bool inicio
            ) {
                const int tamanhoMaximo =
                    horizontal
                        ? std::min(
                              limite,
                              arquivo.rows / 2
                          )
                        : std::min(
                              limite,
                              arquivo.cols / 2
                          );

                if (tamanhoMaximo < 1) {
                    return;
                }

                int espessura = 0;

                for (
                    int i = 0;
                    i < tamanhoMaximo;
                    ++i
                ) {
                    cv::Rect faixa;

                    if (horizontal) {
                        const int y =
                            inicio
                                ? i
                                : arquivo.rows - 1 - i;

                        faixa =
                            cv::Rect(
                                0,
                                y,
                                arquivo.cols,
                                1
                            );
                    }
                    else {
                        const int x =
                            inicio
                                ? i
                                : arquivo.cols - 1 - i;

                        faixa =
                            cv::Rect(
                                x,
                                0,
                                1,
                                arquivo.rows
                            );
                    }

                    std::vector<double> medias;
                    double maiorDesvio = 0.0;

                    estatisticas(
                        arquivo(faixa),
                        medias,
                        maiorDesvio
                    );

                    if (maiorDesvio > desvio) {
                        break;
                    }

                    ++espessura;
                }

                if (espessura < 1) {
                    return;
                }

                const int posicaoInterior =
                    horizontal
                        ? (
                            inicio
                                ? espessura
                                : arquivo.rows -
                                  espessura - 1
                          )
                        : (
                            inicio
                                ? espessura
                                : arquivo.cols -
                                  espessura - 1
                          );

                if (posicaoInterior < 0) {
                    return;
                }

                cv::Rect regiaoBorda;
                cv::Rect faixaInterna;
                cv::Rect ultimaFaixa;

                if (horizontal) {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  arquivo.cols,
                                  espessura
                              )
                            : cv::Rect(
                                  0,
                                  arquivo.rows -
                                      espessura,
                                  arquivo.cols,
                                  espessura
                              );

                    faixaInterna =
                        cv::Rect(
                            0,
                            posicaoInterior,
                            arquivo.cols,
                            1
                        );

                    const int y =
                        inicio
                            ? espessura - 1
                            : arquivo.rows - espessura;

                    ultimaFaixa =
                        cv::Rect(
                            0,
                            y,
                            arquivo.cols,
                            1
                        );
                }
                else {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  espessura,
                                  arquivo.rows
                              )
                            : cv::Rect(
                                  arquivo.cols -
                                      espessura,
                                  0,
                                  espessura,
                                  arquivo.rows
                              );

                    faixaInterna =
                        cv::Rect(
                            posicaoInterior,
                            0,
                            1,
                            arquivo.rows
                        );

                    const int x =
                        inicio
                            ? espessura - 1
                            : arquivo.cols - espessura;

                    ultimaFaixa =
                        cv::Rect(
                            x,
                            0,
                            1,
                            arquivo.rows
                        );
                }

                std::vector<double> mediasBorda;
                std::vector<double> mediasInternas;

                double desvioBorda = 0.0;
                double desvioInterno = 0.0;

                estatisticas(
                    arquivo(ultimaFaixa),
                    mediasBorda,
                    desvioBorda
                );

                estatisticas(
                    arquivo(faixaInterna),
                    mediasInternas,
                    desvioInterno
                );

                const double diferencaCor =
                    diferenca(
                        mediasBorda,
                        mediasInternas
                    );

                const bool transicao =
                    diferencaCor > desvio ||
                    desvioInterno >
                        desvioBorda + desvio;

                if (transicao) {
                    mascaraProtecao(
                        regiaoBorda
                    ).setTo(255);
                }
            };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat mascaraGranulado;

    cv::bitwise_not(
        mascaraProtecao,
        mascaraGranulado
    );

    cv::Mat ruidoCinza =
        cv::Mat::zeros(
            arquivo.size(),
            CV_16SC1
        );

    cv::randn(
        ruidoCinza,
        cv::Scalar(0),
        cv::Scalar(alfa)
    );

    cv::Mat ruidoFinal;

    if (arquivo.channels() == 1) {
        ruidoFinal = ruidoCinza;
    }
    else {
        std::vector<cv::Mat> canaisRuido(
            arquivo.channels(),
            ruidoCinza
        );

        cv::merge(
            canaisRuido,
            ruidoFinal
        );
    }

    cv::Mat imagem16;

    arquivo.convertTo(
        imagem16,
        CV_MAKETYPE(
            CV_16S,
            arquivo.channels()
        )
    );

    cv::Mat ruidoAplicavel =
        cv::Mat::zeros(
            arquivo.size(),
            CV_MAKETYPE(
                CV_16S,
                arquivo.channels()
            )
        );

    ruidoFinal.copyTo(
        ruidoAplicavel,
        mascaraGranulado
    );

    cv::Mat resultado16;

    cv::add(
        imagem16,
        ruidoAplicavel,
        resultado16
    );

    resultado16.convertTo(
        resultado,
        arquivo.type()
    );

    return resultado;
}

cv::Mat Render::nitidez(
    const cv::Mat& arquivo,
    double alfa
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    alfa =
        std::max(
            0.0,
            std::min(
                100.0,
                alfa
            )
        );

    alfa /= 12.5;

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(
            arquivo.size(),
            CV_8UC1
        );

    if (
        borda &&
        limite > 0 &&
        desvio >= 0.0
    ) {
        auto estatisticas =
            [&](
                const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio
            ) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                }
                else {
                    cv::split(
                        regiao,
                        canais
                    );
                }

                medias.resize(
                    canais.size()
                );

                maiorDesvio = 0.0;

                for (
                    size_t i = 0;
                    i < canais.size();
                    ++i
                ) {
                    cv::Scalar media;
                    cv::Scalar desvioCanal;

                    cv::meanStdDev(
                        canais[i],
                        media,
                        desvioCanal
                    );

                    medias[i] = media[0];

                    maiorDesvio =
                        std::max(
                            maiorDesvio,
                            desvioCanal[0]
                        );
                }
            };

        auto diferenca =
            [](
                const std::vector<double>& a,
                const std::vector<double>& b
            ) {
                if (a.size() != b.size()) {
                    return 0.0;
                }

                double maior = 0.0;

                for (
                    size_t i = 0;
                    i < a.size();
                    ++i
                ) {
                    maior =
                        std::max(
                            maior,
                            std::abs(
                                a[i] - b[i]
                            )
                        );
                }

                return maior;
            };

        auto detectar =
            [&](
                bool horizontal,
                bool inicio
            ) {
                const int tamanho =
                    horizontal
                        ? arquivo.rows
                        : arquivo.cols;

                const int tamanhoMaximo =
                    std::min(
                        limite,
                        tamanho / 2
                    );

                if (tamanhoMaximo < 1) {
                    return;
                }

                int espessura = 0;

                for (
                    int i = 0;
                    i < tamanhoMaximo;
                    ++i
                ) {
                    cv::Rect faixa;

                    if (horizontal) {
                        const int y =
                            inicio
                                ? i
                                : arquivo.rows - 1 - i;

                        faixa =
                            cv::Rect(
                                0,
                                y,
                                arquivo.cols,
                                1
                            );
                    }
                    else {
                        const int x =
                            inicio
                                ? i
                                : arquivo.cols - 1 - i;

                        faixa =
                            cv::Rect(
                                x,
                                0,
                                1,
                                arquivo.rows
                            );
                    }

                    std::vector<double> medias;
                    double maiorDesvio = 0.0;

                    estatisticas(
                        arquivo(faixa),
                        medias,
                        maiorDesvio
                    );

                    if (maiorDesvio > desvio) {
                        break;
                    }

                    ++espessura;
                }

                if (espessura < 1) {
                    return;
                }

                const int posicaoInterior =
                    inicio
                        ? espessura
                        : tamanho -
                          espessura -
                          1;

                if (
                    posicaoInterior < 0 ||
                    posicaoInterior >= tamanho
                ) {
                    return;
                }

                cv::Rect regiaoBorda;
                cv::Rect faixaBorda;
                cv::Rect faixaInterna;

                if (horizontal) {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  arquivo.cols,
                                  espessura
                              )
                            : cv::Rect(
                                  0,
                                  arquivo.rows -
                                      espessura,
                                  arquivo.cols,
                                  espessura
                              );

                    faixaBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  espessura - 1,
                                  arquivo.cols,
                                  1
                              )
                            : cv::Rect(
                                  0,
                                  arquivo.rows -
                                      espessura,
                                  arquivo.cols,
                                  1
                              );

                    faixaInterna =
                        cv::Rect(
                            0,
                            posicaoInterior,
                            arquivo.cols,
                            1
                        );
                }
                else {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  espessura,
                                  arquivo.rows
                              )
                            : cv::Rect(
                                  arquivo.cols -
                                      espessura,
                                  0,
                                  espessura,
                                  arquivo.rows
                              );

                    faixaBorda =
                        inicio
                            ? cv::Rect(
                                  espessura - 1,
                                  0,
                                  1,
                                  arquivo.rows
                              )
                            : cv::Rect(
                                  arquivo.cols -
                                      espessura,
                                  0,
                                  1,
                                  arquivo.rows
                              );

                    faixaInterna =
                        cv::Rect(
                            posicaoInterior,
                            0,
                            1,
                            arquivo.rows
                        );
                }

                std::vector<double> mediasBorda;
                std::vector<double> mediasInternas;

                double desvioBorda = 0.0;
                double desvioInterno = 0.0;

                estatisticas(
                    arquivo(faixaBorda),
                    mediasBorda,
                    desvioBorda
                );

                estatisticas(
                    arquivo(faixaInterna),
                    mediasInternas,
                    desvioInterno
                );

                const double diferencaCor =
                    diferenca(
                        mediasBorda,
                        mediasInternas
                    );

                const bool transicao =
                    diferencaCor > desvio ||
                    desvioInterno >
                        desvioBorda + desvio;

                if (
                    desvioBorda <= desvio &&
                    transicao
                ) {
                    mascaraProtecao(
                        regiaoBorda
                    ).setTo(255);
                }
            };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat epsilon;

    cv::GaussianBlur(
        arquivo,
        epsilon,
        cv::Size(3, 3),
        0
    );

    cv::Mat nitidezResultado;

    cv::addWeighted(
        arquivo,
        1.0 + alfa,
        epsilon,
        -alfa,
        0,
        nitidezResultado
    );

    if (!borda) {
        return nitidezResultado;
    }

    resultado = arquivo.clone();

    cv::Mat mascaraAplicacao;

    cv::bitwise_not(
        mascaraProtecao,
        mascaraAplicacao
    );

    nitidezResultado.copyTo(
        resultado,
        mascaraAplicacao
    );

    return resultado;
}

cv::Mat Render::desfocar(
    const cv::Mat& arquivo,
    int gama
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    int ksize = gama;

    if (ksize <= 0) {
        ksize = 1;
    }
    else if (ksize % 2 == 0) {
        ++ksize;
    }

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(
            arquivo.size(),
            CV_8UC1
        );

    if (
        borda &&
        limite > 0 &&
        desvio >= 0.0
    ) {
        auto estatisticas =
            [&](
                const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio
            ) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                }
                else {
                    cv::split(
                        regiao,
                        canais
                    );
                }

                medias.resize(
                    canais.size()
                );

                maiorDesvio = 0.0;

                for (
                    size_t i = 0;
                    i < canais.size();
                    ++i
                ) {
                    cv::Scalar media;
                    cv::Scalar desvioCanal;

                    cv::meanStdDev(
                        canais[i],
                        media,
                        desvioCanal
                    );

                    medias[i] = media[0];

                    maiorDesvio =
                        std::max(
                            maiorDesvio,
                            desvioCanal[0]
                        );
                }
            };

        auto diferenca =
            [](
                const std::vector<double>& a,
                const std::vector<double>& b
            ) {
                if (a.size() != b.size()) {
                    return 0.0;
                }

                double maior = 0.0;

                for (
                    size_t i = 0;
                    i < a.size();
                    ++i
                ) {
                    maior =
                        std::max(
                            maior,
                            std::abs(
                                a[i] - b[i]
                            )
                        );
                }

                return maior;
            };

        auto detectar =
            [&](
                bool horizontal,
                bool inicio
            ) {
                const int tamanho =
                    horizontal
                        ? arquivo.rows
                        : arquivo.cols;

                const int tamanhoMaximo =
                    std::min(
                        limite,
                        tamanho / 2
                    );

                if (tamanhoMaximo < 1) {
                    return;
                }

                int espessura = 0;

                for (
                    int i = 0;
                    i < tamanhoMaximo;
                    ++i
                ) {
                    cv::Rect faixa;

                    if (horizontal) {
                        const int y =
                            inicio
                                ? i
                                : arquivo.rows - 1 - i;

                        faixa =
                            cv::Rect(
                                0,
                                y,
                                arquivo.cols,
                                1
                            );
                    }
                    else {
                        const int x =
                            inicio
                                ? i
                                : arquivo.cols - 1 - i;

                        faixa =
                            cv::Rect(
                                x,
                                0,
                                1,
                                arquivo.rows
                            );
                    }

                    std::vector<double> medias;
                    double maiorDesvio = 0.0;

                    estatisticas(
                        arquivo(faixa),
                        medias,
                        maiorDesvio
                    );

                    if (maiorDesvio > desvio) {
                        break;
                    }

                    ++espessura;
                }

                if (espessura < 1) {
                    return;
                }

                const int posicaoInterior =
                    inicio
                        ? espessura
                        : tamanho -
                          espessura -
                          1;

                if (
                    posicaoInterior < 0 ||
                    posicaoInterior >= tamanho
                ) {
                    return;
                }

                cv::Rect regiaoBorda;
                cv::Rect faixaBorda;
                cv::Rect faixaInterna;

                if (horizontal) {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  arquivo.cols,
                                  espessura
                              )
                            : cv::Rect(
                                  0,
                                  arquivo.rows -
                                      espessura,
                                  arquivo.cols,
                                  espessura
                              );

                    faixaBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  espessura - 1,
                                  arquivo.cols,
                                  1
                              )
                            : cv::Rect(
                                  0,
                                  arquivo.rows -
                                      espessura,
                                  arquivo.cols,
                                  1
                              );

                    faixaInterna =
                        cv::Rect(
                            0,
                            posicaoInterior,
                            arquivo.cols,
                            1
                        );
                }
                else {
                    regiaoBorda =
                        inicio
                            ? cv::Rect(
                                  0,
                                  0,
                                  espessura,
                                  arquivo.rows
                              )
                            : cv::Rect(
                                  arquivo.cols -
                                      espessura,
                                  0,
                                  espessura,
                                  arquivo.rows
                              );

                    faixaBorda =
                        inicio
                            ? cv::Rect(
                                  espessura - 1,
                                  0,
                                  1,
                                  arquivo.rows
                              )
                            : cv::Rect(
                                  arquivo.cols -
                                      espessura,
                                  0,
                                  1,
                                  arquivo.rows
                              );

                    faixaInterna =
                        cv::Rect(
                            posicaoInterior,
                            0,
                            1,
                            arquivo.rows
                        );
                }

                std::vector<double> mediasBorda;
                std::vector<double> mediasInternas;

                double desvioBorda = 0.0;
                double desvioInterno = 0.0;

                estatisticas(
                    arquivo(faixaBorda),
                    mediasBorda,
                    desvioBorda
                );

                estatisticas(
                    arquivo(faixaInterna),
                    mediasInternas,
                    desvioInterno
                );

                const double diferencaCor =
                    diferenca(
                        mediasBorda,
                        mediasInternas
                    );

                const bool transicao =
                    diferencaCor > desvio ||
                    desvioInterno >
                        desvioBorda + desvio;

                if (
                    desvioBorda <= desvio &&
                    transicao
                ) {
                    mascaraProtecao(
                        regiaoBorda
                    ).setTo(255);
                }
            };

        detectar(true, true);
        detectar(true, false);
        detectar(false, true);
        detectar(false, false);
    }

    cv::Mat resultadoDesfocado;

    cv::GaussianBlur(
        arquivo,
        resultadoDesfocado,
        cv::Size(
            ksize,
            ksize
        ),
        0
    );

    if (!borda) {
        return resultadoDesfocado;
    }

    resultado = arquivo.clone();

    cv::Mat mascaraAplicacao;

    cv::bitwise_not(
        mascaraProtecao,
        mascaraAplicacao
    );

    resultadoDesfocado.copyTo(
        resultado,
        mascaraAplicacao
    );

    return resultado;
}

cv::Mat Render::remover(
    const cv::Mat& arquivo,
    double alfa
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    double sensibilidade = alfa;

    if (sensibilidade <= 0.0) {
        sensibilidade = 40.0;
    }

    if (sensibilidade > 254.0) {
        sensibilidade = 254.0;
    }

    cv::Mat cinza;

    if (arquivo.channels() == 3) {
        cv::cvtColor(
            arquivo,
            cinza,
            cv::COLOR_BGR2GRAY
        );
    }
    else if (arquivo.channels() == 4) {
        cv::cvtColor(
            arquivo,
            cinza,
            cv::COLOR_BGRA2GRAY
        );
    }
    else if (arquivo.channels() == 1) {
        cinza = arquivo.clone();
    }
    else {
        return arquivo.clone();
    }

    if (cinza.depth() != CV_8U) {
        cinza.convertTo(
            cinza,
            CV_8U
        );
    }

    /*
     * O inpaint do OpenCV trabalha com imagem de 8 bits
     * de 1 ou 3 canais. Para BGRA, preservamos a imagem
     * original sem tentar aplicar inpaint diretamente
     * sobre quatro canais.
     */
    if (
        arquivo.depth() != CV_8U ||
        (
            arquivo.channels() != 1 &&
            arquivo.channels() != 3
        )
    ) {
        frameAnterior = cinza.clone();
        return arquivo.clone();
    }

    cv::Mat gradY;

    cv::Sobel(
        cinza,
        gradY,
        CV_16S,
        0,
        1,
        3
    );

    cv::convertScaleAbs(
        gradY,
        gradY
    );

    cv::Mat mascara;

    cv::threshold(
        gradY,
        mascara,
        sensibilidade,
        255,
        cv::THRESH_BINARY
    );

    cv::morphologyEx(
        mascara,
        mascara,
        cv::MORPH_OPEN,
        cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(40, 1)
        )
    );

    if (
        !frameAnterior.empty() &&
        frameAnterior.size() == cinza.size() &&
        frameAnterior.type() == cinza.type()
    ) {
        cv::Mat diferenca;

        cv::absdiff(
            cinza,
            frameAnterior,
            diferenca
        );

        cv::threshold(
            diferenca,
            diferenca,
            std::max(
                8.0,
                sensibilidade * 0.35
            ),
            255,
            cv::THRESH_BINARY
        );

        cv::morphologyEx(
            diferenca,
            diferenca,
            cv::MORPH_OPEN,
            cv::getStructuringElement(
                cv::MORPH_RECT,
                cv::Size(3, 1)
            )
        );

        cv::Mat mascaraTemporaria;

        cv::bitwise_and(
            mascara,
            diferenca,
            mascaraTemporaria
        );

        cv::bitwise_or(
            mascara,
            mascaraTemporaria,
            mascara
        );
    }

    int espessura = 3;

    if (sensibilidade < 30.0) {
        espessura = 5;
    }

    cv::dilate(
        mascara,
        mascara,
        cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(
                espessura,
                espessura
            )
        )
    );

    cv::Mat resultadoAtual;

    cv::inpaint(
        arquivo,
        mascara,
        resultadoAtual,
        static_cast<double>(espessura),
        cv::INPAINT_NS
    );

    resultado = arquivo.clone();

    resultadoAtual.copyTo(
        resultado,
        mascara
    );

    frameAnterior = cinza.clone();

    return resultado;
}

cv::Mat Render::limpar(
    const cv::Mat& arquivo,
    float beta
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (beta > 10.0f) {
        beta = 10.0f;
    }

    if (beta < 0.0f) {
        beta = 0.0f;
    }

    if (
        arquivo.channels() != 3 ||
        arquivo.depth() != CV_8U
    ) {
        return arquivo.clone();
    }

    if (beta == 0.0f) {
        return arquivo.clone();
    }

    cv::Mat filtrado;

    cv::fastNlMeansDenoisingColored(
        arquivo,
        filtrado,
        beta,
        beta,
        7,
        21
    );

    cv::Mat cinza;

    cv::cvtColor(
        arquivo,
        cinza,
        cv::COLOR_BGR2GRAY
    );

    cv::Mat gradX;
    cv::Mat gradY;
    cv::Mat gradiente;

    cv::Sobel(
        cinza,
        gradX,
        CV_32F,
        1,
        0,
        3
    );

    cv::Sobel(
        cinza,
        gradY,
        CV_32F,
        0,
        1,
        3
    );

    cv::magnitude(
        gradX,
        gradY,
        gradiente
    );

    cv::threshold(
        gradiente,
        gradiente,
        20.0,
        255.0,
        cv::THRESH_BINARY
    );

    gradiente.convertTo(
        gradiente,
        CV_8UC1
    );

    cv::dilate(
        gradiente,
        gradiente,
        cv::getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(3, 3)
        )
    );

    resultado = filtrado;

    arquivo.copyTo(
        resultado,
        gradiente
    );

    return resultado;
}

cv::Mat Render::brilho(
    const cv::Mat& arquivo,
    double alfa
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa =
        (alfa - 50.0) * 2.54;

    arquivo.convertTo(
        resultado,
        -1,
        1.0,
        alfa
    );

    return resultado;
}

cv::Mat Render::contraste(
    const cv::Mat& arquivo,
    double alfa
) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa /= 50.0;

    arquivo.convertTo(
        resultado,
        -1,
        alfa,
        0
    );

    return resultado;
}

cv::Mat Render::cores(
    const cv::Mat& arquivo,
    double alfa,
    int gama
) {
    if (
        arquivo.empty() ||
        arquivo.depth() != CV_8U ||
        (
            arquivo.channels() != 3 &&
            arquivo.channels() != 4
        )
    ) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa /= 50.0;

    const int canalAlvo =
        gama - 1;

    if (
        canalAlvo < 0 ||
        canalAlvo > 2
    ) {
        return arquivo.clone();
    }

    cv::Mat hsv;

    if (arquivo.channels() == 4) {
        cv::Mat bgr;

        cv::cvtColor(
            arquivo,
            bgr,
            cv::COLOR_BGRA2BGR
        );

        cv::cvtColor(
            bgr,
            hsv,
            cv::COLOR_BGR2HSV
        );
    }
    else {
        cv::cvtColor(
            arquivo,
            hsv,
            cv::COLOR_BGR2HSV
        );
    }

    cv::Mat processada =
        arquivo.clone();

    for (
        int i = 0;
        i < processada.rows;
        ++i
    ) {
        for (
            int j = 0;
            j < processada.cols;
            ++j
        ) {
            const cv::Vec3b& cor =
                hsv.at<cv::Vec3b>(
                    i,
                    j
                );

            cv::Vec3b original;

            if (arquivo.channels() == 3) {
                original =
                    arquivo.at<cv::Vec3b>(
                        i,
                        j
                    );
            }
            else {
                const cv::Vec4b& pixel =
                    arquivo.at<cv::Vec4b>(
                        i,
                        j
                    );

                original =
                    cv::Vec3b(
                        pixel[0],
                        pixel[1],
                        pixel[2]
                    );
            }

            const uchar max_val =
                std::max({
                    original[0],
                    original[1],
                    original[2]
                });

            const uchar min_val =
                std::min({
                    original[0],
                    original[1],
                    original[2]
                });

            if (cor[1] <= 25) {
                continue;
            }

            if (
                (max_val - min_val) < 20
            ) {
                continue;
            }

            if (
                max_val >= 245 &&
                (max_val - min_val) < 35
            ) {
                continue;
            }

            if (arquivo.channels() == 3) {
                auto& pixel =
                    processada.at<cv::Vec3b>(
                        i,
                        j
                    );

                pixel[canalAlvo] =
                    cv::saturate_cast<uchar>(
                        pixel[canalAlvo] *
                        alfa
                    );
            }
            else {
                auto& pixel =
                    processada.at<cv::Vec4b>(
                        i,
                        j
                    );

                pixel[canalAlvo] =
                    cv::saturate_cast<uchar>(
                        pixel[canalAlvo] *
                        alfa
                    );
            }
        }
    }

    resultado = processada;

    return resultado;
}

cv::Mat Render::cinzas(
    const cv::Mat& arquivo,
    double alfa
) {
    if (
        arquivo.empty() ||
        arquivo.depth() != CV_8U ||
        (
            arquivo.channels() != 3 &&
            arquivo.channels() != 4
        )
    ) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    cv::Mat hsv;

    if (arquivo.channels() == 4) {
        cv::Mat bgr;

        cv::cvtColor(
            arquivo,
            bgr,
            cv::COLOR_BGRA2BGR
        );

        cv::cvtColor(
            bgr,
            hsv,
            cv::COLOR_BGR2HSV
        );
    }
    else {
        cv::cvtColor(
            arquivo,
            hsv,
            cv::COLOR_BGR2HSV
        );
    }

    double fator;

    if (alfa <= 50.0) {
        fator =
            alfa / 50.0;
    }
    else {
        fator =
            1.0 +
            (alfa - 50.0) / 50.0;
    }

    for (
        int i = 0;
        i < hsv.rows;
        ++i
    ) {
        for (
            int j = 0;
            j < hsv.cols;
            ++j
        ) {
            auto& pixel =
                hsv.at<cv::Vec3b>(
                    i,
                    j
                );

            pixel[1] =
                cv::saturate_cast<uchar>(
                    pixel[1] * fator
                );
        }
    }

    cv::Mat bgrResultado;

    cv::cvtColor(
        hsv,
        bgrResultado,
        cv::COLOR_HSV2BGR
    );

    if (arquivo.channels() == 4) {
        std::vector<cv::Mat> canais;

        cv::split(
            arquivo,
            canais
        );

        cv::Mat canaisBGR[3];

        cv::split(
            bgrResultado,
            canaisBGR
        );

        std::vector<cv::Mat> resultadoCanais = {
            canaisBGR[0],
            canaisBGR[1],
            canaisBGR[2],
            canais[3]
        };

        cv::merge(
            resultadoCanais,
            resultado
        );
    }
    else {
        resultado = bgrResultado;
    }

    return resultado;
}

void Render::camera(
    int dispositivo
) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    frameAnterior.release();
    regiaoAnterior = cv::Rect();

    if (
        !leitor.open(
            dispositivo,
            cv::CAP_DSHOW
        )
    ) {
        std::cerr
            << "Não foi possível acessar a câmera."
            << std::endl;
    }
}

void Render::midia(
    const std::string& arquivo
) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    frameAnterior.release();
    regiaoAnterior = cv::Rect();

    if (!leitor.open(arquivo)) {
        std::cerr
            << "Erro ao abrir o arquivo."
            << std::endl;
    }
}

void Render::janela() {
    if (!leitor.isOpened()) {
        return;
    }

    cv::namedWindow(
        "Reprodutor",
        cv::WINDOW_AUTOSIZE
    );

    bool localPausar = false;

    cv::HOGDescriptor detectorPessoa;

    detectorPessoa.setSVMDetector(
        cv::HOGDescriptor::
            getDefaultPeopleDetector()
    );

    while (true) {
        if (!localPausar) {
            leitor >> imagem;

            if (imagem.empty()) {
                break;
            }

            cv::Mat cinza;

            if (imagem.channels() == 3) {
                cv::cvtColor(
                    imagem,
                    cinza,
                    cv::COLOR_BGR2GRAY
                );
            }
            else if (imagem.channels() == 4) {
                cv::cvtColor(
                    imagem,
                    cinza,
                    cv::COLOR_BGRA2GRAY
                );
            }
            else if (imagem.channels() == 1) {
                cinza = imagem.clone();
            }
            else {
                break;
            }

            cv::Mat movimento;

            if (
                !frameAnterior.empty() &&
                frameAnterior.size() ==
                    cinza.size() &&
                frameAnterior.type() ==
                    cinza.type()
            ) {
                cv::absdiff(
                    cinza,
                    frameAnterior,
                    movimento
                );

                cv::GaussianBlur(
                    movimento,
                    movimento,
                    cv::Size(5, 5),
                    0
                );

                cv::threshold(
                    movimento,
                    movimento,
                    20,
                    255,
                    cv::THRESH_BINARY
                );

                cv::morphologyEx(
                    movimento,
                    movimento,
                    cv::MORPH_OPEN,
                    cv::getStructuringElement(
                        cv::MORPH_RECT,
                        cv::Size(3, 3)
                    )
                );

                cv::morphologyEx(
                    movimento,
                    movimento,
                    cv::MORPH_CLOSE,
                    cv::getStructuringElement(
                        cv::MORPH_RECT,
                        cv::Size(5, 5)
                    )
                );
            }

            cv::Mat suavizada;

            cv::GaussianBlur(
                cinza,
                suavizada,
                cv::Size(5, 5),
                0
            );

            const cv::Scalar media =
                cv::mean(suavizada);

            const double inferior =
                std::max(
                    0.0,
                    media[0] * 0.33
                );

            const double superior =
                std::min(
                    255.0,
                    media[0] * 1.33
                );

            cv::Mat bordas;

            cv::Canny(
                suavizada,
                bordas,
                inferior,
                superior
            );

            std::vector<
                std::vector<cv::Point>
            > contornos;

            cv::findContours(
                bordas,
                contornos,
                cv::RETR_EXTERNAL,
                cv::CHAIN_APPROX_SIMPLE
            );

            double maiorArea = 0.0;

            std::vector<cv::Point> documento;

            for (
                const auto& contorno :
                contornos
            ) {
                const double area =
                    cv::contourArea(
                        contorno
                    );

                if (
                    area <
                    cinza.total() * 0.08
                ) {
                    continue;
                }

                const double perimetro =
                    cv::arcLength(
                        contorno,
                        true
                    );

                if (perimetro <= 0.0) {
                    continue;
                }

                std::vector<cv::Point>
                    aproximado;

                cv::approxPolyDP(
                    contorno,
                    aproximado,
                    perimetro * 0.02,
                    true
                );

                if (
                    aproximado.size() != 4 ||
                    !cv::isContourConvex(
                        aproximado
                    )
                ) {
                    continue;
                }

                const cv::Rect caixa =
                    cv::boundingRect(
                        aproximado
                    );

                if (
                    caixa.width <
                        cinza.cols * 0.25 ||
                    caixa.height <
                        cinza.rows * 0.25
                ) {
                    continue;
                }

                const double proporcao =
                    static_cast<double>(
                        caixa.width
                    ) /
                    static_cast<double>(
                        caixa.height
                    );

                if (
                    proporcao < 0.35 ||
                    proporcao > 3.0
                ) {
                    continue;
                }

                const double retangularidade =
                    area /
                    static_cast<double>(
                        caixa.area()
                    );

                if (
                    retangularidade < 0.60
                ) {
                    continue;
                }

                if (area > maiorArea) {
                    maiorArea = area;
                    documento = aproximado;
                }
            }

            cv::Mat digitalizacao;

            if (documento.size() == 4) {
                std::vector<cv::Point2f>
                    pontos;

                for (
                    const auto& ponto :
                    documento
                ) {
                    pontos.emplace_back(
                        static_cast<float>(
                            ponto.x
                        ),
                        static_cast<float>(
                            ponto.y
                        )
                    );
                }

                cv::Point2f centro(
                    0.0f,
                    0.0f
                );

                for (
                    const auto& ponto :
                    pontos
                ) {
                    centro += ponto;
                }

                centro *= 0.25f;

                std::sort(
                    pontos.begin(),
                    pontos.end(),
                    [&centro](
                        const cv::Point2f& a,
                        const cv::Point2f& b
                    ) {
                        return std::atan2(
                            a.y - centro.y,
                            a.x - centro.x
                        ) <
                        std::atan2(
                            b.y - centro.y,
                            b.x - centro.x
                        );
                    }
                );

                /*
                 * Após a ordenação angular, reorganiza os
                 * quatro pontos por posição para obter:
                 *
                 * 0 = superior esquerdo
                 * 1 = superior direito
                 * 2 = inferior direito
                 * 3 = inferior esquerdo
                 */
                std::sort(
                    pontos.begin(),
                    pontos.end(),
                    [](
                        const cv::Point2f& a,
                        const cv::Point2f& b
                    ) {
                        if (
                            std::abs(
                                a.y - b.y
                            ) > 1.0f
                        ) {
                            return a.y < b.y;
                        }

                        return a.x < b.x;
                    }
                );

                cv::Point2f superiorEsquerdo;
                cv::Point2f superiorDireito;
                cv::Point2f inferiorEsquerdo;
                cv::Point2f inferiorDireito;

                if (
                    pontos[0].x <
                    pontos[1].x
                ) {
                    superiorEsquerdo = pontos[0];
                    superiorDireito = pontos[1];
                }
                else {
                    superiorEsquerdo = pontos[1];
                    superiorDireito = pontos[0];
                }

                if (
                    pontos[2].x <
                    pontos[3].x
                ) {
                    inferiorEsquerdo = pontos[2];
                    inferiorDireito = pontos[3];
                }
                else {
                    inferiorEsquerdo = pontos[3];
                    inferiorDireito = pontos[2];
                }

                std::vector<cv::Point2f>
                    origem = {
                        superiorEsquerdo,
                        superiorDireito,
                        inferiorDireito,
                        inferiorEsquerdo
                    };

                const double largura =
                    std::max(
                        cv::norm(
                            origem[1] -
                            origem[0]
                        ),
                        cv::norm(
                            origem[2] -
                            origem[3]
                        )
                    );

                const double altura =
                    std::max(
                        cv::norm(
                            origem[3] -
                            origem[0]
                        ),
                        cv::norm(
                            origem[2] -
                            origem[1]
                        )
                    );

                if (
                    largura > 1.0 &&
                    altura > 1.0
                ) {
                    std::vector<cv::Point2f>
                        destino = {
                            cv::Point2f(
                                0.0f,
                                0.0f
                            ),
                            cv::Point2f(
                                static_cast<float>(
                                    largura - 1.0
                                ),
                                0.0f
                            ),
                            cv::Point2f(
                                static_cast<float>(
                                    largura - 1.0
                                ),
                                static_cast<float>(
                                    altura - 1.0
                                )
                            ),
                            cv::Point2f(
                                0.0f,
                                static_cast<float>(
                                    altura - 1.0
                                )
                            )
                        };

                    cv::Mat perspectiva =
                        cv::getPerspectiveTransform(
                            origem,
                            destino
                        );

                    cv::warpPerspective(
                        imagem,
                        digitalizacao,
                        perspectiva,
                        cv::Size(
                            static_cast<int>(
                                largura
                            ),
                            static_cast<int>(
                                altura
                            )
                        )
                    );

                    if (!digitalizacao.empty()) {
                        cv::Mat scannerCinza;

                        if (
                            digitalizacao.channels() ==
                            3
                        ) {
                            cv::cvtColor(
                                digitalizacao,
                                scannerCinza,
                                cv::COLOR_BGR2GRAY
                            );
                        }
                        else if (
                            digitalizacao.channels() ==
                            4
                        ) {
                            cv::cvtColor(
                                digitalizacao,
                                scannerCinza,
                                cv::COLOR_BGRA2GRAY
                            );
                        }
                        else {
                            scannerCinza =
                                digitalizacao.clone();
                        }

                        if (
                            scannerCinza.depth() ==
                            CV_8U
                        ) {
                            cv::GaussianBlur(
                                scannerCinza,
                                scannerCinza,
                                cv::Size(3, 3),
                                0
                            );

                            cv::adaptiveThreshold(
                                scannerCinza,
                                digitalizacao,
                                255,
                                cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                cv::THRESH_BINARY,
                                21,
                                5
                            );

                            cv::polylines(
                                imagem,
                                documento,
                                true,
                                cv::Scalar(
                                    0,
                                    255,
                                    0
                                ),
                                2
                            );
                        }
                    }
                }
            }

            std::vector<cv::Rect> pessoas;

            if (
                imagem.channels() == 3 &&
                imagem.depth() == CV_8U
            ) {
                detectorPessoa.detectMultiScale(
                    imagem,
                    pessoas,
                    1.05,
                    3,
                    0,
                    cv::Size(64, 128)
                );
            }

            bool movimentoHumano = false;

            if (!movimento.empty()) {
                std::vector<
                    std::vector<cv::Point>
                > movimentos;

                cv::findContours(
                    movimento,
                    movimentos,
                    cv::RETR_EXTERNAL,
                    cv::CHAIN_APPROX_SIMPLE
                );

                for (
                    const auto& contorno :
                    movimentos
                ) {
                    if (
                        cv::contourArea(
                            contorno
                        ) <
                        cinza.total() * 0.001
                    ) {
                        continue;
                    }

                    const cv::Rect regiaoMovimento =
                        cv::boundingRect(
                            contorno
                        );

                    for (
                        const auto& pessoa :
                        pessoas
                    ) {
                        if (
                            (
                                regiaoMovimento &
                                pessoa
                            ).area() >
                            pessoa.area() * 0.05
                        ) {
                            movimentoHumano = true;
                            break;
                        }
                    }

                    if (movimentoHumano) {
                        break;
                    }
                }
            }

            for (
                const auto& pessoa :
                pessoas
            ) {
                cv::rectangle(
                    imagem,
                    pessoa,
                    cv::Scalar(
                        255,
                        0,
                        0
                    ),
                    2
                );
            }

            if (movimentoHumano) {
                cv::putText(
                    imagem,
                    "MOVIMENTO HUMANO",
                    cv::Point(
                        10,
                        30
                    ),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.8,
                    cv::Scalar(
                        0,
                        255,
                        0
                    ),
                    2
                );
            }

            frameAnterior =
                cinza.clone();

            cv::imshow(
                "Reprodutor",
                imagem
            );
        }

        int tecla =
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
            localPausar =
                !localPausar;
        }
        else if (
            tecla == 'r' ||
            tecla == 'R'
        ) {
            leitor.set(
                cv::CAP_PROP_POS_FRAMES,
                0
            );

            frameAnterior.release();

            localPausar = false;
        }
    }

    cv::destroyWindow(
        "Reprodutor"
    );
}

Render::~Render() {
    if (leitor.isOpened()) {
        leitor.release();
    }

    if (gravador.isOpened()) {
        gravador.release();
    }
}
