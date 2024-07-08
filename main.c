#include "raylib.h"
#include "raymath.h"
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define screenWidth 800*2
#define screenHeight 600*2

#define CONFIG_ROTATE 0
#define MAX_NPC 8
#define LERP_ALPHA 0.05f
#define PLAYER_SPEED 16
#define MAX_CHAT_LENGTH 0xff
#define MAX_CHAT_QUEUE 8

typedef struct {
    char szChat[MAX_CHAT_LENGTH];
} KChat;

typedef struct Node {
    KChat chatMessage;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *rear;
} Queue;

typedef struct {
    int nPosX;
    int nPosY;
} KPlayer;

typedef struct {
    bool isSpawned;
    int nPosX;
    int nPosY;
    int nSpeed;
    int nHP;
    char szInfo[0xff];
    char szName[0xff];
} KNpc;

typedef struct {
    KPlayer player;
    KNpc npc[MAX_NPC];
    Queue q;
} KGame;

KGame game = {0};
Texture2D player;
Texture2D npc;
Sound fx;
Sound fx_die;
Sound npc_fx1;
Sound npc_fx2;
Sound npc_fx3;
Sound npc_fx4;
const float moveInterval = 1.0f;
float passedTime = 0.0f;
//char szChat[0xff] = {0};

void chat_post(int nOrder, char *szString) {
    DrawText(szString, 20, screenHeight - (40 * nOrder), 20, GREEN);
}

void initQueue(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
}

void enqueue(Queue *q, char *message) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    strncpy(newNode->chatMessage.szChat, message, MAX_CHAT_LENGTH);
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

KChat dequeue(Queue *q) {
    KChat emptyMessage = {"\0"};
    if (q->front == NULL) {
        return emptyMessage;
    }

    Node *temp = q->front;
    KChat chatMessage = temp->chatMessage;
    q->front = q->front->next;

    if (q->front == NULL) q->rear = NULL;

    free(temp);
    return chatMessage;
}

void displayQueue(Queue *q) {
    Node *temp = q->front;
    int nOrder = MAX_CHAT_QUEUE;
    while (temp != NULL) {
        --nOrder;
        chat_post(nOrder, temp->chatMessage.szChat);
        temp = temp->next;
        if (nOrder <= 0) dequeue(&game.q);
    }
}

void player_process() {
}

void npc_process() {
    for(int i=0; i < MAX_NPC; ++i) {
        if (!game.npc[i].isSpawned) {
            game.npc[i].isSpawned = true;
            game.npc[i].nPosX = rand() % screenWidth;
            game.npc[i].nPosY = rand() % screenHeight;
            game.npc[i].nSpeed = rand() % 100;
            game.npc[i].nHP = 100;
            sprintf(game.npc[i].szName, "Cop");
            //sprintf(szChat, "npc[%d][%s] got spawned!", i, game.npc[i].szName);
            char szTemp[MAX_CHAT_LENGTH];
            sprintf(szTemp, "npc[%d][%s] got spawned!", i, game.npc[i].szName);
            enqueue(&game.q, szTemp); // FIXME: Drop FPS
        }
        //chat_post(szChat);
        displayQueue(&game.q); // FIXME: Drop FPS
#if 1
        if (passedTime >= (moveInterval * (rand() % 10))) {
            int hit = rand() % 10 + 1;
            sprintf(game.npc[i].szInfo, "%d", hit);
            game.npc[i].nHP -= hit;
            if (game.npc[i].nHP <= 0) { 
                game.npc[i].isSpawned = false;
                Sound tmp_sound = LoadSoundAlias(fx_die);
                PlaySound(tmp_sound); // FIXME: Drop FPS
            }
            game.npc[i].nPosX = Lerp(game.npc[i].nPosX, game.player.nPosX, LERP_ALPHA);
            game.npc[i].nPosY = Lerp(game.npc[i].nPosY, game.player.nPosY, LERP_ALPHA);
            DrawLine(game.npc[i].nPosX, game.npc[i].nPosY, Lerp(game.npc[i].nPosX, game.player.nPosX, LERP_ALPHA*5), Lerp(game.npc[i].nPosY, game.player.nPosY, LERP_ALPHA*5), GREEN);
            DrawLine(game.player.nPosX, game.player.nPosY, Lerp(game.player.nPosX, game.npc[i].nPosX, LERP_ALPHA*2), Lerp(game.player.nPosY, game.npc[i].nPosY, LERP_ALPHA*2), YELLOW);
        }
#else
        Vector2 direction = {game.player.nPosX - game.npc[i].nPosX, game.player.nPosY - game.npc[i].nPosY};
        direction = Vector2Normalize(direction);
        game.npc[i].nPosX += direction.x * game.npc[i].nSpeed * GetFrameTime();
        game.npc[i].nPosY += direction.y * game.npc[i].nSpeed * GetFrameTime();
#endif

        DrawTexture(npc, game.npc[i].nPosX, game.npc[i].nPosY, WHITE);
        DrawText(game.npc[i].szInfo, game.npc[i].nPosX, game.npc[i].nPosY - (passedTime * 100), 16, RED);
        //DrawText(game.npc[i].szName, game.npc[i].nPosX, game.npc[i].nPosY - 20, 20, WHITE);
    }

    if (passedTime >= moveInterval) passedTime = 0.0f;
}

