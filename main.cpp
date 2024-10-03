//Windows
#include <windows.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")
#include <chrono>
using namespace std;
using namespace cv;

Point targetPosition;
int targetTimeLife; 
int targetMaxTime; 

// Variáveis para o "inimigo"
Point enemyPosition; // Posição do inimigo
int enemyTimeLife; // Contador de life do inimigo
int enemyMaxTime; // Tempo máximo que o inimigo fica na tela
int widthEnemy = 40;
int lenghtEnemy = 80;
bool PlayerTired = false;
bool enemyTired = false;
int enemyHits;
int enemyCooldownTime = 8; // Tempo de cooldown em segundos
int playerCooldownTime = 8;
chrono::steady_clock::time_point cooldownStartTime; // Para gravar o início do cooldown

int enemyLife = -100; // life inicial do inimigo
int maxenemyLife = -100; // life máxima do inimigo

int playerStamina = 100;
int maxPlayerStamina = 100;
int enemyStamina = 100;
int maxEnemyStamina = 100;

// Variáveis para a pílula verde
Point pillPosition; // Posição da pílula
bool showPill = false; // Flag para mostrar a pílula
const int PILL_SIZE = 15; // Tamanho da pílula

// Nome da janela
string wName = "Game";

//Windows
// Caminho para o classificador Haar
string cascade_path = "C:/Users/pvc25/Downloads/ProjetoOpen/haarcascade_frontalface_default.xml";

//Windows
string sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/punch_sound2.mp3"; // Altere para o caminho do seu arquivo de som

//Windows
string life_sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/somvida.mp3";

//Windows
string background_image = "C:/Users/pvc25/Downloads/ProjetoOpen/IMG_9643.jpg";

//Windows
string boxing_bell_sound_path = "C:/Users/pvc25/Downloads/ProjetoOpen/boxing_bell_sound2.mp3";

string gameover_path = "C:/Users/pvc25/Downloads/ProjetoOpen/game-over.mp3";

string gameover2_path = "C:/Users/pvc25/Downloads/ProjetoOpen/game-ver-38511.mp3";

string menu_path = "C:/Users/pvc25/Downloads/ProjetoOpen/menu_theme_song2.wav";

string windef_path = "C:/Users/pvc25/Downloads/ProjetoOpen/windefeat_sound.mp3";

string init_path = "C:/Users/pvc25/Downloads/ProjetoOpen/init_sound.mp3";

string back_path = "C:/Users/pvc25/Downloads/ProjetoOpen/background_music.wav";

void fadeOutMusic(int fadeDuration) {
    // Simula um fade out diminuindo a reprodução
    for (int i = fadeDuration; i > 0; i -= 100) {
        Sleep(100); // Espera 100ms

        // Para o som
        PlaySound(NULL, NULL, 0); // Para o som
        Sleep(10); // Pausa um pouco para dar a sensação de fade
    }
}

// Função para inicializar a luva (gerar nova luva)
void initializeTarget(Size frameSize) {
    int margin = 100; // Margem para evitar que a luva apareça muito perto das bordas
    
    int xPos = rand() % (frameSize.width - 2 * margin) + margin; // Posição aleatória no eixo x
    int yPos = rand() % (frameSize.height - 2 * margin) + margin; // Posição aleatória no eixo y
    targetPosition = Point(xPos, yPos); // Inicializa a posição em um local aleatório na tela


    targetTimeLife = 0; // Inicializa o contador de life
    targetMaxTime = 8; // Define o tempo máximo que a luva fica na tela (100 frames)
}

void initializeEnemy(Size frameSize) {
    int margin = 100; // Margem para evitar que o inimigo apareça muito perto das bordas
    int xPos = rand() % (frameSize.width - 2 * margin) + margin;
    int yPos = rand() % (frameSize.height - 2 * margin) + margin;
    enemyPosition = Point(xPos, yPos); // Inicializa a posição em um local aleatório na tela
    enemyTimeLife = 0; // Inicializa o contador de life
    enemyMaxTime = 8; // Define o tempo máximo que a luva fica na tela (100 frames)
}

