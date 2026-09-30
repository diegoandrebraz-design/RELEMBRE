#include "../include/render.h"
#include <iostream>
#include <algorithm>

Render::Render() = default;

Render::Render(const std::string& arquivo) {
    leitor.open(arquivo);

    if (!leitor.isOpened()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
    }
}

cv::Mat Render::render(int escolha, const Parametros& filtro) {
    leitor >> imagem;
    if (imagem.empty()) return {};

    switch (escolha) {
    case 1: resultado = girar(imagem, filtro.alfa, filtro.gama); break;
    case 2: resultado = recortar(imagem, filtro.gama, filtro.delta); break;
    case 3: resultado = granular(imagem, filtro.alfa); break;
    case 4: resultado = nitidez(imagem, filtro.alfa); break;
    case 5: resultado = desfocar(imagem, filtro.gama); break;
    case 6: resultado = remover(imagem, filtro.alfa); break;
    case 7: resultado = limpar(imagem, filtro.beta); break;
    case 8: resultado = brilho(imagem, filtro.alfa); break;
    case 9: resultado = contraste(imagem, filtro.alfa); break;
    case 10: resultado = cores(imagem, filtro.alfa, filtro.gama); break;
    case 11: resultado = cinzas(imagem, filtro.alfa); break;
    default:
        std::cout << "Opção invalida!\n";
        resultado.release();
        break;
    }

    if (gravador.isOpened() && !resultado.empty()) {
        gravador.write(resultado);
    }

    return resultado;
}

cv::Mat Render::render(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    leitor >> imagem;
    if (imagem.empty()) return {};

    cv::Mat processada = imagem.clone();

    for (size_t i = 0; i < escolhas.size(); ++i) {
        switch (escolhas[i]) {
        case 1: processada = girar(processada, filtro[i].alfa, filtro[i].gama); break;
        case 2: processada = recortar(processada, filtro[i].gama, filtro[i].delta); break;
        case 3: processada = granular(processada, filtro[i].alfa); break;
        case 4: processada = nitidez(processada, filtro[i].alfa); break;
        case 5: processada = desfocar(processada, filtro[i].gama); break;
        case 6: processada = remover(processada, filtro[i].alfa); break;
        case 7: processada = limpar(processada, filtro[i].beta); break;
        case 8: processada = brilho(processada, filtro[i].alfa); break;
        case 9: processada = contraste(processada, filtro[i].alfa); break;
        case 10: processada = cores(processada, filtro[i].alfa, filtro[i].gama); break;
        case 11: processada = cinzas(processada, filtro[i].alfa); break;
        default:
            std::cout << "Opção invalida!\n";
            return {};
        }
    }

    return processada;
}

cv::Mat Render::comparar(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro) {
    bool camera = (leitor.get(cv::CAP_PROP_FRAME_COUNT) <= 0);
    bool pausado = false;

    cv::namedWindow("Comparacao Lado a Lado", cv::WINDOW_NORMAL);

    while (true) {
        if (!pausado) {
            cv::Mat processado = render(escolhas, filtro);

            if (processado.empty()) {
                if (imagem.empty() && camera) {
                    cv::waitKey(10);
                    continue;
                }

                break;
            }

            cv::Mat original = imagem.clone();

            if (original.empty()) {
                if (camera) {
                    cv::waitKey(10);
                    continue;
                }

                break;
            }

            if (original.size() != processado.size()) {
                cv::resize(original, original, processado.size());
            }

            if (processado.channels() == 1 && original.channels() == 3) {
                cv::cvtColor(processado, processado, cv::COLOR_GRAY2BGR);
            } else if (processado.channels() == 3 && original.channels() == 1) {
                cv::cvtColor(original, original, cv::COLOR_GRAY2BGR);
            }

            cv::hconcat(original, processado, resultado);

            int telaLargura = GetSystemMetrics(SM_CXSCREEN);
            int telaAltura = GetSystemMetrics(SM_CYSCREEN);

            int larguraMaxima = static_cast<int>(telaLargura * 0.90);
            int alturaMaxima = static_cast<int>(telaAltura * 0.80);

            double escalaLargura =
                static_cast<double>(larguraMaxima) / resultado.cols;

            double escalaAltura =
                static_cast<double>(alturaMaxima) / resultado.rows;

            double escala = std::min(escalaLargura, escalaAltura);

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
            } else {
                exibicao = resultado;
            }

            cv::resizeWindow(
                "Comparacao Lado a Lado",
                exibicao.cols,
                exibicao.rows
            );

            cv::imshow("Comparacao Lado a Lado", exibicao);
        }

        int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 'C' || tecla == 27) {
            break;
        }

        if (tecla == 'p' || tecla == 'P') {
            pausado = !pausado;
        }

        if (tecla == 'r' || tecla == 'R') {
            if (!camera) {
                leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                pausado = false;
            }
        }
    }

    cv::destroyWindow("Comparacao Lado a Lado");

    return resultado;
}

