//#include <windows.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <unistd.h>
#include <chrono>

using namespace std;
using namespace cv;

// Variáveis para a "luva"
Point luvaPosicao; // Posição da luva
int luvaTempoVida; // Contador de vida da luva
int luvaTempoMaximo; // Tempo máximo que a luva fica na tela

// Variáveis para o "inimigo"
Point inimigoPosicao; // Posição do inimigo
int inimigoTempoVida; // Contador de vida do inimigo
int inimigoTempoMaximo; // Tempo máximo que o inimigo fica na tela

int vidaInimigo = -100; // Vida inicial do inimigo
int maxVidaInimigo = -100; // Vida máxima do inimigo

// Variáveis para a pílula verde
Point pillPosition; // Posição da pílula
bool showPill = false; // Flag para mostrar a pílula
const int PILL_SIZE = 15; // Tamanho da pílula

// Nome da janela
string wName = "Game";

//Linux
string cascade_path = "haarcascade_frontalface_default.xml";
//Windows
// Caminho para o classificador Haar
//string cascade_path = "C:/Users/pvc25/Downloads/ProjetoOpen/haarcascade_frontalface_default.xml";

// Caminho para o arquivo de som
//Linux
string sound_path = "punch_sound2.mp3";
//Windows
//string sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/punch_sound2.mp3"; // Altere para o caminho do seu arquivo de som

//Linux
string life_sound_path = "somvida.mp3";
//Windows
//string life_sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/somvida.mp3";

//Linux
string background_image = "IMG_9643.jpg";
//Windows
//string background_image = "C:/Users/pvc25/Downloads/ProjetoOpen/IMG_9643.jpg";
// Função para inicializar a luva (gerar nova luva)
void inicializarLuva(Size frameSize) {
    int margin = 100; // Margem para evitar que a luva apareça muito perto das bordas
    int xPos = rand() % (frameSize.width - 2 * margin) + margin; // Posição aleatória no eixo x
    int yPos = rand() % (frameSize.height - 2 * margin) + margin; // Posição aleatória no eixo y
    luvaPosicao = Point(xPos, yPos); // Inicializa a posição em um local aleatório na tela
    luvaTempoVida = 0; // Inicializa o contador de vida
    luvaTempoMaximo = 10; // Define o tempo máximo que a luva fica na tela (100 frames)
}

void inicializarInimigo(Size frameSize) {
    int margin = 100; // Margem para evitar que o inimigo apareça muito perto das bordas
    int xPos = rand() % (frameSize.width - 2 * margin) + margin;
    int yPos = rand() % (frameSize.height - 2 * margin) + margin;
    inimigoPosicao = Point(xPos, yPos); // Inicializa a posição em um local aleatório na tela
    inimigoTempoVida = 0; // Inicializa o contador de vida
    inimigoTempoMaximo = 10; // Define o tempo máximo que a luva fica na tela (100 frames)
}

void desenharPilula(Mat& frame) {
    if (showPill) {
        int raioPilula = 15; // Defina o tamanho do círculo da pílula
        circle(frame, pillPosition, raioPilula, Scalar(0, 255, 0), -1); // Desenha um círculo verde
    }
}

// Verifica se a pílula colidiu com o rosto
bool coletouPilula(Rect face) {
    Rect pillRect(pillPosition.x - PILL_SIZE / 2, pillPosition.y - PILL_SIZE / 4, PILL_SIZE, PILL_SIZE / 2);
    return (face & pillRect).area() > 0; // Verifica se há interseção
}

// Função para desenhar um alvo na posição da luva
void desenharAlvo(Mat& frame) {
    int centerX = luvaPosicao.x;
    int centerY = luvaPosicao.y;
    int radius = 20; // Raio do alvo

    // Desenhar os círculos do alvo
    circle(frame, Point(centerX, centerY), radius + 20, Scalar(0, 0, 0), -1); // Vermelho externo
    circle(frame, Point(centerX, centerY), radius + 10, Scalar(255, 255, 255), -1); // Branco (ajustado)
    circle(frame, Point(centerX, centerY), radius + 5, Scalar(0, 0, 0), -1); // Vermelho
    circle(frame, Point(centerX, centerY), radius, Scalar(255, 255, 255), -1); // Branco
    circle(frame, Point(centerX, centerY), 5, Scalar(0, 0, 0), -1); // Pequeno círculo vermelho no centro
}