void drawPill(Mat& frame, Mat &pill_image) {
    if (showPill) {
        int centerX = pillPosition.x;
        int centerY = pillPosition.y;

        pillPosition.y += 15;

        // Redimensiona a imagem se necessário
        Mat resizedPill;
        resize(pill_image, resizedPill, Size(40, 40)); // Redimensiona a imagem

        // Calcula a posição superior esquerda para desenhar a imagem
        Point topLeft(centerX - 50, centerY - 50); // Ajuste conforme necessário

        // Verifica se a posição está dentro dos limites da imagem
    if (topLeft.x >= 0 && topLeft.x + resizedPill.cols <= frame.cols &&
        topLeft.y >= 0 && topLeft.y + resizedPill.rows <= frame.rows) {
        
        // Se a imagem tiver um canal alfa, processe a transparência
        if (resizedPill.channels() == 4) {
            // Cria uma máscara com base no canal alfa
            Mat mask;
            cv::extractChannel(resizedPill, mask, 3); // O quarto canal (índice 3) é o alfa
            
            // Copia a imagem no frame, aplicando a máscara
            for (int y = 0; y < resizedPill.rows; y++) {
                for (int x = 0; x < resizedPill.cols; x++) {
                    // Verifica se o pixel é transparente
                    if (mask.at<uchar>(y, x) > 0) { // Se o pixel não é transparente
                        frame.at<Vec3b>(topLeft.y + y, topLeft.x + x) = 
                            Vec3b(resizedPill.at<Vec4b>(y, x)[0], // Canal B
                                  resizedPill.at<Vec4b>(y, x)[1], // Canal G
                                  resizedPill.at<Vec4b>(y, x)[2]); // Canal R
                    }
                }
            }
        } else {
            resizedPill.copyTo(frame(Rect(topLeft.x, topLeft.y, resizedPill.cols, resizedPill.rows)));
        }
    }
    }

}

// Verifica se a pílula colidiu com o rosto
bool colectPill(Rect face) {
    Rect pillRect(pillPosition.x - PILL_SIZE / 2, pillPosition.y - PILL_SIZE / 4, PILL_SIZE, PILL_SIZE / 2);
    return (face & pillRect).area() > 0; // Verifica se há interseção
}

void drawTarget(Mat& frame, Mat& enemy_image) {
    int centerX = targetPosition.x;
    int centerY = targetPosition.y;

    Mat resizedTarget;
    resize(enemy_image, resizedTarget, Size(100, 100)); // Redimensiona a imagem

    Point topLeft(centerX - 50, centerY - 50); 

    // Verifica se a posição está dentro dos limites da imagem
    if (topLeft.x >= 0 && topLeft.x + resizedTarget.cols <= frame.cols &&
        topLeft.y >= 0 && topLeft.y + resizedTarget.rows <= frame.rows) {
        
        //Transparência
        if (resizedTarget.channels() == 4) {
            Mat mask;
            extractChannel(resizedTarget, mask, 3);
            
            for (int y = 0; y < resizedTarget.rows; y++) {
                for (int x = 0; x < resizedTarget.cols; x++) {
                    if (mask.at<uchar>(y, x) > 0) { 
                        frame.at<Vec3b>(topLeft.y + y, topLeft.x + x) = 
                            Vec3b(resizedTarget.at<Vec4b>(y, x)[0], // Canal B
                                  resizedTarget.at<Vec4b>(y, x)[1], // Canal G
                                  resizedTarget.at<Vec4b>(y, x)[2]); // Canal R
                    }
                }
            }
        } else {
            resizedTarget.copyTo(frame(Rect(topLeft.x, topLeft.y, resizedTarget.cols, resizedTarget.rows)));
        }
    }
}

bool hitFace(Rect face) {
    return face.contains(targetPosition); 
}

