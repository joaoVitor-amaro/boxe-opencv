//#include <windows.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <vector>
#include <random>

using namespace std;
using namespace cv;

// Variáveis para a "luva"
Point luvaPosicao;    // Posição da luva
int luvaTempoVida;    // Contador de vida da luva
int luvaTempoMaximo;  // Tempo máximo que a luva fica na tela

// Nome da janela
string wName = "Game";

// Caminho para o classificador Haar
string cascade_path = "haarcascade_frontalface_default.xml";
//Windows
//string cascade_path = "C:/Users/pvc25/Downloads/ProjetoOpen/haarcascade_frontalface_default.xml";

// Caminho para o arquivo de som - Windows
//string sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/punch_sound2.mp3"; // Altere para o caminho do seu arquivo de som

// Função para inicializar a luva (gerar nova luva)
void inicializarLuva(Size frameSize) {
    int margin = 100; // Margem para evitar que a luva apareça muito perto das bordas
    int xPos = rand() % (frameSize.width - 2 * margin) + margin; // Posição aleatória no eixo x
    int yPos = rand() % (frameSize.height - 2 * margin) + margin; // Posição aleatória no eixo y
    luvaPosicao = Point(xPos, yPos);        // Inicializa a posição em um local aleatório na tela
    luvaTempoVida = 0;                      // Inicializa o contador de vida
    luvaTempoMaximo = 4;                  // Define o tempo máximo que a luva fica na tela (100 frames)
}

// Função para desenhar a luva no frame
void desenharLuva(Mat& frame) {
    int raioLuva = 20; // Raio da luva
    circle(frame, luvaPosicao, raioLuva, Scalar(0, 0, 255), -1);  // Desenha o círculo representando a luva
}

// Verifica se a luva atingiu o rosto
bool acertouRosto(Rect face) {
    return face.contains(luvaPosicao);  // Verifica se a posição da luva está dentro do retângulo da face
}

// Função de detecção e desenho de rostos
void detectarRostos(Mat& frame, CascadeClassifier& cascade, vector<Rect>& faces, bool& faceHit) {
    Mat grayFrame;
    cvtColor(frame, grayFrame, COLOR_BGR2GRAY);  // Converte o frame para escala de cinza
    equalizeHist(grayFrame, grayFrame);          // Normaliza o histograma para melhorar o contraste

    // Detecta rostos na imagem
    cascade.detectMultiScale(grayFrame, faces, 1.3, 2, 0 | CASCADE_SCALE_IMAGE, Size(40, 40));

    // Desenha retângulos ao redor das faces detectadas
    for (size_t i = 0; i < faces.size(); i++) {
        Rect r = faces[i];
        // Se a luva atingiu a face, mude a cor do retângulo para vermelho
        Scalar rectangleColor = faceHit ? Scalar(0, 0, 255) : Scalar(255, 0, 0);
        rectangle(frame, Point(cvRound(r.x), cvRound(r.y)),
                  Point(cvRound((r.x + r.width - 1)), cvRound((r.y + r.height - 1))),
                  rectangleColor, 3);
    }

}

// Função para detectar apenas cores vermelhas bem claras
void detectarVermelhoClaro(Mat& frame) {
    Mat hsvFrame, mask;
    cvtColor(frame, hsvFrame, COLOR_BGR2HSV); // Converter para o espaço de cor HSV

    // Definir os limites para vermelho bem claro
    Scalar lowerRed1(0, 100, 200); // Limite inferior (Hue, Saturação, Valor)
    Scalar upperRed1(10, 255, 255); // Limite superior
    Scalar lowerRed2(170, 100, 200); // Limite inferior para vermelho em outra parte do círculo
    Scalar upperRed2(180, 255, 255); // Limite superior

    // Criar máscara para cores vermelhas claras
    Mat mask1, mask2;
    inRange(hsvFrame, lowerRed1, upperRed1, mask1); // Máscara para o primeiro intervalo
    inRange(hsvFrame, lowerRed2, upperRed2, mask2); // Máscara para o segundo intervalo
    mask = mask1 | mask2; // Combina as duas máscaras

    // Encontrar contornos nas áreas detectadas
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Desenhar retângulos azuis ao redor das áreas vermelhas detectadas
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > 0) {
            Rect boundingBox = boundingRect(contours[i]);
            rectangle(frame, boundingBox.tl(), boundingBox.br(), Scalar(255, 0, 0), 2); // Desenha retângulo azul
        }
    }
}