// Verifica se a luva atingiu o rosto
bool acertouRosto(Rect face) {
    return face.contains(luvaPosicao); // Verifica se a posição da luva está dentro do retângulo da face
}

// Função de detecção e desenho de rostos
void detectarRostos(Mat& frame, CascadeClassifier& cascade, vector<Rect>& faces, bool& faceHit) {
    Mat grayFrame;
    cvtColor(frame, grayFrame, COLOR_BGR2GRAY); // Converte o frame para escala de cinza
    equalizeHist(grayFrame, grayFrame); // Normaliza o histograma para melhorar o contraste

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

/*void desenharInimigo(Mat& frame) {
    int centerX = inimigoPosicao.x;
    int centerY = inimigoPosicao.y;
    int radius = 20; // Raio do inimigo

    // Desenhar os círculos do inimigo
    circle(frame, Point(centerX, centerY), radius, Scalar(0, 0, 255), -1); // Vermelho
}*/

void desenharInimigo(Mat& frame) {
    int larguraBoneco = 40;
    int alturaBoneco = 80;

    // Cabeça (circulo)
    Point centroCabeca(inimigoPosicao.x, inimigoPosicao.y - alturaBoneco / 2 + larguraBoneco / 4);
    int raioCabeca = larguraBoneco / 4;
    circle(frame, centroCabeca, raioCabeca, Scalar(255, 0, 0), -1);

    // Corpo
    Rect corpo(inimigoPosicao.x - larguraBoneco / 4, inimigoPosicao.y - alturaBoneco / 2 + larguraBoneco / 2, larguraBoneco / 2, alturaBoneco / 2);
    rectangle(frame, corpo, Scalar(255, 0, 0), -1);

    // Braços
    Rect bracoEsq(inimigoPosicao.x - larguraBoneco / 2, inimigoPosicao.y - alturaBoneco / 2 + larguraBoneco / 2, larguraBoneco / 4, alturaBoneco / 4);
    Rect bracoDir(inimigoPosicao.x + larguraBoneco / 4, inimigoPosicao.y - alturaBoneco / 2 + larguraBoneco / 2, larguraBoneco / 4, alturaBoneco / 4);
    rectangle(frame, bracoEsq, Scalar(255, 0, 0), -1);
    rectangle(frame, bracoDir, Scalar(255, 0, 0), -1);

}

// Verifica se a luva atingiu o inimigo
bool acertouInimigo(Rect boundingBox) {
    return boundingBox.contains(inimigoPosicao); // Verifica se a posição do inimigo está dentro do retângulo da face
}

// Função para detectar apenas cores vermelhas bem claras
void detectarVermelhoClaro(Mat& frame, bool& inimigoHit) {
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

            // Verifica se o inimigo foi atingido
            if (acertouInimigo(boundingBox)) {
                inimigoHit = true; // Marca que o inimigo foi atingido
                inimigoTempoVida = inimigoTempoMaximo; // Reinicia o tempo de vida do inimigo
            }
        }
    }
}

void drawHealthBarJogador(cv::Mat& frame, int health, int max_health) {
    int xjoga = 0;
    int yjoga = 25;

    // Escolha a fonte, escala, cor, espessura e estilo
    int font = cv::FONT_HERSHEY_SIMPLEX; // Tipo de fonte
    double fontScale = 1.0; // Escala do texto
    cv::Scalar color(255, 0, 0); // Cor (BGR - Azul)
    int thickness = 2; // Espessura da linha do texto
    int lineType = cv::LINE_AA; 

    // Posição e dimensões da barra de vida
    int bar_width = 100; // Largura da barra de vida (eixo X)
    int bar_height = 20; // Altura da barra de vida
    int x = 10; // Posição X no frame
    int y = 35; // Posição Y no frame

    // Calcular a proporção da vida (percentual de vida restante)
    float health_ratio = (float)health / max_health;

    // Desenhar o contorno da barra de vida (fundo)
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + bar_width, y + bar_height), cv::Scalar(0, 0, 0), 2);

    // Desenhar a parte preenchida da barra de vida
    int current_bar_width = static_cast<int>(bar_width * health_ratio); // Tamanho proporcional à vida
    cv::putText(frame, "Jogador", cv::Point(xjoga, yjoga), font, fontScale, color, thickness, lineType);
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + current_bar_width, y + bar_height), cv::Scalar(255, 0, 0), cv::FILLED); // Azul
}

