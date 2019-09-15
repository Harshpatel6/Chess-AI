#include "MainChess.h"
#include <ChessEngine/Sprite.h>
#include <ChessEngine/Errors.h>
#include <ChessEngine/ResourceManager.h>
#include <ChessEngine/Camera2D.h>
#include <iostream>
#include <string>
MainChess::MainChess()
{
	//Initial Properties
	_windowWidth = 1120;
	_windowHeight = 720;

	_camera2D.init(_windowWidth, _windowHeight);
	_camera2D.setPosition(glm::vec2(-360.0f, -360.0f));
	_camera2D.setScale(360.0f);


	_statsWidth = 400; //How much the stats panel will take from the total _windowWidth
	_statsHeight = 0; //How much the stats panel will take from the total _windowHeight
	_statsPanelPosX = 1.0f;
	_statsPanelPosY = 1.0f;
	_statsPanelWidth = 1.2f;
	_statsPanelHeight = -2.0f;
	_statsOpen = true;

	//Statistical Bargraph initialization
	_statsExecuteBarHeight = -0.1f;
	_statsExecuteBarWidth = 0.15f;
	_statsExecuteBarPosX = 1.1f;
	_statsExecuteBarPosY = -1.0f;

	//Player goes first (White Pieces)
	_playerTurn = true;
	
	//Chess State On
	_chessState = ChessState::PLAY;

}


MainChess::~MainChess()
{
}


void MainChess::run()
{
	initSystems();

	chessLoop();
}

//Initializes SDL(Offers timing functions, input functions, etc...) and OpenGL
void MainChess::initSystems()
{	

	ChessEngine::init(); //Initialize ChessEngine (and thus SDL)

	_window.create("Chess AI", _windowWidth, _windowHeight, 0);


	initShaders();
	_spriteBatch.init(); //General use one
	_spriteBatchStats.init(); //For sprites in the Stats window

	_spriteFont = new ChessEngine::SpriteFont("Fonts/04B_03__.ttf", 32);

	//Set intital board state
	initChessBoard();


}

//Populates the chess board with peices in their initial starting positions
void MainChess::initChessBoard() {
	_boardWidth =  _windowWidth - _statsWidth;
	_boardHeight = _windowHeight - _statsHeight;
	
	//Initialize chess board
	int i, j;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			board[i][j] = 0;
		}
	}

	//Initialize piece starting locations
	board[0][0] = board[0][7] = 4;          // ROOK
	board[7][0] = board[7][7] = -4;
	board[0][1] = board[0][6] = 3;          // Knight
	board[7][1] = board[7][6] = -3;
	board[0][2] = board[0][5] = 2;          // BISHOP
	board[7][2] = board[7][5] = -2;
	board[0][3] = 10;                        // QUEEN
	board[7][3] = -10;
	board[0][4] = 100;                       // KING
	board[7][4] = -100;

	for (j = 0; j < 8; j++)                 // PAWN
	{
		board[1][j] = 1;
	}
	for (j = 0; j < 8; j++)                 // ENEMY PAWN
	{
		board[6][j] = -1;
	}

	turn = 1;

	pointBest[0] = -1;
	pointBest[1] = 0;
	pointBest[2] = 0;
	pointBest[3] = 0;
	pointBest[4] = 0;
}

//Initializes and assigns fragmentation and vertex shader variables within the directory "Shaders/"
void MainChess::initShaders() {

	_colorProgram.compileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");
	_colorProgram.addAttribute("vertexPosition");
	_colorProgram.addAttribute("vertexColor");
	_colorProgram.addAttribute("vertexUV");
	_colorProgram.linkShaders();

}

//This functions utilizes SDL to process player inputs (Mouse Location, Keyboard Inputs)
void MainChess::processInput()
{	
	SDL_Event evnt;

	//returns 1 if there is a pending event, 0 if there are no pending events
	while (SDL_PollEvent(&evnt))
	{//If there is an event do:
		switch (evnt.type)
		{
		case SDL_QUIT: //Quits Game, exits all loops
			_chessState = ChessState::EXIT;

			break;
		case SDL_MOUSEBUTTONDOWN:
			_inputManager.keyDown(evnt.button.button);
			//Player Chess Actions------------------
			if (_playerTurn == true ) {
				if (selectedPiece.type == 0) { //If a piece is not selected
					if (squareOccupied(board, getMouseQuad()) && getPieceAtQuad(board, getMouseQuad()) < 0) { //If the square is occupied with a chess piece
						selectPieceToMove(board, getMouseQuad());//Select Piece
					}
				}
				else { //If a piece is selected
					if (movePiece(board, getMouseQuad()) == false) { //If clicking on a non-valid square when moving selected piece
						selectPieceToMove(board, getMouseQuad());//De-select piece
					}
					else {//If a piece was succesfully moved

						//Check if player Pawn is at the end
						if (isPawnAtEnd(board) == 1)
						{
							board[getQuadX(getMouseQuad())][getQuadY(getMouseQuad())] = pawnPromotion(-1, 0);
						}

					}
				}
			}
			//-----------------------
			break;
		case SDL_MOUSEBUTTONUP:
			_inputManager.keyUp(evnt.button.button);
			break;
		case SDL_MOUSEMOTION:
			_inputManager.setMouseCord(evnt.motion.x, evnt.motion.y);
			break;

		case SDL_KEYDOWN:
			_inputManager.keyDown(evnt.key.keysym.sym);
			break;

		case SDL_KEYUP:
			_inputManager.keyUp(evnt.key.keysym.sym);
			break;
		}
	}
	//Increases Stats Panel Size Properties until the given values while panel is opened
	if (_statsOpen == true) {
		if (_statsPanelPosX >= 1.1f) {
			_statsPanelPosX -= 0.1f;
		}
		if (_boardSquareWidth > 0.25f) {
			_boardSquareWidth -= 0.01f;
		}
	}
	//Decreases Stats Panel Size Properties until the given values while panel is opened
	if (_statsOpen == false) {
		if (_statsPanelPosX <= 2.2f) {
			_statsPanelPosX += 0.08f;
		}
		if (_boardSquareWidth <= 0.38f) {
			_boardSquareWidth += 0.01f;
		}
	}
	//Open Stats Panel
	if (_inputManager.isKeyPressed(SDLK_o)) {
		openStats();
	}
	//Close Stats Panel
	if (_inputManager.isKeyPressed(SDLK_p)) {
		closeStats();
	}
	//AI turn override
	if (_inputManager.isKeyPressed(SDLK_s) && _playerTurn == false) {
		switchTurn();
	}

}