cv::Mat Render::girar(const cv::Mat& arquivo, double alfa, int gama) {
    cv::Point2f centro(
        static_cast<float>(arquivo.cols) / 2.0f,
        static_cast<float>(arquivo.rows) / 2.0f
    );

    cv::Mat mocao = cv::getRotationMatrix2D(centro, alfa, 1.0);

    double seno = std::abs(mocao.at<double>(0, 1));
    double cosseno = std::abs(mocao.at<double>(0, 0));

    int novaLargura = static_cast<int>(
        arquivo.rows * seno + arquivo.cols * cosseno
    );

    int novaAltura = static_cast<int>(
        arquivo.rows * cosseno + arquivo.cols * seno
    );

    mocao.at<double>(0, 2) +=
        (novaLargura - arquivo.cols) / 2.0;

    mocao.at<double>(1, 2) +=
        (novaAltura - arquivo.rows) / 2.0;

    cv::Mat processada;

    cv::warpAffine(
        arquivo,
        processada,
        mocao,
        cv::Size(novaLargura, novaAltura),
        cv::INTER_LINEAR,
        cv::BORDER_CONSTANT,
        cv::Scalar(255, 255, 255)
    );

    if (gama == 1 || gama == 0 || gama == -1) {
        cv::flip(processada, processada, gama);
    }

    return processada;
}

cv::Mat Render::recortar(const cv::Mat& arquivo, int gama, int delta) {
    if (gama <= 1 || delta <= 1) {
        return arquivo.clone();
    }

    if (esquerda < 0) esquerda = 0;
    if (topo < 0) topo = 0;

    if (esquerda >= arquivo.cols) esquerda = arquivo.cols - 1;
    if (topo >= arquivo.rows) topo = arquivo.rows - 1;

    if (esquerda + gama > arquivo.cols) {
        gama = arquivo.cols - esquerda;
    }

    if (topo + delta > arquivo.rows) {
        delta = arquivo.rows - topo;
    }

    cv::Rect areaCorte(esquerda, topo, gama, delta);

    return arquivo(areaCorte).clone();
}

