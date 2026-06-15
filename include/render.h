#ifndef RENDER_H
#define RENDER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct Parametros {
    double alfa = 1.0;
    float beta = 0.0;
    int gama = 0;
    int delta = 0;
};

class Render{

public:
    Render();
    Render(const std::string& arquivo);

    cv::Mat render(int escolha, const Parametros& filtro);

    cv::Mat render(const std::vector<int>& escolhas, const std::vector<Parametros>& filtros);
    void demo(const std::vector<int>& escolhas, const std::vector<Parametros>& filtros);

    cv::VideoCapture leitor;

    void camera(int dispositivo);
    void midia(const std::string& arquivo);
    void recorte(int x, int y);
    void janela();

    ~Render();

private:
    cv::Mat imagem;
    cv::Mat resultado;
    cv::VideoWriter gravador;

    int esquerda = 0;
    int topo = 0;

    cv::Mat girar(const cv::Mat& arquivo, double alfa, int gama);
    cv::Mat recortar(const cv::Mat& arquivo, int gama, int delta);
    cv::Mat nitidez(const cv::Mat& arquivo, double alfa);
    cv::Mat desfocar(const cv::Mat& arquivo, int gama);
    cv::Mat remover(const cv::Mat& arquivo, int gama);
    cv::Mat limpar(const cv::Mat& arquivo, float beta);
    cv::Mat brilho(const cv::Mat& arquivo, double alfa);
    cv::Mat contraste(const cv::Mat& arquivo, double alfa);
    cv::Mat cores(const cv::Mat& arquivo, double alfa, int gama);
    cv::Mat cinzas(const cv::Mat& arquivo, double alfa);
};

#endif