//Returns the chess square the mouse is currently over
int MainChess::getMouseQuad() {
	int quadWidth = (_boardWidth) / 8;
	int quadHeight = (_boardHeight ) / 8;

	int mx = (int)(_inputManager.getMouseCords().x);
	int my = (int)(_inputManager.getMouseCords().y);

	int quadrant = (mx / quadWidth) + 8 * (my / quadHeight);
	return quadrant;
}

//places a given chess piece at the quadrant on the board
void MainChess::placePiece(int board[8][8], int quadrant, int pieceID) {
	board[quadrant / 8][quadrant % 8] = pieceID;
}

//places a given chess piece at the quadrant on the board
void MainChess::placePieceXY(int board[8][8], int x, int y, int pieceID) {
	board[x][y] = pieceID;
}

//Removes a chess peice from the chess board by assigning a value of 0 (an empty spot) to the board array
void MainChess::removePiece(int board[8][8], int quadrant) {
	board[quadrant / 8][quadrant % 8] = 0;
}

//Returns true if the square is occupied by a chess piece, false otherwise
bool MainChess::squareOccupied(int board[8][8], int quadrant) {
	if (board[quadrant / 8][quadrant % 8] == 0)
		return false;
	
	return true;
}

//Returns the chess piece value (pawn = 1,rook = 4,king = 10 etc) at the given quadrant
int MainChess::getPieceAtQuad(int board[8][8], int quadrant) {
	return board[quadrant / 8][quadrant % 8];
}

//Returns the givens quadrants Coloumn index
int MainChess::getQuadX(int quadrant) {
	return quadrant / 8;
}

//Returns the givens quadrants Row index
int MainChess::getQuadY(int quadrant) {
	return quadrant % 8;
}

//Returns the quardanet from the given indexes in a 2D array
int MainChess::getQuadFromXY(int x, int y) {
	int quad = (x * 8) + y;
	return quad;
}

//Selects (using a quadrant which treats the 2D board array as a 1D array) and holds a chess pieces type and location for future movement
void MainChess::selectPieceToMove(int board[8][8], int quadrant) {
	if (selectedPiece.type == 0 && squareOccupied(board,quadrant)) {//If no piece is selected, select the given piece
		selectedPiece.type = getPieceAtQuad(board, quadrant);
		selectedPiece.quadrant = quadrant;
		selectedPiece.quadX = getQuadX(quadrant);
		selectedPiece.quadY = getQuadY(quadrant);
	}
	else {//If a piece is already selected
		selectedPiece.type = 0; //De-select the piece
		selectedPiece.quadrant = -1;
		selectedPiece.quadX = -1;
		selectedPiece.quadY = -1;
	}
}

//Selects (using x and y array cords) and holds a chess pieces type and location for future movement, 
void MainChess::selectPieceToMoveXY(int board[8][8], int x, int y) {
	if (selectedPiece.type == 0) {//If no piece is selected, select the given piece
		selectedPiece.type = getPieceAtQuad(board, getQuadFromXY(x, y));
		selectedPiece.quadrant = getQuadFromXY(x,y);
		selectedPiece.quadX = x;
		selectedPiece.quadY = y;
	}
	else {//If a piece is already selected
		selectedPiece.type = 0; //De-select the piece
		selectedPiece.quadrant = -1;
		selectedPiece.quadX = -1;
		selectedPiece.quadY = -1;
	}
}

//Calculates the Points gained from a piece attacking another piece
int MainChess::pointGain(int board[8][8], int x, int y, int wOrb)
{
	int piece = board[x][y];
	if (wOrb > 0 && piece > 0)
		return 0;
	if (wOrb < 0 && piece < 0)
		return 0;
	if (piece < 0)
	{
		piece = piece * -1;
	}
	return piece;
}

//Determines if a given chess move (from sposX,sposY to dposX,dposY) is valid within the rules of chess
int MainChess::isValidMove(int board[8][8], int sposX, int sposY, int dposX, int dposY, int wOrb)
{
	int xDiff = sposX - dposX;
	int yDiff = sposY - dposY;
	int unSignX;
	int unSignY;
	int piece = board[sposX][sposY];

	if (sposX > 7 || dposX > 7 || sposY > 7 || dposY > 7 || sposX < 0 || sposY < 0 || dposX < 0 || dposY < 0)
		return 0;

	if (piece < 0)
		piece = piece * -1;

	if (xDiff >= 0)
		unSignX = xDiff;

	if (yDiff >= 0)
		unSignY = yDiff;

	if (xDiff < 0)
		unSignX = xDiff * -1;

	if (yDiff < 0)
		unSignY = yDiff * -1;

	if (wOrb == 1)
	{
		if (board[sposX][sposY] < 0)
			return 0;
	}
	if (wOrb == -1)
	{
		if (board[sposX][sposY] > 0)
			return 0;
	}

	if (wOrb == 1 && board[dposX][dposY] > 0)
		return 0;

	if (wOrb == -1 && board[dposX][dposY] < 0)
		return 0;

	switch (piece)
	{
	case 1:
		if (wOrb == 1 && xDiff > 0)
			return 0;

		if (wOrb == -1 && xDiff < 0)
			return 0;

		if (unSignX == 1 && yDiff == 0)
			if (board[dposX][dposY] == 0)
				return 1; // Check moving up one
		if (unSignX == 2 && yDiff == 0)
		{
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
			{
				if (board[dposX][dposY] != 0)
					return 0;

				if (wOrb == 1 && sposX == 1)
					return 1;

				if (wOrb == -1 && sposX == 6)
					return 1;
			}
		}

		if (unSignX == 1 && unSignY == 1)
		{
			if (board[dposX][dposY] > 0 && wOrb == -1)
				return 1; // Check kill

			if (board[dposX][dposY] < 0 && wOrb == 1)
				return 1; // Check kill
		}
		return 0;
	case 2:
		if (unSignX == unSignY)
		{
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
			{
				if (wOrb == -1 && board[dposX][dposY] >= 0)
					return 1; // Check next step

				if (wOrb == 1 && board[dposX][dposY] <= 0)
					return 1; // Check next step
			}
		}
		return 0;
	case 3:
		if (unSignX == 2 && unSignY == 1)
			return 1; // Check up/down and left, up/down and right

		if (unSignY == 2 && unSignX == 1)
			return 1; // Check right/left and up, right/left and down

		return 0;
	case 4:
		if (xDiff == 0 && yDiff != 0)
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
				return 1; // Check right and left

		if (xDiff != 0 && yDiff == 0)
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
				return 1; // Check up and down

		return 0;
	case 10:
		// BISHOP
		if (unSignX == unSignY)
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
				return 1; // Check next step

		// ROOK
		if (xDiff == 0 && yDiff != 0)
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
				return 1; // Check right and left

		if (xDiff != 0 && yDiff == 0)
			if (pieceInMiddle(board, sposX, sposY, dposX, dposY) == 1)
				return 1; // Check up and down

		return 0;
	case 100:
		if (unSignX == unSignY && unSignX == 1)
			return 1;

		if (unSignX == 0 && unSignY == 1)
			return 1;

		if (unSignX == 1 && unSignY == 0)
			return 1;

		return 0;
	}
	return 0;
}

