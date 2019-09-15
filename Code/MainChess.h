#pragma once
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <time.h>
#include <chrono>
#include <ChessEngine/ChessEngine.h>
#include <ChessEngine/GLSLProgram.h>
#include <ChessEngine/GLTexture.h>
#include <ChessEngine/Sprite.h>
#include <ChessEngine/SpriteFont.h>
#include <ChessEngine/Window.h>
#include <ChessEngine/Camera2D.h>
#include <ChessEngine/SpriteBatch.h>
#include <ChessEngine/InputManager.h>
enum class ChessState {PLAY, EXIT};

class MainChess
{
	struct SelectedPiece { //Holds information about the selected chess piece

		int type;
		int quadrant;
		int quadX;
		int quadY;
	};

public:
	MainChess();
	~MainChess();

	void run();



private:
	//Initialization Functions
	void initSystems();
	void initShaders();
	void initChessBoard();

	//Input Processing Functions
	void processInput();
	int getMouseQuad();

	//Chess Rule and Movement Functions
	void placePiece(int board[8][8], int quadrant, int pieceID);
	void placePieceXY(int board[8][8], int x, int y, int pieceID);
	void removePiece(int board[8][8], int quadrant);
	bool squareOccupied(int board[8][8], int quadrant);
	int getPieceAtQuad(int board[8][8], int quadrant);
	int getQuadX(int quadrant);
	int getQuadY(int quadrant);
	int getQuadFromXY(int x, int y);
	void selectPieceToMove(int board[8][8], int quadrant);
	void selectPieceToMoveXY(int board[8][8], int x, int y);
	int pointGain(int board[8][8], int x, int y, int wOrb);
	int isValidMove(int board[8][8], int sposX, int sposY, int dposX, int dposY, int wOrb);
	int pieceInMiddle(int board[8][8], int sposX, int sposY, int dposX, int dposY);
	int isCheckPurpose(int board[8][8], int sposx, int sposy, int dposx, int dposy, int wOrb);
	int inCheckOccur(int board[8][8]);
	int checkMate(int board[8][8], int kingBW);
	int isCheckAvoidable(int board[8][8], int sposX, int sposY, int dposX, int dposY, int xOrb);
	int isPawnAtEnd(int board[8][8]);
	int pawnPromotion(int sign, int playerAI);
	bool movePiece(int board[8][8], int destQuad);
	void movePieceXY(int board[8][8], int x, int y);
	void switchTurn();


	//Statistical Functions
	int executionTime();
	int getExecutionTime();
	void openStats();
	void closeStats();

	//Drawing Functions
	void drawChess();
	void drawPieces();
	void drawBoard();
	void drawStats();


	//Console Debugging Functions
	void printBoard(int board[][8]);

	//Looping functions that handle actions for the player and AI respectively, chessLoop handles the overall chess game loop
	void chessLoop();
	void aiLoop();


	ChessEngine::Window _window;
	int _windowWidth;
	int _windowHeight;
	int _boardWidth;
	int _boardHeight;
	int _statsWidth;
	int _statsHeight;
	ChessEngine::SpriteBatch _spriteBatch;
	ChessEngine::SpriteBatch _spriteBatchStats;
	ChessEngine::GLSLProgram _colorProgram;
	ChessEngine::Camera2D _camera2D;
	ChessEngine::SpriteFont* _spriteFont;

	//Statistical panel backdrop properties
	bool _statsOpen;
	float _statsPanelPosX;
	float _statsPanelPosY;
	float _statsPanelWidth;
	float _statsPanelHeight;

	//Execution statistical bar properties
	float _statsExecuteBarWidth;
	float _statsExecuteBarHeight;
	float _statsExecuteBarPosX;
	float _statsExecuteBarPosY;

	float _boardSquareWidth = 0.25f;
	float _boardSquareHeight = 0.25f;

	int board[8][8]; //Holds the Current Chess baord state
	
	//The highest scoring next board states 1 = highest scoring (other then the board state selected), 2 = second highest scoring board state
	int _boardScore1[8][8];
	int _boardScore2[8][8];
	int _boardScore3[8][8];

	ChessState _chessState;


	ChessEngine::InputManager _inputManager;
	
	
	int _elapsedAITime; //Holds time elapsed since beggining of AI turn
	int _totalElapsedAITime = 0;
	int _elapsedPlayerTime; //Holds time elapsed since beggining of Player turn
	int _totalElapsedPlayerTime = 0; //If this value reaches above 100 min (100000ms) in the first 40 moves  by (_executionTimeTracker / 2) == 40, then the player has used up all alloted time.

	int wOrb = -1;
	bool _playerTurn;
	int _turnCounterPlayer = 0;
	int _turnCounterAI = 0;
	std::chrono::high_resolution_clock::time_point t1, t2;
	bool _executionTimeTracker = false; //Also used to track how many turns has passed in total
	SelectedPiece selectedPiece;
	int  sposX, sposY, dposX, dposY;
	int turn = 1, valid;
	int i, j, k, x, y, index, piece;
	int arr[16][2];
	int currentPoint;
	int pointBest[5];
	int minMax[5];

};