cv::Mat Render::granular(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    alfa = std::max(0.0, std::min(100.0, alfa));

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(arquivo.size(), CV_8UC1);

    if (borda && limite > 0 && desvio >= 0.0) {
        auto estatisticas =
            [&](const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                } else {
                    cv::split(regiao, canais);
                }

                medias.resize(canais.size());
                maiorDesvio = 0.0;

                for (size_t i = 0; i < canais.size(); ++i) {
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
            [](const std::vector<double>& a,
               const std::vector<double>& b) {
                double maior = 0.0;

                for (size_t i = 0; i < a.size(); ++i) {
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
            [&](bool horizontal, bool inicio) {
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

                for (int i = 0;
                     i < tamanhoMaximo;
                     ++i) {

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
                    } else {
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
                } else {
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
                }

                cv::Rect ultimaFaixa;

                if (horizontal) {
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
                } else {
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
    } else {
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

    cv::Mat resultado16 = imagem16.clone();

    cv::Mat ruidoAplicavel;

    ruidoFinal.copyTo(
        ruidoAplicavel,
        mascaraGranulado
    );

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

cv::Mat Render::nitidez(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa = alfa / 12.5;

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(
            arquivo.size(),
            CV_8UC1
        );

    if (borda && limite > 0 && desvio >= 0.0) {
        auto estatisticas =
            [&](const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                } else {
                    cv::split(regiao, canais);
                }

                medias.resize(canais.size());
                maiorDesvio = 0.0;

                for (size_t i = 0; i < canais.size(); ++i) {
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
            [](const std::vector<double>& a,
               const std::vector<double>& b) {
                if (a.size() != b.size()) {
                    return 0.0;
                }

                double maior = 0.0;

                for (size_t i = 0; i < a.size(); ++i) {
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
            [&](bool horizontal, bool inicio) {
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

                for (int i = 0;
                     i < tamanhoMaximo;
                     ++i) {

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
                    } else {
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
                        : tamanho - espessura - 1;

                if (posicaoInterior < 0 ||
                    posicaoInterior >= tamanho) {
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
                } else {
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

                if (desvioBorda <= desvio &&
                    transicao) {
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
        resultado = nitidezResultado;
        return resultado;
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

cv::Mat Render::desfocar(const cv::Mat& arquivo, int gama) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    int ksize = gama;

    if (ksize <= 0) {
        ksize = 1;
    }
    else if (ksize % 2 == 0) {
        ksize = ksize + 1;
    }

    cv::Mat mascaraProtecao =
        cv::Mat::zeros(
            arquivo.size(),
            CV_8UC1
        );

    if (borda && limite > 0 && desvio >= 0.0) {
        auto estatisticas =
            [&](const cv::Mat& regiao,
                std::vector<double>& medias,
                double& maiorDesvio) {
                std::vector<cv::Mat> canais;

                if (regiao.channels() == 1) {
                    canais.push_back(regiao);
                }
                else {
                    cv::split(regiao, canais);
                }

                medias.resize(canais.size());
                maiorDesvio = 0.0;

                for (size_t i = 0; i < canais.size(); ++i) {
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
            [](const std::vector<double>& a,
               const std::vector<double>& b) {
                if (a.size() != b.size()) {
                    return 0.0;
                }

                double maior = 0.0;

                for (size_t i = 0; i < a.size(); ++i) {
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
            [&](bool horizontal, bool inicio) {
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

                for (int i = 0;
                     i < tamanhoMaximo;
                     ++i) {

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
                        : tamanho - espessura - 1;

                if (posicaoInterior < 0 ||
                    posicaoInterior >= tamanho) {
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

                if (desvioBorda <= desvio &&
                    transicao) {
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
        cv::Size(ksize, ksize),
        0
    );

    if (!borda) {
        resultado = resultadoDesfocado;
        return resultado;
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

cv::Mat Render::limpar(const cv::Mat& arquivo, float beta) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (beta > 10.0f) {
        beta = 10.0f;
    }

    if (beta < 0.0f) {
        beta = 0.0f;
    }

    if (beta == 0.0f) {
        resultado = arquivo.clone();
        return resultado;
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

cv::Mat Render::brilho(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa = (alfa - 50.0) * 2.54;

    arquivo.convertTo(
        resultado,
        -1,
        1.0,
        alfa
    );

    return resultado;
}

cv::Mat Render::contraste(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa = alfa / 50.0;

    arquivo.convertTo(
        resultado,
        -1,
        alfa,
        0
    );

    return resultado;
}

cv::Mat Render::cores(const cv::Mat& arquivo, double alfa, int gama) {
    if (arquivo.empty() || (arquivo.channels() != 3 && arquivo.channels() != 4)) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    alfa = alfa / 50.0;

    int canalAlvo = gama - 1;

    if (canalAlvo < 0 || canalAlvo > 2) {
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
    } else {
        cv::cvtColor(
            arquivo,
            hsv,
            cv::COLOR_BGR2HSV
        );
    }

    cv::Mat processada = arquivo.clone();

    for (int i = 0; i < processada.rows; ++i) {
        for (int j = 0; j < processada.cols; ++j) {
            const cv::Vec3b& cor =
                hsv.at<cv::Vec3b>(i, j);

            const cv::Vec3b original =
                arquivo.channels() == 3
                    ? arquivo.at<cv::Vec3b>(i, j)
                    : cv::Vec3b(
                          arquivo.at<cv::Vec4b>(i, j)[0],
                          arquivo.at<cv::Vec4b>(i, j)[1],
                          arquivo.at<cv::Vec4b>(i, j)[2]
                      );

            uchar max_val =
                std::max({
                    original[0],
                    original[1],
                    original[2]
                });

            uchar min_val =
                std::min({
                    original[0],
                    original[1],
                    original[2]
                });

            if (cor[1] <= 25) {
                continue;
            }

            if ((max_val - min_val) < 20) {
                continue;
            }

            if (max_val >= 245 &&
                (max_val - min_val) < 35) {
                continue;
            }

            if (arquivo.channels() == 3) {
                auto& pixel =
                    processada.at<cv::Vec3b>(i, j);

                pixel[canalAlvo] =
                    cv::saturate_cast<uchar>(
                        pixel[canalAlvo] * alfa
                    );
            } else {
                auto& pixel =
                    processada.at<cv::Vec4b>(i, j);

                pixel[canalAlvo] =
                    cv::saturate_cast<uchar>(
                        pixel[canalAlvo] * alfa
                    );
            }
        }
    }

    resultado = processada;
    return resultado;
}

cv::Mat Render::cinzas(const cv::Mat& arquivo, double alfa) {
    if (arquivo.empty()) {
        return arquivo.clone();
    }

    if (alfa < 0.0) {
        alfa = 0.0;
    }

    if (alfa > 100.0) {
        alfa = 100.0;
    }

    cv::Mat hsv;

    cv::cvtColor(
        arquivo,
        hsv,
        cv::COLOR_BGR2HSV
    );

    double fator;

    if (alfa <= 50.0) {
        fator = alfa / 50.0;
    } else {
        fator = 1.0 + (alfa - 50.0) / 50.0;
    }

    for (int i = 0; i < hsv.rows; ++i) {
        for (int j = 0; j < hsv.cols; ++j) {
            auto& pixel =
                hsv.at<cv::Vec3b>(i, j);

            pixel[1] =
                cv::saturate_cast<uchar>(
                    pixel[1] * fator
                );
        }
    }

    cv::cvtColor(
        hsv,
        resultado,
        cv::COLOR_HSV2BGR
    );

    return resultado;
}

void Render::camera(int dispositivo) {
    if (leitor.isOpened()) {
        leitor.release();
    }

    if (!leitor.open(dispositivo, cv::CAP_DSHOW)) {
        std::cerr << "Não foi possível acessar a câmera." << std::endl;
    }
}

void Render::midia(const std::string& arquivo) {
    leitor.open(arquivo);

    if (!leitor.isOpened()) {
        std::cout << "Erro ao abrir o arquivo" << std::endl;
    }
}

void Render::janela() {
    cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);
    bool localPausar = false;

    while (true) {
        if (!localPausar) {
            leitor >> imagem;
            if (imagem.empty()) {
                break;
            }
            cv::imshow("Reprodutor", imagem);
        }

        int tecla = cv::waitKey(30) & 0xFF;

        if (tecla == 'c' || tecla == 27) {
            break;
        }
        if (tecla == 'p') {
            localPausar = !localPausar;
        }
        else if (tecla == 'r') {
            leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
            localPausar = false;
        }
    }
    cv::destroyWindow("Reprodutor");
}

Render::~Render() {
    if (leitor.isOpened())   { leitor.release(); }
    if (gravador.isOpened()) { gravador.release(); }
}