void drawHealthBar(cv::Mat& frame, int health, int max_health) {
    int xjoga = 0;
    int yjoga = 20;

    // Escolha a fonte, escala, cor, espessura e estilo
    int font = cv::FONT_HERSHEY_SIMPLEX; // Tipo de fonte
    double fontScale = 1.0;               // Escala do texto
    cv::Scalar color(0, 255, 0);          // Cor (BGR - Verde)
    int thickness = 2;                    // Espessura da linha do texto
    int lineType = cv::LINE_AA;    

    // Posição e dimensões da barra de vida
    int bar_width = 100;  // Largura da barra de vida (eixo X)
    int bar_height = 20;  // Altura da barra de vida
    int x = 10;           // Posição X no frame
    int y = 30;           // Posição Y no frame

    // Calcular a proporção da vida (percentual de vida restante)
    float health_ratio = (float)health / max_health;

    // Desenhar o contorno da barra de vida (fundo)
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + bar_width, y + bar_height), cv::Scalar(0, 0, 0), 2);

    // Desenhar a parte preenchida da barra de vida
    int current_bar_width = static_cast<int>(bar_width * health_ratio);  // Tamanho proporcional à vida
    cv::putText(frame, "Jogador", cv::Point(xjoga, yjoga), font, fontScale, color, thickness, lineType);
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + current_bar_width, y + bar_height), cv::Scalar(255, 0, 0), cv::FILLED); // Azul

}


int main(int argc, const char** argv) {
    VideoCapture capture;
    Mat frame;
    CascadeClassifier cascade;
    double scale = 1;
    char key = 0;
    setNumThreads(1);
    int vida = 100;
    int max_vida = 100;

    // Carregar o classificador Haar
    if (!cascade.load(cascade_path)) {
        cout << "ERROR: Could not load classifier cascade: " << cascade_path << endl;
        return -1;
    }

    // Abrir webcam
    if (!capture.open(0)) {
        cout << "Capture from camera #0 didn't work" << endl;
        return 1;
    }

    if (capture.isOpened()) {
        cout << "Video capturing has been started ..." << endl;
        namedWindow(wName, WINDOW_NORMAL);

        capture.set(CAP_PROP_FRAME_WIDTH, 640);
        capture.set(CAP_PROP_FRAME_HEIGHT, 480);

        vector<Rect> faces;  // Para armazenar as faces detectadas
        bool faceHit = false; // Variável para rastrear se uma face foi atingida

        // Inicializar a primeira luva
        inicializarLuva(Size(640, 480));

        while (1) {
            capture >> frame;
            if (frame.empty())
                break;

            // Inverter a imagem horizontalmente
            flip(frame, frame, 1); // 1 significa inverter horizontalmente

            if (key == 0)  // Apenas na primeira vez
                resizeWindow(wName, static_cast<int>(frame.cols / scale), static_cast<int>(frame.rows / scale));

            // Detectar rostos no frame original
            detectarRostos(frame, cascade, faces, faceHit);

            // Verificar se a luva deve ser desenhada
            if (luvaTempoVida < luvaTempoMaximo) {
                desenharLuva(frame);
            }
            drawHealthBar(frame, vida, max_vida);
            // Verificar se a luva atingiu a face
            faceHit = !faces.empty() && acertouRosto(faces[0]);
            if (faceHit) {
                cout << "Face atingida!" << endl;
                //Toca musica no linux
                system("mplayer punch_sound2.mp3 &");
                // Tocar o som usando ShellExecute
                //ShellExecute(NULL, "open", sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                
                // Reiniciar a luva
                luvaTempoVida = luvaTempoMaximo; // Definir para desaparecer imediatamente
            }

            // Incrementar o contador de tempo de vida da luva
            if (luvaTempoVida < luvaTempoMaximo) {
                luvaTempoVida++; // Incrementa o contador
            } else {
                // Se o tempo de vida acabou, inicializar uma nova luva
                inicializarLuva(frame.size());
            }

            // Detectar cores vermelhas bem claras
            detectarVermelhoClaro(frame);

            // Mostrar o frame final com rostos e a luva
            imshow(wName, frame);

            key = (char)waitKey(10);
            if (key == 27 || key == 'q' || key == 'Q')  // Pressionar ESC ou 'q' para sair
                break;
            if (getWindowProperty(wName, WND_PROP_VISIBLE) == 1)
                continue;
        }
    }

    capture.release();
    destroyAllWindows();
    return 0;
}