int MainChess::pieceInMiddle(int board[8][8], int sposX, int sposY, int dposX, int dposY)
{
	int xDiff = sposX - dposX;
	int yDiff = sposY - dposY;
	int signX = 0;
	int signY = 0;

	if (xDiff < 0)
		signX = 1;

	if (yDiff < 0)
		signY = 1;

	if ((xDiff != 0) && (yDiff == 0))
	{
		//Moves in x zone only
		while (1 == 1)
		{
			if (signX == 1)
			{
				sposX++;
			}
			else
				sposX--;

			if (sposX == dposX)
				break;

			if (board[sposX][sposY] != 0)
				return 0;
		}
	}
	if ((xDiff == 0) && (yDiff != 0))
	{
		//Moves in y zone only
		while (1 == 1)
		{
			if (signY == 1)
			{
				sposY++;
			}
			else
				sposY--;

			if (sposY == dposY)
				break;

			if (board[sposX][sposY] != 0)
				return 0;
		}
	}
	if ((xDiff != 0) && (yDiff != 0))
	{
		//Moves diagonally
		while (1 == 1)
		{
			if ((signX == 1) && (signY == 0))
			{
				sposX++;
				sposY--;
			}
			else if ((signX == 1) && (signY == 1))
			{
				sposX++;
				sposY++;
			}
			else if ((signX == 0) && (signY == 1))
			{
				sposX--;
				sposY++;
			}
			else if ((signX == 0) && (signY == 0))
			{
				sposX--;
				sposY--;
			}

			if (sposX == dposX)
				break;

			if (board[sposX][sposY] != 0)
				return 0;
		}
	}
	return 1;
}

int MainChess::isCheckPurpose(int board[8][8], int sposx, int sposy, int dposx, int dposy, int wOrb)
{
	int i;
	int j;
	int tempboard[8][8];
	int check = 0;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			tempboard[i][j] = board[i][j];
		}
	}
	tempboard[dposx][dposy] = board[sposx][sposy];
	tempboard[sposx][sposy] = 0;
	check = inCheckOccur(tempboard);

	if (check == 2)
		if (checkMate(tempboard, 2) != 1)
		{
			printf("You have been checkmated");
			system("pause");
		}

	if (check == 1)
		if (checkMate(tempboard, 1) != 1)
		{
			printf("You have been checkmated");
		}

	if (wOrb == -1 && check == 2)
		return 1;

	if (wOrb == 1 && check == 1)
		return 1;

	return 0;
}

int MainChess::inCheckOccur(int board[8][8])
{
	int i;
	int j;
	int wkingX = 0;
	int wkingY = 0;
	int bkingX = 0;
	int bkingY = 0;
	int piece = 0;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (board[i][j] == 100)
			{
				bkingX = i;
				bkingY = j;
			}
			if (board[i][j] == -100)
			{
				wkingX = i;
				wkingY = j;
			}
		}
	}

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (board[i][j] != 0 && board[i][j] != 100 && board[i][j] != -100)
			{
				if (board[i][j] < 0)
				{
					if (isValidMove(board, i, j, bkingX, bkingY, -1) == 1)
						return 1;
				}
				if (board[i][j] > 0)
					if (isValidMove(board, i, j, wkingX, wkingY, 1) == 1)
						return 2;
			}
		}
	}
	return 0;
}

