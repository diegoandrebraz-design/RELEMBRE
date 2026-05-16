#ifndef RENDER_H
#define RENDER_H

#include <opencv2/opencv.hpp>
#include <string>

class Render
{
public:
    Render(const std::string& arquivo);
    void testarOpenCV();
    ~Render();
    void namedWindow();

private:
    cv::Mat imagem;
    cv::VideoCapture leitor;
    cv::VideoWriter gravador;

    bool pausar;
    bool cancelar;
    bool reiniciar;
};

#endif