void faceDetect(Mat& frame, CascadeClassifier& cascade, vector<Rect>& faces, bool& faceHit) {
    Mat grayFrame;
    cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
    equalizeHist(grayFrame, grayFrame);

    cascade.detectMultiScale(grayFrame, faces, 1.3, 2, 0 | CASCADE_SCALE_IMAGE, Size(110, 110));

    for (size_t i = 0; i < faces.size(); i++) {
        Rect r = faces[i];
        Scalar rectangleColor = faceHit ? Scalar(0, 0, 0) : Scalar(255, 0, 0);
        rectangle(frame, Point(cvRound(r.x), cvRound(r.y)),
        Point(cvRound((r.x + r.width - 1)), cvRound((r.y + r.height - 1))),
        rectangleColor, 3);
    }

}

void drawEnemy(Mat& frame, const Mat& enemyface_image) {

    int centerX = enemyPosition.x + 10;
    int centerY = enemyPosition.y + 30;

    int width = 140;  // Largura desejada da imagem
    int height = 140; // Altura desejada da imagem

    Mat resizedEnemy;
    resize(enemyface_image, resizedEnemy, Size(width, height)); // Redimensiona a imagem

    Point topLeft(centerX - width / 2, centerY - height / 2);


    if (topLeft.x >= 0 && topLeft.x + resizedEnemy.cols <= frame.cols &&
        topLeft.y >= 0 && topLeft.y + resizedEnemy.rows <= frame.rows) {
        
        if (resizedEnemy.channels() == 4) {
            Mat mask;
            extractChannel(resizedEnemy, mask, 3);
            for (int y = 0; y < resizedEnemy.rows; y++) {
                for (int x = 0; x < resizedEnemy.cols; x++) {
                    if (mask.at<uchar>(y, x) > 0) {
                        frame.at<Vec3b>(topLeft.y + y, topLeft.x + x) = 
                            Vec3b(resizedEnemy.at<Vec4b>(y, x)[0],
                                  resizedEnemy.at<Vec4b>(y, x)[1],
                                  resizedEnemy.at<Vec4b>(y, x)[2]);
                    }
                }
            }
        } else {
            resizedEnemy.copyTo(frame(Rect(topLeft.x, topLeft.y, resizedEnemy.cols, resizedEnemy.rows)));
        }
    }

}

bool hitEnemy(Rect boundingBox) {
    Rect enemyRect(enemyPosition.x - widthEnemy / 2, enemyPosition.y - lenghtEnemy / 2, widthEnemy, lenghtEnemy); 
    return (boundingBox & enemyRect).area() > 0;
}

void setPixel(int x, int y, Mat &img, unsigned char R, unsigned char G, unsigned char B) {
    img.data[x*3 + y*img.cols*3    ] = B;
    img.data[x*3 + y*img.cols*3 + 1] = G;
    img.data[x*3 + y*img.cols*3 + 2] = R;
}

unsigned char * getPixel(int x, int y, Mat &img) {
    return &img.data[x*3 + y*img.cols*3];
}
void drawColor(Mat &img) {
    unsigned char *P;
    int R, G, B;
    int medX = 0, medY = 0, tot = 0;

    
    for (int x = 0 ; x < img.cols; x++) {
        for (int y = 0 ; y < img.rows; y++) {
            P = getPixel(x, y, img);
            B = P[0];
            G = P[1];
            R = P[2];
            if (R > 150 && G < 50 && B < 50)
            {
                medX += x;
                medY += y;
                tot++;
                setPixel(x, y, img, 0, 255, 0);

            }
        }
    }
    if (tot > 0) {
        medX = medX / tot;
        medY = medY / tot;
        if ((medX < img.cols + 5) && (medY < img.rows + 5) && (medX > 5) && (medY > 5))
            rectangle( img, Point(medX - 5, medY - 5),
                        Point(medX + 5, medY + 5),
                       Scalar(0, 0, 255), 3);
    }

} 