int MainChess::checkMate(int board[8][8], int kingBW)
{
	int i, j, k, l, x, y;
	int piece = 0;
	int sign = 1;

	if (kingBW == 2) // Black king under check
	{
		sign = -1;
		piece = piece * -1;
	}

	printf("THE SIGN IS %d", sign);
	//system("pause");

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			piece = board[i][j];

			switch (piece)
			{
			case 1:
				//Pawn
				//Block by killing
				for (k = 0; k < 4; k++)
				{
					if (k == 0)
					{
						x = 1;
						y = 1;
					}
					if (k == 1)
					{
						x = 1;
						y = -1;
					}
					if (k == 2)
					{
						x = 1;
						y = 0;
					}
					if (k == 3)
					{
						x = 2;
						y = 0;
					}
					if (isValidMove(board, i, j, i + x, j + y, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + x, j + y, 1 * sign) == 1)
							return 1;
				}
				break;
			case 2:
				// Bishop
				// Block by blocking or killing
				for (l = 0; l < 8; l++)
				{
					if (isValidMove(board, i, j, i + l, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j - l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i + l, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j + l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j - l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j + l, 1 * sign) == 1)
							return 1;
				}
				break;
			case 3:
				// Knight moves
				if (isValidMove(board, i, j, i + 2, j + 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 2, j + 1, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i + 2, j - 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 2, j - 1, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i - 2, j + 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 2, j + 1, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i - 2, j - 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 2, j - 1, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i - 1, j - 2, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 1, j - 2, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i + 1, j - 2, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 1, j - 2, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i - 1, j + 2, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 1, j + 2, 1 * sign) == 1)
						return 1;

				if (isValidMove(board, i, j, i + 1, j + 2, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 1, j + 2, 1 * sign) == 1)
						return 1;
				break;
			case 4:
				//Rook moves
				for (l = 0; l < 8; l++)
				{
					if (isValidMove(board, i, j, i + l, j, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i, j + l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i, j - l, 1 * sign) == 1)
							return 1;
				}
				break;
			case 10:
				for (l = 0; l < 8; l++)
				{
					// Bishop
					// Block by blocking or killing
					if (isValidMove(board, i, j, i + l, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j - l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i + l, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j + l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j - l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j + l, 1 * sign) == 1)
							return 1;
					// Rook Moves
					if (isValidMove(board, i, j, i + l, j, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i + l, j, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i - l, j, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i - l, j, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i, j + l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i, j + l, 1 * sign) == 1)
							return 1;

					if (isValidMove(board, i, j, i, j - l, 1 * sign) == 1)
						if (isCheckAvoidable(board, i, j, i, j - l, 1 * sign) == 1)
							return 1;
				}
				break;
			case 100:
				if (isValidMove(board, i, j, i + 1, j, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 1, j, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i + 1, j + 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 1, j + 1, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i + 1, j - 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i + 1, j - 1, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i, j + 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i, j + 1, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i, j - 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i, j - 1, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i - 1, j, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 1, j, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i - 1, j + 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 1, j + 1, 1 * sign) == 1)
						return 1;
				if (isValidMove(board, i, j, i - 1, j - 1, 1 * sign) == 1)
					if (isCheckAvoidable(board, i, j, i - 1, j - 1, 1 * sign) == 1)
						return 1;
			}
		}
	}

	printf("\n\nCHECKMATE!!!");
	system("pause");
	exit(0);
	return 0;
}

int MainChess::isCheckAvoidable(int board[8][8], int sposX, int sposY, int dposX, int dposY, int xOrb)
{
	int tempBoard[8][8];
	int i, j;

	if (dposX > 7 || dposX > 7 || dposY < 0 || dposY < 0)
		return 0;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			tempBoard[i][j] = board[i][j];
		}
	}

	tempBoard[dposX][dposY] = tempBoard[sposX][sposY];
	tempBoard[sposX][sposY] = 0;

	if (inCheckOccur(tempBoard) == 0)
		return 1;

	return 0;
}


//Checks if a pawn has reached the other end of the board
int MainChess::isPawnAtEnd(int board[8][8])
{
	int i;
	int j;
	int sign = 1;

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (board[i][j] == (sign * -1))
				return 1;
		}
		i = i + 6;
		sign = -1;
	}
	return 0;
}

//Allows the player to select a piece when a pawn reaches the other end of the board, The AI recieves a queen (-1 sign == black)
int MainChess::pawnPromotion(int sign, int playerAI)
{
	int i;

	if (playerAI == 0)
	{
		//return 4;
		printf("What do you want to promote your pawn to? ");
		scanf_s("%d", &i);
		return (i * sign);
	}

	if (playerAI == 1)
	{
		return 5 * sign;
	}
	return 0;
}


//Returns true if the piece succesfully moved
bool MainChess::movePiece(int board[8][8], int destQuad) {

		if ((isValidMove(board, selectedPiece.quadX, selectedPiece.quadY, getQuadX(destQuad), getQuadY(destQuad), wOrb) && selectedPiece.type != 0)) {

			//if (isCheckPurpose(board, selectedPiece.quadX, selectedPiece.quadY, getQuadX(destQuad), getQuadY(destQuad), wOrb) == 0)
			//{
				//board[getQuadX(destQuad)][getQuadY(destQuad)] = board[selectedPiece.quadX][selectedPiece.quadY];
				//board[selectedPiece.quadX][selectedPiece.quadY] = 0;
				//system("cls");
				//printBoard(board);
			//}

			placePiece(board, destQuad, selectedPiece.type);//Replace the quadrant with the selected piece
			removePiece(board, selectedPiece.quadrant);//Remove the piece from the quadrant it was moved from
			selectPieceToMove(board, destQuad);//unselect piece after move
			switchTurn(); //Switch to the other player/AI's turn
			
			return true;
		}
	return false;
}

void MainChess::movePieceXY(int board[8][8], int dx, int dy) {

	if ((isValidMove(board, selectedPiece.quadX, selectedPiece.quadY, dx, dy, wOrb))) {

		//if (isCheckPurpose(board, selectedPiece.quadX, selectedPiece.quadY, getQuadX(destQuad), getQuadY(destQuad), wOrb) == 0)
		//{
			//board[getQuadX(destQuad)][getQuadY(destQuad)] = board[selectedPiece.quadX][selectedPiece.quadY];
			//board[selectedPiece.quadX][selectedPiece.quadY] = 0;
			//system("cls");
			//printBoard(board);
		//}
	
		placePieceXY(board, dx,dy, selectedPiece.type);//Replace the quadrant with the selected piece
		removePiece(board, selectedPiece.quadrant);//Remove the piece from the quadrant it was moved from
		selectPieceToMoveXY(board, dx,dy);//unselect piece after move
		switchTurn(); //Switch to the other player/AI's turn

	}
}

