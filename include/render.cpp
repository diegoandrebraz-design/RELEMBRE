#include "render.h"
#include <cassert>
#include <iostream>

Render::Render(const std::string& arquivo)
{
leitor.open(arquivo);
    if (!leitor.isOpened()){
       std::cout<<"Erro ao abrir o arquivo"<<std::endl;
    }
    assert(leitor.isOpened());

    pausar = false;
    cancelar = false;
    reiniciar = false;
}

Render::~Render()
{
    if (leitor.isOpened()){
        leitor.release();
    }

    if (gravador.isOpened()){
        gravador.release();
    }
}

void Render::namedWindow()
    {
        cv::Mat frame;
        cv::namedWindow("Reprodutor", cv::WINDOW_AUTOSIZE);

        while (true)
        {
            leitor >> frame;
            if (frame.empty())
            {
                break;
            }

            cv::imshow("Reprodutor", frame);
            int tecla = cv::waitKey(30);

            if (tecla=='c')
            {
                cancelar = true;
                while (cancelar)
                {
                    break;
                }
            }
            else if (tecla=='p')
            {
                pausar = !pausar;
                while (pausar)
                {
                    int teclaPausa = cv::waitKey(30);
                    if (teclaPausa == 'p')
                    {
                        pausar=!pausar;
                    }
                }
            }
            else if (tecla=='r')
            {
                reiniciar = true;
                while (reiniciar)
                {
                    leitor.set(cv::CAP_PROP_POS_FRAMES, 0);
                    reiniciar = false;
                }
            }
            {
                break;
            }
        }

        cv::destroyWindow("Reprodutor");
    }