void detectRed(Mat& frame, bool& inimigoHit) {
    Mat hsvFrame, mask;
    cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

    Scalar lowerRed1(0, 100, 200); 
    Scalar upperRed1(10, 255, 255); 
    Scalar lowerRed2(170, 100, 200);
    Scalar upperRed2(180, 255, 255); 

    Mat mask1, mask2;
    inRange(hsvFrame, lowerRed1, upperRed1, mask1);
    inRange(hsvFrame, lowerRed2, upperRed2, mask2); 
    mask = mask1 | mask2; 

    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > 0) {
            Rect boundingBox = boundingRect(contours[i]);
            rectangle(frame, boundingBox.tl(), boundingBox.br(), Scalar(255, 0, 0), 2);

        if(playerStamina > 0) {
            if (hitEnemy(boundingBox)) {
                cout << "Inimigo atingido!" << endl;
                // Toca o som de soco
                ShellExecute(NULL, "open", sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                enemyLife += 10; // Aplica dano ao inimigo(vida negativa)
                inimigoHit = true; 
                enemyTimeLife = enemyMaxTime;
            } 
        
        } else {
            PlayerTired = true;
        }
        }
    }
}

void checkplayerCooldown() {
    if (PlayerTired) {
	    auto currentTime = chrono::steady_clock::now();
	    auto cooldownElapsed = chrono::duration_cast<chrono::seconds>(currentTime - cooldownStartTime).count();   
	    if (cooldownElapsed >= playerCooldownTime) {
	        PlayerTired = false;
	        playerStamina = maxPlayerStamina; // Recupera a stamina do jogador
        }  
    }
}	

void drawHealthbarPlayer(cv::Mat& frame, int health, int max_health) {
    int xjoga = 0;
    int yjoga = 25;

    int font = FONT_HERSHEY_SIMPLEX; 
    double fontScaleNum = 0.5;
    double fontScaleStr = 0.5;
    Scalar color(255, 0, 0);
    int thickness = 2;
    int lineType = cv::LINE_AA; 

    int bar_width = 100;
    int bar_height = 20; 
    int x = 10;
    int y = 35; 

    float health_ratio = (float)health / max_health;
    rectangle(frame, Point(x, y), Point(x + bar_width, y + bar_height), cv::Scalar(0, 0, 0), 2);

    int current_bar_width = static_cast<int>(bar_width * health_ratio); 
    putText(frame, "Jogador", Point(xjoga, yjoga), font, fontScaleStr, color, thickness, lineType);
    rectangle(frame, Point(x, y), cv::Point(x + current_bar_width, y + bar_height), Scalar(255, 0, 0), FILLED); // Azul

    string healthText = to_string(health) + "/" + to_string(max_health); // Formata a string com a vida
    putText(frame, healthText, Point(x + bar_width + 10, y + bar_height - 5), font, fontScaleNum, color, thickness, lineType); // Desenha a vida

    // Desenhar barra de fôlego
    float stamina_ratio = (float)playerStamina / maxPlayerStamina;
    rectangle(frame, Point(x, y + bar_height + 5), Point(x + bar_width, y + bar_height + bar_height + 5), Scalar(0, 0, 0), 2);
    int current_stamina_width = static_cast<int>(bar_width * stamina_ratio);
    rectangle(frame, Point(x, y + bar_height + 5), Point(x + current_stamina_width, y + bar_height + bar_height + 5), Scalar(0, 255, 0), FILLED);
    putText(frame, to_string(playerStamina) + "/" + to_string(maxPlayerStamina), Point(x + 110, y + 20 + bar_height), font, fontScaleNum, Scalar(0, 0, 0), thickness);
}