//Alternates between turns between the AI and the Player
void MainChess::switchTurn() {
	executionTime();//End tracking time from previous turn

	if (_playerTurn == true) {//Switch to AI turn
		_turnCounterPlayer++;
		_executionTimeTracker = false;
		executionTime(); //Begin tracking time elapsed for AI
		_playerTurn = false;
	}
	else {//switch to Player turn
		_turnCounterAI++;
		_executionTimeTracker = false;
		executionTime(); //Begin tracking time elapsed for Player
		wOrb = -1;
		_playerTurn = true;
	}

	turn++;
}

//Returns the excecution time elapsed on every other function call, first call starts tracking time, second returns the time elapsed since first function call
int MainChess::executionTime() {
	if (_executionTimeTracker == false) {
		t1 = std::chrono::high_resolution_clock::now();
		_executionTimeTracker = true;
		return -420;
	}
	else {
		t2 = std::chrono::high_resolution_clock::now();
		_executionTimeTracker = false;
		if (_playerTurn == false) {
			_elapsedAITime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			_totalElapsedAITime += _elapsedAITime;
			return _elapsedAITime;
		}
		else {
			_elapsedPlayerTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			_totalElapsedPlayerTime += _elapsedPlayerTime;
			
			return _elapsedPlayerTime;
		}
		return _elapsedAITime;
	}

}

//Returns the elapsed time in milliseconds from first call of executionTime()
int MainChess::getExecutionTime() {
	t2 = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

//Opens the side panel that displays stats and timers, sets relevent properties
void MainChess::openStats() {
	if (_statsOpen == false) {
		_statsWidth = 400;
		_statsHeight = 0;
		_boardWidth = _windowWidth - _statsWidth;
		_boardHeight = _windowHeight - _statsHeight;
		_statsOpen = true;
	}

}
//Closes the side panel that displays stats and timers, sets relevent properties
void MainChess::closeStats() {
	if (_statsOpen == true) {
		_statsWidth = 0;
		_statsHeight = 0;
		_boardWidth = _windowWidth;
		_boardHeight = _windowHeight;
		_statsOpen = false;
	}

}

//Holds all drawing functions for the Chess GUI
void MainChess::drawChess()
{
	//Drawing to chess window (_window)************************************************* _window START
	glClearDepth(1.0);
	//Clear both the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_colorProgram.use();

	glActiveTexture(GL_TEXTURE0);

	GLint textureLocation = _colorProgram.getUniformLocation("sampler");
	glUniform1i(textureLocation, 0);//bind this texture to texture0 (GL_TEXTURE0)

	//Set the camera matrix
	GLuint pLocation = _colorProgram.getUniformLocation("P");
	glm::mat4 cameraMatrix = _camera2D.getCameraMatrix();
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));//("ID","number of matrixes", "transposition or not", pointer to matrix data)

	_spriteBatch.begin(); //drawing happens between here:------------------
	
	drawBoard();
	drawPieces();
	//char buffer[256];
	//sprintf_s(buffer, "Average Execution Time: ");
	//_spriteFont->draw(_spriteBatch, buffer, glm::vec2(-1.0f, 0.0f), glm::vec2(1.0f, 1.0f), 0.0f, ChessEngine::ColorRGBA8(0, 0, 255, 255));
	

	_spriteBatch.end(); //--------------------------

	_spriteBatch.renderBatch();

	_spriteBatchStats.begin();
	drawStats();
	_spriteBatchStats.end();
	_spriteBatchStats.renderBatch();

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	_colorProgram.unuse();

	_window.swapBuffer();

	//********************************************************************* _window END

}

//Prints the boards state to console
void MainChess::printBoard(int board[][8])
{
	int i, j;

	printf("--------------------------------\n");

	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			printf("|");
			if (board[i][j] != 0)
			{
				if (board[i][j] == 2)
					printf(" b ");
				if (board[i][j] == -2)
					printf(" B ");
				if (board[i][j] == 3)
					printf(" h ");
				if (board[i][j] == -3)
					printf(" H ");
				if (board[i][j] == 4)
					printf(" r ");
				if (board[i][j] == -4)
					printf(" R ");
				if (board[i][j] == 5)
					printf(" q ");
				if (board[i][j] == -5)
					printf(" Q ");
				if (board[i][j] == 10)
					printf(" k ");
				if (board[i][j] == -10)
					printf(" K ");
				if (board[i][j] == 1)
					printf(" p ");
				if (board[i][j] == -1)
					printf(" P ");
			}
			else
				printf("   ");
		}
		printf("|\n--------------------------------\n");
	}
}

//Draws the chess pieces to their board positions based on the board array
void MainChess::drawPieces() {


	ChessEngine::GLTexture pawnB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_pawn_1x_ns.png");
	ChessEngine::GLTexture pawnW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_pawn_1x_ns.png");
	ChessEngine::GLTexture bishB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_bishop_1x_ns.png");
	ChessEngine::GLTexture bishW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_bishop_1x_ns.png");
	ChessEngine::GLTexture knightB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_knight_1x_ns.png");
	ChessEngine::GLTexture knightW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_knight_1x_ns.png");
	ChessEngine::GLTexture rookB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_rook_1x_ns.png");
	ChessEngine::GLTexture rookW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_rook_1x_ns.png");
	ChessEngine::GLTexture queenB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_queen_1x_ns.png");
	ChessEngine::GLTexture queenW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_queen_1x_ns.png");
	ChessEngine::GLTexture kingB = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/w_king_1x_ns.png");
	ChessEngine::GLTexture kingW = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/b_king_1x_ns.png");
	ChessEngine::ColorRGBA8 color;
	glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 255;

	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			switch (board[row][col])
			{
			case 1:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, pawnW.id, 0.0f, color);
				break;
			case -1:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, pawnB.id, 0.0f, color);
				break;

			case 2:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, bishW.id, 0.0f, color);
				break;

			case -2:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, bishB.id, 0.0f, color);
				break;

			case 3:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, knightW.id, 0.0f, color);
				break;

			case -3:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, knightB.id, 0.0f, color);
				break;

			case 4:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, rookW.id, 0.0f, color);
				break;

			case -4:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, rookB.id, 0.0f, color);
				break;

			case 10:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, queenW.id, 0.0f, color);
				break;

			case -10:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, queenB.id, 0.0f, color);
				break;

			case 100:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, kingW.id, 0.0f, color);
				break;

			case -100:
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, kingB.id, 0.0f, color);
				break;
			}


		}

	}

}

