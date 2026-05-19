#ifndef RENDER_H
#define RENDER_H

#include <opencv2/opencv.hpp>
#include <string>

class Render
{
public:
    Render();
    Render(const std::string& arquivo);
    cv::Mat render(const cv::Mat& arquivo, int escolha, double alfa, double beta);
    void testarOpenCV();
    void janela();
    ~Render();

private:
    cv::Mat imagem;
    cv::Mat resultado;
    cv::VideoCapture leitor;
    cv::VideoWriter gravador;

    bool pausar;
    bool cancelar;
    bool reiniciar;

    cv::Mat girar(const cv::Mat& arquivo, double alfa);
    cv::Mat recortar(const cv::Mat& arquivo, double alfa, double beta);
    cv::Mat desfocar(const cv::Mat& arquivo, double alfa);
    cv::Mat limpar(const cv::Mat& arquivo, double alfa);
    cv::Mat remover(const cv::Mat& arquivo, double alfa);
    cv::Mat contraste(const cv::Mat& arquivo, double alfa);
    cv::Mat brilho(const cv::Mat& arquivo, double alfa);
    cv::Mat cores(const cv::Mat& arquivo, double alfa, double beta);
    cv::Mat nitidez(const cv::Mat& arquivo, double alfa);
    cv::Mat cinzas(const cv::Mat& arquivo, double alfa);
};

#endif