int main(void)
{
    srand(time(NULL));

    InitWindow(screenWidth, screenHeight, "raydition");
    InitAudioDevice();

    initQueue(&game.q);

    player = LoadTexture("./assets/myboss.png");
    npc = LoadTexture("./assets/ani001_at_000.png");

    fx_die = LoadSound("./assets/sound/sound_i021.wav");
    fx = LoadSound("./assets/sound/player/common/npcsound/FootStep.wav");
    npc_fx1 = LoadSound("./assets/sound/skill/qczs.wav");
    npc_fx2 = LoadSound("./assets/sound/skill/sb003.wav");
    npc_fx3 = LoadSound("./assets/sound/skill/sb004.wav");
    npc_fx4 = LoadSound("./assets/sound/skill/sb005.wav");

    int frameWidth = player.width;
    int frameHeight = player.height;

    Rectangle sourceRec = { 0.0f, 0.0f, (float)frameWidth, (float)frameHeight };
    Rectangle destRec = { screenWidth/2.0f, screenHeight/2.0f, frameWidth*2.0f, frameHeight*2.0f };
    Vector2 origin = { (float)frameWidth, (float)frameHeight };

    int rotation = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if(CONFIG_ROTATE) rotation++;

        if (IsKeyDown(KEY_W)) {
            if (!IsSoundPlaying(fx)) PlaySound(fx);
            destRec.y -=PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_S)) {
            if (!IsSoundPlaying(fx)) PlaySound(fx);
            destRec.y +=PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_A)) {
            if (!IsSoundPlaying(fx)) PlaySound(fx);
            destRec.x -=PLAYER_SPEED;
        }
        if (IsKeyDown(KEY_D)) {
            if (!IsSoundPlaying(fx)) PlaySound(fx);
            destRec.x +=PLAYER_SPEED;
        }

        if (IsKeyDown(KEY_I)) {
            Sound tmp_sound = LoadSoundAlias(npc_fx1);
            PlaySound(tmp_sound);
        }
        if (IsKeyDown(KEY_J)) {
            Sound tmp_sound = LoadSoundAlias(npc_fx2);
            PlaySound(tmp_sound);
        }
        if (IsKeyDown(KEY_K)) {
            Sound tmp_sound = LoadSoundAlias(npc_fx3);
            PlaySound(tmp_sound);
        }
        if (IsKeyDown(KEY_L)) {
            Sound tmp_sound = LoadSoundAlias(npc_fx4);
            PlaySound(tmp_sound);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            passedTime += GetFrameTime();

            game.player.nPosX = destRec.x;
            game.player.nPosY = destRec.y;

            player_process();
            npc_process();

            DrawTexturePro(player, sourceRec, destRec, origin, (float)rotation, WHITE);
            DrawFPS(20, 20);
            DrawText("(c) Duc Tran", screenWidth - 100, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    UnloadTexture(player);        // Texture unloading
    CloseWindow();                // Close window and OpenGL context

    return 0;
}