void drawHealthBarInimigo(cv::Mat& frame, int health, int max_health) {
    int xInimigo = 520;
    int yInimigo = 25;

    // Escolha a fonte, escala, cor, espessura e estilo
    int font = cv::FONT_HERSHEY_SIMPLEX; // Tipo de fonte
    double fontScale = 1.0; // Escala do texto
    cv::Scalar color(255, 0, 0); // Cor (BGR - Azul)
    int thickness = 2; // Espessura da linha do texto
    int lineType = cv::LINE_AA; 

    // Posição e dimensões da barra de vida
    int bar_width = -100; // Largura da barra de vida (eixo X)
    int bar_height = 20; // Altura da barra de vida
    int x = 625; // Posição X no frame
    int y = 35; // Posição Y no frame

    // Calcular a proporção da vida (percentual de vida restante)
    float health_ratio = (float)health / max_health;

    // Desenhar o contorno da barra de vida (fundo)
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + bar_width, y + bar_height), cv::Scalar(0, 0, 0), 2);

    // Desenhar a parte preenchida da barra de vida
    int current_bar_width = static_cast<int>(bar_width * health_ratio); // Tamanho proporcional à vida
    cv::putText(frame, "Inimigo", cv::Point(xInimigo, yInimigo), font, fontScale, color, thickness, lineType);
    cv::rectangle(frame, cv::Point(x, y), cv::Point(x + current_bar_width, y + bar_height), cv::Scalar(0, 0, 255), cv::FILLED); // Azul
}

void TextMenuPrincipal(Mat& frame) {
    int xGame = 200;
    int yGame = 400;
    int xSair = 200;
    int ySair = 450;

    // Escolha a fonte, escala, cor, espessura e estilo
    int font = FONT_HERSHEY_SIMPLEX; // Tipo de fonte
    double fontScale = 1.0; // Escala do texto
    Scalar textColor(0, 0, 0); // Cor do texto (preto)
    Scalar boxColor1(0, 255, 0); // Cor da caixa para "NEW GAME" (verde)
    Scalar boxColor2(0, 0, 255); // Cor da caixa para "BACK" (vermelho)
    int thickness = 2; // Espessura da linha do texto
    int lineType = LINE_AA;

    // Definir tamanhos para os textos
    Size textSizeGame = getTextSize("NEW GAME", font, fontScale, thickness, nullptr);
    Size textSizeSair = getTextSize("BACK", font, fontScale, thickness, nullptr);

    // Aumentar a margem das caixas
    int padding = 12; // Margem adicional ao redor do texto

    // Definir uma largura fixa para as caixas
    int fixedWidth = 200; // Você pode ajustar esse valor conforme necessário

    // Calcular posições das caixas
    Rect boxGame(Point(xGame - padding, yGame - textSizeGame.height - padding),
                 Size(fixedWidth, textSizeGame.height + padding * 2));
    Rect boxSair(Point(xSair - padding, ySair - textSizeSair.height - padding),
                 Size(fixedWidth, textSizeSair.height + padding * 2));

    // Desenhar as caixas
    rectangle(frame, boxGame, boxColor1, FILLED); // Caixa para "NEW GAME"
    rectangle(frame, boxSair, boxColor2, FILLED); // Caixa para "BACK"

    // Desenhar os textos
    putText(frame, "NEW GAME", Point(xGame + (fixedWidth - textSizeGame.width) / 2, yGame),
            font, fontScale, textColor, thickness, lineType);
    putText(frame, "BACK", Point(xSair + (fixedWidth - textSizeSair.width) / 2, ySair),
            font, fontScale, textColor, thickness, lineType);
}