//Draws the chess board, also handles drawing the valid squares a selected chess peice may move to.
void MainChess::drawBoard() {
	
	ChessEngine::GLTexture darkSquare = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/square brown dark_1x_ns.png");
	ChessEngine::GLTexture lightSquare = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/square brown light_1x_ns.png");
	ChessEngine::GLTexture validSquare = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/square gray light _1x_ns.png");
	ChessEngine::GLTexture panelBackground = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/square gray dark _1x_ns.png");
	ChessEngine::ColorRGBA8 colorSelect;
	ChessEngine::ColorRGBA8 color;
	ChessEngine::ColorRGBA8 colorBackground;
	colorBackground.r = 255;
	colorBackground.b = 255;
	colorBackground.g = 255;
	colorBackground.a = 255;
	glm::vec4 uv(0.0f, 0.0f, 0.5f, 1.0f);
	colorSelect.r = 0;
	colorSelect.g = 0;
	colorSelect.b = 255;
	colorSelect.a = 255;
	color.r = 255;
	color.g = 255;
	color.b = 255;
	color.a = 255;
	GLuint imageID = lightSquare.id;

	//Generate chess board background
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			
			if (selectedPiece.quadX == row && selectedPiece.quadY == col) {
				_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, imageID, 0.0f, colorSelect);
			}
			else {
				
				if ((isValidMove(board, selectedPiece.quadX, selectedPiece.quadY, row, col, wOrb) && selectedPiece.type != 0)) {
					//printf("valid\n");
					//printf("xX: %d , yY: %d\n", selectedPiece.quadX, selectedPiece.quadY);

					_spriteBatch.draw(glm::vec4((((float)col * _boardSquareWidth) - 1.0f), ((-(float)row * 0.25f) + 0.75f), _boardSquareWidth, 0.25f), uv, validSquare.id, 0.0f, color);
				}
				else {
					_spriteBatch.draw(glm::vec4(((float)col * _boardSquareWidth) - 1.0f, (-(float)row * 0.25f) + 0.75f, _boardSquareWidth, 0.25f), uv, imageID, 0.0f, color);
				}
			}
			//Alternate image
			if (imageID == darkSquare.id)
				imageID = lightSquare.id;
			else
				imageID = darkSquare.id;

		}
		

		if (imageID == darkSquare.id)
			imageID = lightSquare.id;
		else
			imageID = darkSquare.id;

	}

	glm::vec4 uvPanel(0.0f, 0.0f, 0.5f, 1.0f);
	//Stats Panel background
	_spriteBatch.draw(glm::vec4(_statsPanelPosX, _statsPanelPosY, _statsPanelWidth, _statsPanelHeight), uvPanel, panelBackground.id, 0.0f, colorBackground);

}

//Draws timers and statistical information
void MainChess::drawStats() {
		char boardScore[256];
		char avrgAITimeBuffer[256];
		char remainingAITimeBuffer[256];
		char remainingPlayerTimeBuffer[256];

		ChessEngine::GLTexture executionTimeBar = ChessEngine::ResourceManager::getTexture("Textures/JohnPablok Cburnett Chess set/PNGs/No shadow/1x/square gray light _1x_ns test.png");


		ChessEngine::ColorRGBA8 colorExecutionBar;
		colorExecutionBar.r = 255;
		colorExecutionBar.b = 255;
		colorExecutionBar.g = 255;
		colorExecutionBar.a = 255;

		
		glm::vec4 uv(1.0f, 1.0f, 1.0f, 2.0f);


		//Statistical Information
		if (_statsOpen == true) { //Only draw statistical info when panel is open
			
			//Drawing Turn Timers
			if (_turnCounterPlayer > 0) {
				int elapsedAITime = _totalElapsedAITime;
				int elapsedPlayerTime = _totalElapsedPlayerTime;

				if (_playerTurn == true) {
					elapsedPlayerTime += getExecutionTime();
				}
				else {
					elapsedAITime += getExecutionTime();
				}

				sprintf_s(remainingAITimeBuffer, "AI Time: %d:%d", ((66 * 60000) - elapsedAITime) % 6000000 / 100000, ((66 * 60000) - elapsedAITime) % 60000 / 1000);
				sprintf_s(remainingPlayerTimeBuffer, "Player Time: %d:%d", ((66 * 60000) - elapsedPlayerTime) % 6000000 / 100000,((66 * 60000) - elapsedPlayerTime) % 60000 / 1000);
				if (_turnCounterAI > 0) {
					sprintf_s(avrgAITimeBuffer, "Avrg AI Time(ms): %d", elapsedAITime / _turnCounterAI);
					sprintf_s(boardScore, "Point Score of AI Board: %d", currentPoint);
				}
			}

			_spriteBatchStats.draw(glm::vec4(_statsExecuteBarPosX, _statsExecuteBarPosY, _statsExecuteBarWidth, _statsExecuteBarHeight), uv, executionTimeBar.id, 0.0f, colorExecutionBar);

			
			
			//Draw Text Stat Information (bottom to top)
			_spriteFont->draw(_spriteBatchStats, avrgAITimeBuffer, glm::vec2(1.05f, -1.0f), glm::vec2(0.0018f, 0.0018f), 0.0f, ChessEngine::ColorRGBA8(255, 0, 0, 255));
			_spriteFont->draw(_spriteBatchStats, remainingAITimeBuffer, glm::vec2(1.05f, -0.90f), glm::vec2(0.0018f, 0.0018f), 0.0f, ChessEngine::ColorRGBA8(255, 0, 0, 255));
			_spriteFont->draw(_spriteBatchStats, remainingPlayerTimeBuffer, glm::vec2(1.05f, -0.80f), glm::vec2(0.0018f, 0.0018f), 0.0f, ChessEngine::ColorRGBA8(255, 0, 0, 255));
			_spriteFont->draw(_spriteBatchStats, boardScore, glm::vec2(1.05f, -0.70f), glm::vec2(0.0018f, 0.0018f), 0.0f, ChessEngine::ColorRGBA8(255, 0, 0, 255));

		}
}


