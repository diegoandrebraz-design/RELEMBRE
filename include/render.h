#ifndef RENDER_H
#define RENDER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct Parametros {
    double alfa = 1.0;
    float beta = 0.0f;
    int gama = 0;
    int delta = 0;
};

class Render {
public:
    Render();
    ~Render();

    cv::Mat render(int escolha, const Parametros& filtro);

    cv::Mat render(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro);

    cv::Mat comparar(const std::vector<int>& escolhas, const std::vector<Parametros>& filtro);

    void camera(int dispositivo);
    void midia(const std::string& arquivo);
    void recorte(int x, int y);
    void janela();

    cv::VideoCapture leitor;

private:
    cv::Mat imagem;
    cv::Mat resultado;
    cv::Mat frameAnterior;

    cv::VideoWriter gravador;

    cv::Rect regiaoAnterior;

    int esquerda = 0;
    int topo = 0;

    bool borda = false;
    int limite = 0;
    double desvio = 0.0;

    static cv::Mat girar(const cv::Mat& arquivo, double alfa, int gama);

    cv::Mat recortar(const cv::Mat& arquivo, int gama, int delta);

    cv::Mat granular(const cv::Mat& arquivo, double alfa);

    cv::Mat nitidez(const cv::Mat& arquivo, double alfa);

    cv::Mat desfocar(const cv::Mat& arquivo, int gama);

    cv::Mat remover(const cv::Mat& arquivo, double alfa);

    cv::Mat limpar(const cv::Mat& arquivo, float beta);

    cv::Mat brilho(const cv::Mat& arquivo, double alfa);

    cv::Mat contraste(const cv::Mat& arquivo, double alfa);

    cv::Mat cores(const cv::Mat& arquivo, double alfa, int gama);

    cv::Mat cinzas(const cv::Mat& arquivo, double alfa);
};

#endif