void drawHealthbarInimigo(Mat& frame, int health, int max_health) {
    int xInimigo = 520; // Posição X do inimigo
    int yInimigo = 30;  // Posição Y do inimigo

    int font = FONT_HERSHEY_SIMPLEX; 
    double fontScaleNum = 0.5;
    double fontScaleStr = 0.5;
    Scalar color(255, 0, 0);
    int thickness = 2;
    int lineType = cv::LINE_AA; 

    int bar_width = 100; // Largura da barra
    int bar_height = 20; // Altura da barra

    // Desenhar barra de saúde do inimigo
    // Desenhar barra de saúde do inimigo
    float health_ratio = (float)health / max_health;
    putText(frame, "Oponente", Point(xInimigo, yInimigo - 10), font, fontScaleStr, color, thickness, lineType);
    rectangle(frame, Point(xInimigo, yInimigo), Point(xInimigo + bar_width, yInimigo + bar_height - 5), Scalar(0, 0, 0), 2);
    int current_health_width = static_cast<int>(bar_width * health_ratio); 
    rectangle(frame, Point(xInimigo, yInimigo), Point(xInimigo + current_health_width, yInimigo + bar_height), Scalar(0, 0, 255), cv::FILLED); // Vermelho

    string healthText = to_string(health * -1) + "/" + to_string(max_health * -1); // Formata a string com a vida
    putText(frame, healthText, Point(xInimigo - 80, yInimigo + 15), font, fontScaleNum, color, thickness, lineType); // Coloca o texto à esquerda da barra

    // Desenhar barra de fôlego do inimigo
    float stamina_ratio = (float)enemyStamina / maxEnemyStamina;
    rectangle(frame, Point(xInimigo, yInimigo + bar_height + 5), Point(xInimigo + bar_width, yInimigo + bar_height + 5 + bar_height), Scalar(0, 0, 0), 2);
    int current_stamina_width = static_cast<int>(bar_width * stamina_ratio);
    rectangle(frame, Point(xInimigo, yInimigo + bar_height + 5), Point(xInimigo + current_stamina_width, yInimigo + bar_height + 5 + bar_height), Scalar(0, 255, 0), FILLED); // Verde
    putText(frame, to_string(enemyStamina ) + "/" + to_string(maxEnemyStamina), Point(xInimigo - 80, yInimigo + bar_height + 15 + 5), font, fontScaleNum, Scalar(0, 0, 0), thickness);
}

void checkEnemyCooldown() {
    if (enemyTired) {
        auto currentTime = chrono::steady_clock::now();
        auto cooldownElapsed = chrono::duration_cast<chrono::seconds>(currentTime - cooldownStartTime).count();

        if (cooldownElapsed >= enemyCooldownTime) {
            // O tempo de cooldown terminou, o inimigo pode voltar a atacar
            enemyTired = false;
            enemyStamina = 100; // Recupera a stamina do inimigo
            cout << "Inimigo recuperado e pode atacar novamente!" << endl;
        }
    }
}


void TextMenu(Mat& frame) {
    int xGame = 179;
    int yGame = 400;
    int xSair = 179;
    int ySair = 450;

    int font = FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.5; 
    Scalar textColor(0, 0, 0); 
    Scalar boxColor1(0, 255, 0); 
    Scalar boxColor2(0, 0, 255); 
    int thickness = 2; 
    int lineType = LINE_AA;

    // Definir tamanhos para os textos
    Size textSizeGame = getTextSize("PRESS START ENTER", font, fontScale, thickness, nullptr);
    Size textSizeSair = getTextSize("BACK - ESC", font, fontScale, thickness, nullptr);

    // Aumentar a margem das caixas
    int padding = 12; // Margem adicional ao redor do texto

    // Definir uma largura fixa para as caixas
    int fixedWidth = 250; // Você pode ajustar esse valor conforme necessário

    // Calcular posições das caixas
    Rect boxGame(Point(xGame - padding, yGame - textSizeGame.height - padding),
                 Size(fixedWidth, textSizeGame.height + padding * 2));
    Rect boxSair(Point(xSair - padding, ySair - textSizeSair.height - padding),
                 Size(fixedWidth, textSizeSair.height + padding * 2));

    // Desenhar as caixas
    rectangle(frame, boxGame, boxColor1, FILLED); // Caixa para "NEW GAME"
    rectangle(frame, boxSair, boxColor2, FILLED); // Caixa para "BACK"

    // Desenhar os textos
    putText(frame, "PRESS START ENTER", Point(xGame + (fixedWidth - textSizeGame.width) / 2, yGame),
            font, fontScale, textColor, thickness, lineType);
    putText(frame, "BACK - ESC", Point(xSair + (fixedWidth - textSizeSair.width) / 2, ySair),
            font, fontScale, textColor, thickness, lineType);
}