void Textempo(Mat& frame, int& seconds) {
    int xTextTempo = 250;
    int yTextTempo = 25;

    int xTempo = 280; // Posição X no frame
    int yTempo = 70; // Posição Y no frame

    // Escolha a fonte, escala, cor, espessura e estilo
    int font = cv::FONT_HERSHEY_SIMPLEX; // Tipo de fonte
    double fontScale = 1.0; // Escala do texto
    cv::Scalar color(255, 0, 0); // Cor (BGR - Azul)
    int thickness = 2; // Espessura da linha do texto
    int lineType = cv::LINE_AA;
    string time_text = to_string(seconds); 

    cv::putText(frame, "Tempo", cv::Point(xTextTempo, yTextTempo), font, fontScale, color, thickness, lineType);
    cv::putText(frame, time_text, cv::Point(xTempo, yTempo), font, fontScale, color, thickness, lineType);
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
    int tempo_round = 10;
    int vitoriaJogador = 0;
    int vitoriaInimigo = 0;
    int qtd_rounds = 1;

    cout << "Vida do Inimigo: " << vidaInimigo << endl;

    // Carregar o classificador Haar
    if (!cascade.load(cascade_path)) {
        cout << "ERROR: Could not load classifier cascade: " << cascade_path << endl;
        return -1;
    }

    // Criar uma janela para exibição
    namedWindow(wName, WINDOW_AUTOSIZE);

    // Criar uma imagem de fundo
    Mat backgroundImage = imread(background_image);

    // Verificar se a imagem foi carregada corretamente
    if (backgroundImage.empty()) {
        cout << "Erro ao carregar a imagem de fundo!" << endl;
        return -1;
    }

    // Redimensionar a imagem de fundo para o tamanho da janela, se necessário
    resize(backgroundImage, backgroundImage, Size(640, 480));

    TextMenuPrincipal(backgroundImage);

    // Exibir o frame preto inicialmente
    imshow(wName, backgroundImage);
    cout << "Pressione 'ENTER' para iniciar a câmera." << endl;

    // Aguardar até que a tecla 'ENTER' seja pressionada
    while (true) {
        key = (char)waitKey(10);
        if (key == 27) { // ESC para sair
            return 0;
        } 
        if (key == 13) { // ENTER para iniciar
            break;
        }
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

        vector<Rect> faces; // Para armazenar as faces detectadas
        bool faceHit = false; // Variável para rastrear se uma face foi atingida
        bool inimigoHit = false;

        // Inicializar a primeira luva
        inicializarLuva(Size(640, 480));

        // Inicializar o inimigo
        inicializarInimigo(Size(640, 480));
        //sleep(3);
        system("mplayer boxing_bell_sound2.mp3 &");
        
        // Inicializar a pílula verde
        srand(static_cast<unsigned int>(time(0))); // Inicializa o gerador de números aleatórios
        pillPosition = Point(0, 0);
        // Inicializa o tempo do cronômetro
        auto start_time = chrono::steady_clock::now();
        while (1) {
            capture >> frame;
            if (frame.empty())
                break;

            if(qtd_rounds == 4) {
                if(vitoriaJogador > vitoriaInimigo) {
                    cout << "Jogador ganhou" << endl;
                } else if (vitoriaJogador < vitoriaInimigo) {
                    cout << "Inimigo ganhou" << endl;
                } else {
                    cout << "Empate" << endl;
                }
                break;
            }  

            // Inverter a imagem horizontalmente
            flip(frame, frame, 1); // 1 significa inverter horizontalmente

            if (key == 0) // Apenas na primeira vez
                resizeWindow(wName, static_cast<int>(frame.cols / scale), static_cast<int>(frame.rows / scale));

            // Detectar rostos no frame original
            detectarRostos(frame, cascade, faces, faceHit);

            // Detectar vermelhos no frame original
            detectarVermelhoClaro(frame, inimigoHit);

            // Verificar se a luva deve ser desenhada
            if (luvaTempoVida < luvaTempoMaximo) {
                desenharAlvo(frame);
            }

            // Verificar se o inimigo deve ser desenhado
            if (inimigoTempoVida < inimigoTempoMaximo) {
                desenharInimigo(frame);
            }
            auto current_time = chrono::steady_clock::now();
            auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();

            // Reduz o tempo em um segundo a cada segundo
            if (elapsed_seconds >= 1) {
                tempo_round -= 1;
                if (tempo_round == 0) {
                    if(vida > vidaInimigo*(-1)) {
                        vitoriaJogador++;
                    } else if (vida < vidaInimigo*(-1)){
                        vitoriaInimigo++;
                    }
                    
                    cout << "Fim de round 1" << endl;
                    Textemp(frame, tempo_round);
                    sleep(2);
                    tempo_round = 10;  // Garante que o tempo não fique negativo
                    qtd_rounds++;
                    vida = max_vida;
                    vidaInimigo = maxVidaInimigo;   
                }
                start_time = current_time;  // Reinicia o tempo de referência
            }
            //Texto do tempo
            Textempo(frame, tempo_round);
            //Barra de vida dos jogador e Inimigo
            drawHealthBarJogador(frame, vida, max_vida);
            drawHealthBarInimigo(frame, vidaInimigo, maxVidaInimigo);

            // Verificar colisão com o inimigo
            if (!faces.empty() && acertouInimigo(faces[0])) {
                cout << "Inimigo atingido!" << endl;
                system("mplayer punch_sound.mp3 &");
                vidaInimigo += 20; // Diminui vida ao acertar o inimigo
                inimigoHit = true; // Marca que o inimigo foi atingido
                inimigoTempoVida = inimigoTempoMaximo; // Reinicia o tempo de vida do inimigo
            }

            // Verificar se a luva atingiu a face
            faceHit = !faces.empty() && acertouRosto(faces[0]);
            if (faceHit) {
                cout << "Face atingida!" << endl;
                vida -= 10; 
                if (vida < 0) vida = 0; // Garantir que a vida não fique negativa

                // Verificar se a vida está abaixo de 25 para mostrar a pílula
                if (vida < 25 && !showPill) {
                    // Gerar a posição da pílula aleatoriamente
                    pillPosition = Point(rand() % frame.cols, rand() % frame.rows);
                    showPill = true; // Exibe a pílula
                }

                // Toca música no Linux
                system("mplayer punch_sound2.mp3 &");

                // Reiniciar a luva
                luvaTempoVida = luvaTempoMaximo; // Definir para desaparecer imediatamente
            }

            desenharPilula(frame);

            // Verificar colisão da pílula com o rosto
            for (Rect& face : faces) {
                if (coletouPilula(face)) {
                    vida += 10; // Aumentar vida em 15
                    // Toca o som no Linux
                    system("mplayer somvida.mp3 &");
                    if (vida > max_vida) vida = max_vida; // Não exceder o máximo
                    showPill = false; // Ocultar a pílula após coleta
                    pillPosition = Point(-100, -100); // Move a pílula para fora da tela para "remover"
                    break; // Sair do loop após coletar
                }
            }

            // Incrementar o contador de tempo de vida do inimigo
            if (inimigoTempoVida < inimigoTempoMaximo) {
                inimigoTempoVida++; // Incrementa o contador
            } else {
                // Se o tempo de vida acabou, inicializar um novo inimigo
                inicializarInimigo(frame.size());
            }

            if (vidaInimigo >= 0) {
                vidaInimigo = maxVidaInimigo; // Reinicia a vida do inimigo
                // Opcional: Reinicializar a posição ou outras características do inimigo
                inicializarInimigo(frame.size());
            }

            // Incrementar o contador de tempo de vida da luva
            if (luvaTempoVida < luvaTempoMaximo) {
                luvaTempoVida++; // Incrementa o contador
            } else {
                // Se o tempo de vida acabou, inicializar uma nova luva
                inicializarLuva(frame.size());
            }
            // Detectar cores vermelhas bem claras
            detectarVermelhoClaro(frame, inimigoHit);
            // Mostrar o frame final com rostos e a luva
            imshow(wName, frame);

            key = (char)waitKey(10);
            if (key == 27) // Pressionar ESC
                break;
            if (getWindowProperty(wName, WND_PROP_VISIBLE) == 1)
                continue;
        }
    }
    cout << vitoriaJogador << endl;
    cout << vitoriaInimigo << endl;
    capture.release();
    destroyAllWindows();
    return 0;
}