//Overarching chess game loop
void MainChess::chessLoop()
{
	while (_chessState != ChessState::EXIT)
	{

		processInput();
		aiLoop(); //AI loop
		_camera2D.update();
		drawChess(); //Draws graphics to window

	}
}



//The AI loop where AI is aloud to calculate moves and perform an action
void MainChess::aiLoop()
{
	if ( _playerTurn == false)
	{

	//AI turn
	for (k = 0; k < 1; k++)
	{
		index = 0;
		
		if (wOrb == -1)
		{
			wOrb = 1;
		}
		else 
		{
			wOrb = -1;
		}

		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
			{
				if (wOrb == 1 && board[i][j] > 0)   // Black
				{
					arr[index][0] = i;
					arr[index][1] = j;
					index++;
				}
				if (wOrb == -1 && board[i][j] < 0)  // White
				{
					arr[index][0] = i;
					arr[index][1] = j;
					index++;
				}
			}
		}

		for (i = 0; i < index; i++)
		{
			x = arr[i][0];
			y = arr[i][1];
			piece = board[x][y];
			pointBest[0] = -1;

			if (wOrb == -1)
				piece = piece * -1;

			switch (piece)
			{
			case 1:
				// Pawn
				if (isValidMove(board, x, y, x + 1, y + 1, wOrb) == 1)
				{
					currentPoint = pointGain(board, x + 1, y + 1, wOrb);
					if (currentPoint > pointBest[0])
					{
						pointBest[0] = currentPoint;
						pointBest[1] = x;
						pointBest[2] = y;
						pointBest[3] = x + 1;
						pointBest[4] = y + 1;
					}
				}
				else
					if (isValidMove(board, x, y, x + 1, y - 1, wOrb) == 1)
					{
						currentPoint = pointGain(board, x + 1, y - 1, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x + 1;
							pointBest[4] = y - 1;
						}
					}
				break;
			case 2:
				// Bishop
				for (j = 0; j < 8; j++)
				{
					if (isValidMove(board, x, y, x + j, y + j, wOrb) == 1)
					{
						currentPoint = pointGain(board, x + j, y + j, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x + j;
							pointBest[4] = y + j;
						}
					}
					else
						if (isValidMove(board, x, y, x + j, y - j, wOrb) == 1)
						{
							currentPoint = pointGain(board, x + j, y - j, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x + j;
								pointBest[4] = y - j;
							}
						}
						else
							if (isValidMove(board, x, y, x - j, y + j, wOrb) == 1)
							{
								currentPoint = pointGain(board, x - j, y + j, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x - j;
									pointBest[4] = y + j;
								}
							}
							else
								if (isValidMove(board, x, y, x - j, y - j, wOrb) == 1)
								{
									currentPoint = pointGain(board, x - j, y - j, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x - j;
										pointBest[4] = y - j;
									}
								}
				}
				break;
			case 3:
				// Knight
				if (isValidMove(board, x, y, x + 2, y + 1, wOrb) == 1)
				{
					currentPoint = pointGain(board, x + 2, y + 1, wOrb);
					if (currentPoint > pointBest[0])
					{
						pointBest[0] = currentPoint;
						pointBest[1] = x;
						pointBest[2] = y;
						pointBest[3] = x + 2;
						pointBest[4] = y + 1;
					}
				}
				else
					if (isValidMove(board, x, y, x + 2, y - 1, wOrb) == 1)
					{
						currentPoint = pointGain(board, x + 2, y - 1, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x + 2;
							pointBest[4] = y - 1;
						}
					}
					else
						if (isValidMove(board, x, y, x - 2, y + 1, wOrb) == 1)
						{
							currentPoint = pointGain(board, x - 2, y + 1, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x - 2;
								pointBest[4] = y + 1;
							}
						}
						else
							if (isValidMove(board, x, y, x - 2, y - 1, wOrb) == 1)
							{
								currentPoint = pointGain(board, x - 2, y - 1, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x - 2;
									pointBest[4] = y - 1;
								}
							}
							else
								if (isValidMove(board, x, y, x + 1, y + 2, wOrb) == 1)
								{
									currentPoint = pointGain(board, x + 1, y + 2, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x + 1;
										pointBest[4] = y + 2;
									}
								}
								else
									if (isValidMove(board, x, y, x + 1, y - 2, wOrb) == 1)
									{
										currentPoint = pointGain(board, x + 1, y - 2, wOrb);
										if (currentPoint > pointBest[0])
										{
											pointBest[0] = currentPoint;
											pointBest[1] = x;
											pointBest[2] = y;
											pointBest[3] = x + 1;
											pointBest[4] = y - 2;
										}
									}
									else
										if (isValidMove(board, x, y, x - 1, y + 2, wOrb) == 1)
										{
											currentPoint = pointGain(board, x - 1, y + 2, wOrb);
											if (currentPoint > pointBest[0])
											{
												pointBest[0] = currentPoint;
												pointBest[1] = x;
												pointBest[2] = y;
												pointBest[3] = x - 1;
												pointBest[4] = y + 2;
											}
										}
										else
											if (isValidMove(board, x, y, x - 1, y - 2, wOrb) == 1)
											{
												currentPoint = pointGain(board, x + 1, y - 2, wOrb);
												if (currentPoint > pointBest[0])
												{
													pointBest[0] = currentPoint;
													pointBest[1] = x;
													pointBest[2] = y;
													pointBest[3] = x - 1;
													pointBest[4] = y - 2;
												}
											}
				break;
			case 4:
				// Rook
				for (j = 0; j < 8; j++)
				{
					if (isValidMove(board, x, y, x - j, y, wOrb) == 1)
					{
						currentPoint = pointGain(board, x - j, y, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x - j;
							pointBest[4] = y;
						}
					}
					else
						if (isValidMove(board, x, y, x + j, y, wOrb) == 1)
						{
							currentPoint = pointGain(board, x + j, y, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x + j;
								pointBest[4] = y;
							}
						}
						else
							if (isValidMove(board, x, y, x, y - j, wOrb) == 1)
							{
								currentPoint = pointGain(board, x, y - j, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x;
									pointBest[4] = y - j;
								}
							}
							else
								if (isValidMove(board, x, y, x, y + j, wOrb) == 1)
								{
									currentPoint = pointGain(board, x, y + j, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x;
										pointBest[4] = y + j;
									}
								}
				}
				break;
			case 10:
				// Queen
				// Bishop
				for (j = 0; j < 8; j++)
				{
					if (isValidMove(board, x, y, x + j, y + j, wOrb) == 1)
					{
						currentPoint = pointGain(board, x + j, y + j, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x + j;
							pointBest[4] = y + j;
						}
					}
					else
						if (isValidMove(board, x, y, x + j, y - j, wOrb) == 1)
						{
							currentPoint = pointGain(board, x + j, y - j, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x + j;
								pointBest[4] = y - j;
							}
						}
						else
							if (isValidMove(board, x, y, x - j, y + j, wOrb) == 1)
							{
								currentPoint = pointGain(board, x - j, y + j, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x - j;
									pointBest[4] = y + j;
								}
							}
							else
								if (isValidMove(board, x, y, x - j, y - j, wOrb) == 1)
								{
									currentPoint = pointGain(board, x - j, y - j, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x - j;
										pointBest[4] = y - j;
									}
								}
				}
				// Rook
				for (j = 0; j < 8; j++)
				{
					if (isValidMove(board, x, y, x - j, y, wOrb) == 1)
					{
						currentPoint = pointGain(board, x - j, y, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x - j;
							pointBest[4] = y;
						}
					}
					else
						if (isValidMove(board, x, y, x + j, y, wOrb) == 1)
						{
							currentPoint = pointGain(board, x + j, y, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x + j;
								pointBest[4] = y;
							}
						}
						else
							if (isValidMove(board, x, y, x, y - j, wOrb) == 1)
							{
								currentPoint = pointGain(board, x, y - j, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x;
									pointBest[4] = y - j;
								}
							}
							else
								if (isValidMove(board, x, y, x, y + j, wOrb) == 1)
								{
									currentPoint = pointGain(board, x, y + j, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x;
										pointBest[4] = y + j;
									}
								}
				}
				break;
			case 100:
				// King
				if (isValidMove(board, x, y, x + 1, y, wOrb) == 1)
				{
					currentPoint = pointGain(board, x + 1, y, wOrb);
					if (currentPoint > pointBest[0])
					{
						pointBest[0] = currentPoint;
						pointBest[1] = x;
						pointBest[2] = y;
						pointBest[3] = x + 1;
						pointBest[4] = y;
					}
				}
				else
					if (isValidMove(board, x, y, x + 1, y + 1, wOrb) == 1)
					{
						currentPoint = pointGain(board, x + 1, y + 1, wOrb);
						if (currentPoint > pointBest[0])
						{
							pointBest[0] = currentPoint;
							pointBest[1] = x;
							pointBest[2] = y;
							pointBest[3] = x + 1;
							pointBest[4] = y + 1;
						}
					}
					else
						if (isValidMove(board, x, y, x + 1, y - 1, wOrb) == 1)
						{
							currentPoint = pointGain(board, x + 1, y - 1, wOrb);
							if (currentPoint > pointBest[0])
							{
								pointBest[0] = currentPoint;
								pointBest[1] = x;
								pointBest[2] = y;
								pointBest[3] = x + 1;
								pointBest[4] = y - 1;
							}
						}
						else
							if (isValidMove(board, x, y, x, y + 1, wOrb) == 1)
							{
								currentPoint = pointGain(board, x, y + 1, wOrb);
								if (currentPoint > pointBest[0])
								{
									pointBest[0] = currentPoint;
									pointBest[1] = x;
									pointBest[2] = y;
									pointBest[3] = x;
									pointBest[4] = y + 1;
								}
							}
							else
								if (isValidMove(board, x, y, x - 1, y + 1, wOrb) == 1)
								{
									currentPoint = pointGain(board, x - 1, y + 1, wOrb);
									if (currentPoint > pointBest[0])
									{
										pointBest[0] = currentPoint;
										pointBest[1] = x;
										pointBest[2] = y;
										pointBest[3] = x - 1;
										pointBest[4] = y + 1;
									}
								}
								else
									if (isValidMove(board, x, y, x - 1, y, wOrb) == 1)
									{
										currentPoint = pointGain(board, x - 1, y, wOrb);
										if (currentPoint > pointBest[0])
										{
											pointBest[0] = currentPoint;
											pointBest[1] = x;
											pointBest[2] = y;
											pointBest[3] = x - 1;
											pointBest[4] = y;
										}
									}
									else
										if (isValidMove(board, x, y, x - 1, y - 1, wOrb) == 1)
										{
											currentPoint = pointGain(board, x - 1, y - 1, wOrb);
											if (currentPoint > pointBest[0])
											{
												pointBest[0] = currentPoint;
												pointBest[1] = x;
												pointBest[2] = y;
												pointBest[3] = x - 1;
												pointBest[4] = y - 1;
											}
										}
										else
											if (isValidMove(board, x, y, x, y - 1, wOrb) == 1)
											{
												currentPoint = pointGain(board, x, y - 1, wOrb);
												if (currentPoint > pointBest[0])
												{
													pointBest[0] = currentPoint;
													pointBest[1] = x;
													pointBest[2] = y;
													pointBest[3] = x;
													pointBest[4] = y - 1;
												}
											}
				break;
			}
		}	
		//Move the piece
		sposX = pointBest[1];
		sposY = pointBest[2];
		dposX = pointBest[3];
		dposY = pointBest[4];

		selectPieceToMoveXY(board, sposX, sposY);
		movePieceXY(board,dposX,dposY);

	}

  }
}