void TextTime(Mat& frame, int& seconds, int round) {
    int xTextTime = 230;
    int yTextTime = 30;

    int XTime = 300; // Posição X no frame
    int YTime = 70; // Posição Y no frame

    int font = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.6;
    cv::Scalar color(255, 0, 0); 
    int thickness = 2;
    int lineType = cv::LINE_AA;
    string time_text = to_string(seconds); 

    string text = "Tempo - Round " + to_string(round);
    putText(frame, text, Point(xTextTime, yTextTime), font, fontScale, color, thickness, lineType);
    putText(frame, time_text, Point(XTime, YTime), font, fontScale, color, thickness, lineType);
    //cv::putText(frame, "Tempo", cv::Point(xTextTime, yTextTime), font, fontScale, color, thickness, lineType);
    //cv::putText(frame, time_text, cv::Point(XTime, YTime), font, fontScale, color, thickness, lineType);
}

void displayText(string& text, string& windowName) {
    Mat victoryFrame = Mat::zeros(600, 800, CV_8UC3);
    putText(victoryFrame, text, Point(520, 400), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Escreve no frame
    imshow(windowName, victoryFrame);
}

int main(int argc, const char** argv) {
    VideoCapture capture;
    Mat frame;
    CascadeClassifier cascade;
    double scale = 1;
    char key = 0;
    setNumThreads(1);
    int life = 100;
    int max_life = 100;
    int round_Time = 60;
    int playerVictory = 0;
    int enemyVictory = 0;
    int qtd_rounds = 1;

    Mat enemypunch_image = imread("C:/Users/pvc25/Downloads/ProjetoOpen/socoadversario.png", IMREAD_UNCHANGED);
    Mat enemyface_image = imread("C:/Users/pvc25/Downloads/ProjetoOpen/imagem cinza.png", IMREAD_UNCHANGED);
    Mat pill_image = imread("C:/Users/pvc25/Downloads/ProjetoOpen/vida.png", IMREAD_UNCHANGED);

    capture.set(CAP_PROP_FPS, 60);  

    // Carregar o classificador Haar
    if (!cascade.load(cascade_path)) {
        cout << "ERROR: Could not load classifier cascade: " << cascade_path << endl;
        return -1;
    }

    while(true) {
        PlaySound(NULL, NULL, 0);
        int life = 100;
        int max_life = 100;
        int round_Time = 100;
        int playerVictory = 0;
        int enemyVictory = 0;
        int qtd_rounds = 1;
        enemyLife = -100; // life inicial do inimigo
        maxenemyLife = -100; // life máxima do inimigo
        // Criar uma janela para exibição
        namedWindow(wName, WINDOW_NORMAL);
        resizeWindow(wName, 1024, 768);


        // Criar uma imagem de fundo
        Mat backgroundImage = imread(background_image);

        // Verificar se a imagem foi carregada corretamente
        if (backgroundImage.empty()) {
            cout << "Erro ao carregar a imagem de fundo!" << endl;
            return -1;
        }

        // Redimensionar a imagem de fundo para o tamanho da janela, se necessário
        resize(backgroundImage, backgroundImage, Size(640, 480));
        TextMenu(backgroundImage);
        PlaySound(menu_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);

        // Exibir o frame preto inicialmente
        imshow(wName, backgroundImage);
        cout << "Pressione 'ENTER' para iniciar a câmera." << endl;

        // Abrir webcam
        if (!capture.open(0)) {
            cout << "Capture from camera #0 didn't work" << endl;
            return 1;
        }

        if (capture.isOpened()) {
            cout << "Video capturing has been started ..." << endl;
            namedWindow(wName, WINDOW_NORMAL);
            
            waitKey(1000);
            vector<Rect> faces; // Para armazenar as faces detectadas
            bool faceHit = false; // Variável para rastrear se uma face foi atingida
            bool inimigoHit = false;

            // Inicializar a primeira luva
            initializeTarget(Size(640, 480));

            // Inicializar o inimigo
            initializeEnemy(Size(640, 480));
            
            
            // Inicializar a pílula verde
            srand(static_cast<unsigned int>(time(0))); // Inicializa o gerador de números aleatórios
            pillPosition = Point(0, 0);
            // Inicializa o tempo do cronômetro
            auto start_time = chrono::steady_clock::now();

    // Criar uma janela para exibição
    namedWindow(wName, WINDOW_NORMAL);


    // Criar uma imagem de fundo
    Mat backgroundImage = imread(background_image);


    // Verificar se a imagem foi carregada corretamente
    if (backgroundImage.empty()) {
        cout << "Erro ao carregar a imagem de fundo!" << endl;
        return -1;
    }

    // Redimensionar a imagem de fundo para o tamanho da janela, se necessário
    resize(backgroundImage, backgroundImage, Size(640, 480));

    TextMenu(backgroundImage);

    // Exibir o frame preto inicialmente
    imshow(wName, backgroundImage);
    cout << "Pressione 'ENTER' para iniciar a câmera." << endl;

    // Aguardar até que a tecla 'ENTER' seja pressionada
    while (true) {
        key = (char)waitKey(10);
        if (key == 27) { // ESC para sair
            fadeOutMusic(3000);
            PlaySound(NULL, NULL, 0);
            return 0;
        } 
        if (key == 13) { // ENTER para iniciar
            fadeOutMusic(3000);
            PlaySound(NULL, NULL, 0);
            break;
        }
    }

    ShellExecute(NULL, "open", boxing_bell_sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    PlaySound(back_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);

    // Abrir webcam
    if (!capture.open(0)) {
        cout << "Capture from camera #0 didn't work" << endl;
        return 1;
    }

    if (capture.isOpened()) {
        cout << "Video capturing has been started ..." << endl;
        namedWindow(wName, WINDOW_NORMAL);

        vector<Rect> faces; // Para armazenar as faces detectadas
        bool faceHit = false; // Variável para rastrear se uma face foi atingida
        bool inimigoHit = false;


        initializeTarget(Size(640, 480));
        initializeEnemy(Size(640, 480));
        ShellExecute(NULL, "open", boxing_bell_sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
        
        // Inicializar a pílula verde
        srand(static_cast<unsigned int>(time(0))); // Inicializa o gerador de números aleatórios
        pillPosition = Point(0, 0);
        // Inicializa o tempo do cronômetro
        auto start_time = chrono::steady_clock::now();
        while (1) {
            capture >> frame;
            if (frame.empty())
                break;
            drawColor(frame);
            if(qtd_rounds == 4) {
                PlaySound(NULL, NULL, 0);

                Mat victoryFrame = Mat::zeros(frame.size(), frame.type()); // Cria um frame preto
                string victoryText = "";
                if(playerVictory > enemyVictory) {
                    PlaySound(NULL, NULL, 0);
                    ShellExecute(NULL, "open", windef_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    victoryText = "VICTORY";
                } else if (playerVictory < enemyVictory) {
                    PlaySound(NULL, NULL, 0);
                    ShellExecute(NULL, "open", gameover2_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    victoryText = "GAME OVER";
                }
                ShellExecute(NULL, "open", windef_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                putText(victoryFrame, victoryText, Point(200, 240), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Escreve no frame
                imshow(wName, victoryFrame); // Mostra o frame
                waitKey(2000); // Aguarda 2 segundos
                break;
            }  

            // Inverter a imagem horizontalmente
            flip(frame, frame, 1); // 1 significa inverter horizontalmente

            if (key == 0) 
                resizeWindow(wName, static_cast<int>(frame.cols / scale), static_cast<int>(frame.rows / scale));

            // Detectar rostos no frame original
            faceDetect(frame, cascade, faces, faceHit);

            // Detectar vermelhos no frame original
            detectRed(frame, inimigoHit);

            // Verificar se a luva deve ser desenhada
            if (targetTimeLife < targetMaxTime) {
                drawTarget(frame, enemypunch_image);
            }

            // Verificar se o inimigo deve ser desenhado
            if (enemyTimeLife < enemyMaxTime) {
                drawEnemy(frame, enemyface_image);
            }

            checkEnemyCooldown();
            checkplayerCooldown();
            auto current_time = chrono::steady_clock::now();
            auto elapsed_seconds = chrono::duration_cast<chrono::seconds>(current_time - start_time).count();

            // Reduz o tempo em um segundo a cada segundo
            if (elapsed_seconds >= 1) {
                round_Time -= 1;
                if (round_Time == 0) {
                    if(life > enemyLife*(-1)) {
                        playerVictory++;
                    } else if (life < enemyLife*(-1)){
                        enemyVictory++;
                    }
                    Mat fimRoundFrame = Mat::zeros(frame.size(), frame.type());
                    string textRound = "Fim do round " + to_string(qtd_rounds);
                    putText(fimRoundFrame, textRound, Point(240, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Escreve no frame
                    imshow(wName, fimRoundFrame); // Mostra o frame
                    waitKey(4000); // Aguarda 2 segundos
                    qtd_rounds++;
                    round_Time = 50;  // Reiniciar o tempo do round
                    life = max_life;
                    enemyLife = maxenemyLife;
                    playerStamina = maxPlayerStamina;
                    enemyStamina = maxEnemyStamina;
                    ShellExecute(NULL, "open", boxing_bell_sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    waitKey(200);
                    PlaySound(back_path.c_str(), NULL, SND_FILENAME | SND_ASYNC);  
                }
                start_time = current_time;  
            }

            //Texto do tempo
            TextTime(frame, round_Time, qtd_rounds);
            //Barra de life dos jogador e Inimigo
            drawHealthbarPlayer(frame, life, max_life);
            drawHealthbarInimigo(frame, enemyLife, maxenemyLife);
            

            faceHit = !faces.empty() && hitFace(faces[0]);
            if (faceHit && !enemyTired) {
                cout << "Face atingida!" << endl;
                life -= 10; 
                enemyHits++;
                enemyStamina -= 20;

                if (life <= 0){
                        life = 0;
                        PlaySound(NULL, NULL, 0);
                        ShellExecute(NULL, "open", windef_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        Mat playerKnockoutFrame = Mat::zeros(frame.size(), frame.type()); // Cria um frame preto
                        string textRound = "Jogador nocauteado ";
                        putText(playerKnockoutFrame, textRound, Point(240, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Escreve no frame
                        imshow(wName, playerKnockoutFrame); // Mostra o frame
                        waitKey(4000); // Aguarda 2 segundos
                        break;
                }

                if (life < 25 && !showPill) {
                        pillPosition = Point(rand() % frame.cols, 0);
                        showPill = true; // Exibe a pílula
                        pillPosition.y += 5;
                }


                if (enemyStamina <= 0){
                    enemyStamina = 0;
                    enemyTired = true;
                    cooldownStartTime = chrono::steady_clock::now();

                }
                ShellExecute(NULL, "open", sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                targetTimeLife = targetMaxTime;
            }

            drawPill(frame, pill_image);

            for (Rect& face : faces) {
                if (colectPill(face)) {
                    life += 10; 
                    ShellExecute(NULL, "open", life_sound_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    if (life > max_life) life = max_life;
                    showPill = false;
                    pillPosition = Point(-100, -100); 
                    break;
                }
            }
            if (enemyTimeLife < enemyMaxTime) {
                    enemyTimeLife++; 
                } else {
                    if (enemyLife >= 0) {
                        PlaySound(NULL, NULL, 0);
                        ShellExecute(NULL, "open", windef_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        Mat enemyKnockoutFrame = Mat::zeros(frame.size(), frame.type()); // Cria um frame preto
                        string textRound = "Inimigo nocauteado ";
                        putText(enemyKnockoutFrame, textRound, Point(240, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Escreve no frame
                        imshow(wName, enemyKnockoutFrame); // Mostra o frame
                        waitKey(4000); // Aguarda 2 segundos
                        break;
                    } else {
                        enemyTimeLife++; 
                        initializeEnemy(frame.size());
                    }
                }

            if (targetTimeLife < targetMaxTime) {
                targetTimeLife++;
            } else {
                initializeTarget(Size(640, 480));
            }

            detectRed(frame, inimigoHit);
            imshow(wName, frame);

            key = (char)waitKey(10);
            if (key == 27) // Pressionar ESC
                break;
            if (getWindowProperty(wName, WND_PROP_VISIBLE) == 1)
                continue;
            }
        }
    }
    }
    capture.release();
    destroyAllWindows();

    return 0;
}