#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "raylib.h"

//Si no encuentra Raylib: Proyecto --> Propiedades --> General --> Poner path de carpeta de includes
//Si no encuentra Raylib: Proyecto --> Propiedades --> Vinculadores --> Poner path de carpeta de .lib

using namespace std;

//Game Vars 
const float m_version = 0.5f;
float m_gameVersion = 0.0f;
string m_title = "none :'(";
int m_texturesNum = 0;
map<string, string> m_textures;
map<string, Texture2D> m_textures2D;
Sound m_bgMusic;
bool GameOver;
string m_winner = "";

//Map
int m_bgWidth = 0, m_bgHeight = 0;
int m_fgWidth = 0, m_fgHeight = 0;
int m_objWidth = 0, m_objHeight = 0;
string** m_background;
string** m_foreground;
string** m_objects;

float m_marginX, m_marginZ;
float m_offsetX, m_offsetZ;

Vector3 m_drawPosition = { 0.0f, 0.0f, 0.0f };
float cubeSize[3] = { 1.0f, 1.0f, 1.0f }; //Width Height Length

//Bombs
struct Bombs
{
	int posX = 0, posZ = 0;
	float time = 150.0f;
};
//Power Up
struct PowerUp
{
	float time = 0.0f;
	const float maxTime = 300.0f;
};
//Player
struct Player
{
	bool alive = true;
	string playerNum = "1";
	int posX = 0, posZ = 0;
	Color color = WHITE;

	int num_bombs = 0, max_bombs = 2;
	vector<Bombs> bombs;
	bool m_hasBombDown = false;
	PowerUp pwrUp;
};
vector<Player> m_players;

//Utitities
bool isAlive(int player) {
	return m_players[player].alive;
}
bool isPassableObject(int x, int y) {
	return (m_objects[x][y] == "0" || m_objects[x][y] == "U") && m_foreground[x][y] != "T";
}
bool canBreakBlock(int x, int y) {
	return m_objects[x][y] != "X" && m_objects[x][y] != "B";
}
bool isPlayer(string num) {
	return num == "1" || num == "2" || num == "3" || num == "4";
}
bool isPwrUp(string pos) {
	return pos == "U";
}
int getPlayer(int x, int y) {
	return stoi(m_objects[x][y]) - 1;
}
bool canPlaceBomb(Player player) {
	return player.num_bombs < player.max_bombs;
}
void DeadPlayerText(Player player) {
	cout << "Player " << player.playerNum << " died." << endl;
}

//Import Functions
void ImportFile() {
	ifstream file("level.sus", ios::in);
	if (!file.is_open()) {
		cout << "\n\nERROR <1>: \"level.sus\" not found!\n" << endl;
		system("pause");
		exit(1);
	}
	else { cout << "file opened!" << endl; }
	string line;

	//Extension Control
	getline(file, line, ';');
	if (line != "SUS") {
		cout << "\n\nERROR <2>: Wrong extension detected!\n" << endl;
		system("pause");
		exit(2);
	}
	cout << "\nExtension: " << line << endl;
	//Version Control
	getline(file, line, ';');
	m_gameVersion = stof(line);
	if (stof(line) < m_version) {
		cout << "\n\nWARNING <3>: Old version detected!, upgrade to " << m_version <<" to enjoy the newest features :)" << "\n" << endl;
		system("pause");
	}
	cout << "Version: " << line << endl;
	getline(file, line); // End Line

	//Title Control
	getline(file, line, ';');
	if (line != "TITLE") {
		cout << "\n\nERROR <4>: Title missing!\n" << endl;
		system("pause");
		exit(4);
	}
	//Set Title
	getline(file, line, ';');
	m_title = line;
	cout << "Game Title: " << m_title << endl;
	getline(file, line); // End Line

	if (m_gameVersion >= 0.5f) {
		//Music Control
		getline(file, line, ';');
		if (line != "MUSIC") {
			cout << "\n\nERROR <5>: Music missing!\n" << endl;
			system("pause");
			exit(5);
		}
		//Set Music
		getline(file, line, ';');
		const char* MUSIC_PATH = line.c_str();
		m_bgMusic = LoadSound(MUSIC_PATH);
		cout << "BGMusic path: " << MUSIC_PATH << endl;
		getline(file, line); // End Line
	}

	//Textures Control
	getline(file, line, ';');
	if (line != "TEXTURES") {
		cout << "\n\nERROR <6>: Textures missing!\n" << endl;
		system("pause");
		exit(6);
	}
	//Set Textures number
	getline(file, line, ';');
	m_texturesNum = stoi(line);
	cout << "\nTextures to save: " << m_texturesNum << endl;
	getline(file, line); // End Line

	//Save textures
	for (int i = 0; i < m_texturesNum; i++) {
		getline(file, line, ';');
		string texId = line;
		getline(file, line, ';');
		string texPath = line;
		m_textures[texId] = texPath;

		cout << "Texture " << texId << " -> " << texPath << endl;
		getline(file, line); // End Line
	}

	//Background Control
	getline(file, line, ';');
	if (line != "BACKGROUND") {
		cout << "\n\nERROR <7>: Background missing!\n" << endl;
		system("pause");
		exit(7);
	}
	//Set Backrgound size
	getline(file, line, ';');
	m_bgWidth = stoi(line);
	getline(file, line, ';');
	m_bgHeight = stoi(line);
	cout << "\nBackground size: " << m_bgWidth << "x" << m_bgHeight << endl;
	getline(file, line); // End Line
	//Set bg level
	m_background = new string * [m_bgHeight]; //El doble puntero crea strings de la altura del lvl
	for (int i = 0; i < m_bgHeight; i++) {
		m_background[i] = new string[m_bgWidth]; //El doble puntero crea en cada altura su longitud
	}
	for (int i = 0; i < m_bgHeight; i++) {
		for (int j = 0; j < m_bgWidth; j++) {
			getline(file, line, ';');
			m_background[i][j] = line;
			cout << m_background[i][j] << " ";
		}
		if (i != m_bgHeight - 1) { cout << endl; }
		getline(file, line); // End Line
	}

	//Foreground Control
	getline(file, line, ';');
	if (line != "FOREGROUND") {
		cout << "\n\nERROR <8>: Foreground missing!\n" << endl;
		system("pause");
		exit(8);
	}
	//Set Forergound size
	getline(file, line, ';');
	m_fgWidth = stoi(line);
	getline(file, line, ';');
	m_fgHeight = stoi(line);
	cout << "\nForeground size: " << m_fgWidth << "x" << m_fgHeight << endl;
	getline(file, line); // End Line
	//Set fg level
	m_foreground = new string * [m_fgHeight]; //El doble puntero crea strings de la altura del lvl
	for (int i = 0; i < m_fgHeight; i++) {
		m_foreground[i] = new string[m_fgWidth]; //El doble puntero crea en cada altura su longitud
	}
	for (int i = 0; i < m_fgHeight; i++) {
		for (int j = 0; j < m_fgWidth; j++) {
			getline(file, line, ';');
			m_foreground[i][j] = line;
			cout << m_foreground[i][j] << " ";
		}
		if (i != m_fgHeight - 1) { cout << endl; }
		getline(file, line); // End Line
	}

	//Objects Control
	getline(file, line, ';');
	if (line != "OBJECTS") {
		cout << "\n\nERROR <9>: Objects missing!\n" << endl;
		system("pause");
		exit(9);
	}
	//Set Objects size
	getline(file, line, ';');
	m_objWidth = stoi(line);
	getline(file, line, ';');
	m_objHeight = stoi(line);
	cout << "\nObjects size: " << m_objWidth << "x" << m_objHeight << endl;
	getline(file, line); // End Line

	//Set Objects level
	m_objects = new string * [m_objHeight]; //El doble puntero crea strings de la altura del lvl
	for (int i = 0; i < m_objHeight; i++) {
		m_objects[i] = new string[m_objWidth]; //El doble puntero crea en cada altura su longitud
	}
	for (int i = 0; i < m_objHeight; i++) {
		for (int j = 0; j < m_objWidth; j++) {
			getline(file, line, ';');
			m_objects[i][j] = line;
			cout << m_objects[i][j] << " ";
		}
		cout << endl;
		getline(file, line); // End Line
	}

	file.close();
}
void LoadTextures() {
	cout << "\nTextures Loaded:" << endl;
	map<string, string>::iterator it;
	for (it = m_textures.begin(); it != m_textures.end();it++) {
		cout << "\n" << it->first << ": " << it->second << endl;
		m_textures2D[it->first]= LoadTextureFromImage(LoadImage(it->second.c_str()));
	}
}
void Initplayers() {
	for (int i = 0; i < m_objHeight; i++) {
		for (int j = 0; j < m_objWidth; j++) {
			if (isPlayer(m_objects[i][j])) {
				Player player;
				player.playerNum = m_objects[i][j];
				player.posX = i;
				player.posZ = j;
				if (player.playerNum == "1") { player.color = WHITE; }
				if (player.playerNum == "2") { player.color = GREEN; }
				if (player.playerNum == "3") { player.color = ORANGE; }
				if (player.playerNum == "4") { player.color = SKYBLUE; }

				m_players.push_back(player);
			}
		}
	}
}

//Draw Functions
void DrawLevel() {
	//Draw Background
	for (int i = 0; i < m_bgHeight; i++) {
		for (int j = 0; j < m_bgWidth; j++) {
			m_drawPosition = { (float)j + (-m_offsetX + m_marginX) ,0.0f, (float)i + (-m_offsetZ + m_marginZ) };

			if (m_background[i][j] != "X") {
				DrawCubeTexture(m_textures2D[m_background[i][j]], m_drawPosition, cubeSize[0], cubeSize[1], cubeSize[2], WHITE);
			}
		}
	}
	//Draw Foreground
	for (int i = 0; i < m_fgHeight; i++) {
		for (int j = 0; j < m_fgWidth; j++) {
			m_drawPosition = { (float)j + (-m_offsetX + m_marginX) ,1.0f, (float)i + (-m_offsetZ + m_marginZ) };

			if (m_foreground[i][j] != "0") {
				DrawCubeTexture(m_textures2D[m_foreground[i][j]], m_drawPosition, cubeSize[0], cubeSize[1], cubeSize[2], WHITE);
			}
		}
	}
	//Draw Grid
	if (m_bgHeight > m_bgWidth) { DrawGrid(m_bgHeight + 1, 1.0f); }
	else { DrawGrid(m_bgWidth + 1, 1.0f); }
}
void DrawObjects() {
	for (int i = 0; i < m_objHeight; i++) {
		for (int j = 0; j < m_objWidth; j++) {
			m_drawPosition = { (float)j + (-m_offsetX + m_marginX) ,1.0f, (float)i + (-m_offsetZ + m_marginZ) };

			if (isPlayer(m_objects[i][j])) {
				m_players[stoi(m_objects[i][j]) - 1].posX = i;
				m_players[stoi(m_objects[i][j]) - 1].posZ = j;

				if (m_players[stoi(m_objects[i][j]) - 1].alive) {
					DrawSphere(m_drawPosition, cubeSize[0] / 2, m_players[stoi(m_objects[i][j]) - 1].color);
				}
			}

			if (m_objects[i][j] == "B") {
				DrawSphere(m_drawPosition, cubeSize[0] / 3.0f, BLACK);
			}
			if (m_objects[i][j] == "U") {
				DrawSphere(m_drawPosition, cubeSize[0] / 2.0f, PINK);
			}
		}
	}
}

//Gameflow
void PlayerMovement(KeyboardKey up,KeyboardKey down, KeyboardKey right, KeyboardKey left, int player) {
	//OPPOSITE AXIS
	//HEIGHT IS X AXIS and WIDTH IS Z AXIS

	//RIGHT
	if (IsKeyPressed(right) && isPassableObject(m_players[player].posX, m_players[player].posZ + 1)) {
		//Place Bomb
		if (!m_players[player].m_hasBombDown) { m_objects[m_players[player].posX][m_players[player].posZ] = "0"; }
		else { m_objects[m_players[player].posX][m_players[player].posZ] = "B"; }

		//Get power up
		if(isPwrUp(m_objects[m_players[player].posX][m_players[player].posZ + 1])) {
			m_players[player].pwrUp.time = m_players[player].pwrUp.maxTime;
			cout << "Player: " << m_players[player].playerNum << " got a Power Up" << endl;
		}

		//Move
		m_objects[m_players[player].posX][m_players[player].posZ + 1] = m_players[player].playerNum;
		m_players[player].m_hasBombDown = false;
	}
	//LEFT
	else if (IsKeyPressed(left) && isPassableObject(m_players[player].posX, m_players[player].posZ - 1)) {
		//Place Bomb
		if (!m_players[player].m_hasBombDown) { m_objects[m_players[player].posX][m_players[player].posZ] = "0"; }
		else { m_objects[m_players[player].posX][m_players[player].posZ] = "B"; }

		//Get power up
		if (isPwrUp(m_objects[m_players[player].posX][m_players[player].posZ - 1])) {
			m_players[player].pwrUp.time = m_players[player].pwrUp.maxTime;
			cout << "Player: " << m_players[player].playerNum << " got a Power Up" << endl;
		}

		//Move
		m_objects[m_players[player].posX][m_players[player].posZ - 1] = m_players[player].playerNum;
		m_players[player].m_hasBombDown = false;
	}
	//UP
	else if (IsKeyPressed(up) && isPassableObject(m_players[player].posX - 1, m_players[player].posZ)) {
		//Place Bomb
		if (!m_players[player].m_hasBombDown) { m_objects[m_players[player].posX][m_players[player].posZ] = "0"; }
		else { m_objects[m_players[player].posX][m_players[player].posZ] = "B"; }

		//Get power up
		if (isPwrUp(m_objects[m_players[player].posX - 1][m_players[player].posZ])) {
			m_players[player].pwrUp.time = m_players[player].pwrUp.maxTime;
			cout << "Player: " << m_players[player].playerNum << " got a Power Up" << endl;
		}

		//Move
		m_objects[m_players[player].posX - 1][m_players[player].posZ] = m_players[player].playerNum;
		m_players[player].m_hasBombDown = false;
	}
	//DOWN
	else if (IsKeyPressed(down) && isPassableObject(m_players[player].posX + 1, m_players[player].posZ)) {
		//Place Bomb
		if (!m_players[player].m_hasBombDown) { m_objects[m_players[player].posX][m_players[player].posZ] = "0"; }
		else { m_objects[m_players[player].posX][m_players[player].posZ] = "B"; }

		//Get power up
		if (isPwrUp(m_objects[m_players[player].posX + 1][m_players[player].posZ])) {
			m_players[player].pwrUp.time = m_players[player].pwrUp.maxTime;
			cout << "Player: " << m_players[player].playerNum << " got a Power Up" << endl;
		}

		//Move
		m_objects[m_players[player].posX + 1][m_players[player].posZ] = m_players[player].playerNum;
		m_players[player].m_hasBombDown = false;
	}
}
void BombPlacement(KeyboardKey attack, int player) {
	if (IsKeyPressed(attack) && canPlaceBomb(m_players[player]) && m_players[player].alive) {
		m_players[player].m_hasBombDown = true;

		Bombs bomb;
		bomb.posX = m_players[player].posX;
		bomb.posZ = m_players[player].posZ;
		m_players[player].bombs.push_back(bomb);
		m_players[player].num_bombs = m_players[player].bombs.size();
	}
}
void BombTimer() {
	for (int i = 0; i < m_players.size(); i++)
	{
		for (int j = 0; j < m_players[i].bombs.size(); j++) {
			m_players[i].bombs[j].time--;

			if (m_players[i].bombs[j].time <= 0.0f) {
				//Down
				string downPos = m_objects[m_players[i].bombs[j].posX + 1][m_players[i].bombs[j].posZ];
				if (isPlayer(downPos)) {
					DeadPlayerText(m_players[getPlayer(m_players[i].bombs[j].posX + 1, m_players[i].bombs[j].posZ)]);

					m_players[getPlayer(m_players[i].bombs[j].posX + 1, m_players[i].bombs[j].posZ)].alive = false;
					m_objects[m_players[i].bombs[j].posX + 1][m_players[i].bombs[j].posZ] = "0";
				}
				else if (isPwrUp(downPos))
				{
					m_foreground[m_players[i].bombs[j].posX + 1][m_players[i].bombs[j].posZ] = "0";
					cout << "PowerUp found!"<<endl;
				}
				else if (canBreakBlock(m_players[i].bombs[j].posX + 1, m_players[i].bombs[j].posZ)) {
					m_objects[m_players[i].bombs[j].posX + 1][m_players[i].bombs[j].posZ] = "0";
					m_foreground[m_players[i].bombs[j].posX + 1][m_players[i].bombs[j].posZ] = "0";
				}
				//Up
				string upPos = m_objects[m_players[i].bombs[j].posX - 1][m_players[i].bombs[j].posZ];
				if (isPlayer(upPos)) {
					DeadPlayerText(m_players[getPlayer(m_players[i].bombs[j].posX - 1, m_players[i].bombs[j].posZ)]);

					m_players[getPlayer(m_players[i].bombs[j].posX - 1, m_players[i].bombs[j].posZ)].alive = false;
					m_objects[m_players[i].bombs[j].posX - 1][m_players[i].bombs[j].posZ] = "0";
				}
				else if (isPwrUp(upPos))
				{
					m_foreground[m_players[i].bombs[j].posX - 1][m_players[i].bombs[j].posZ] = "0";
					cout << "PowerUp found!" << endl;
				}
				else if (canBreakBlock(m_players[i].bombs[j].posX - 1, m_players[i].bombs[j].posZ)) {
					m_objects[m_players[i].bombs[j].posX - 1][m_players[i].bombs[j].posZ] = "0";
					m_foreground[m_players[i].bombs[j].posX - 1][m_players[i].bombs[j].posZ] = "0";
				}
				//Left
				string leftPos = m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ-1];
				if (isPlayer(leftPos)) {
					DeadPlayerText(m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ - 1)]);

					m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ - 1)].alive = false;
					m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ - 1] = "0";
				}
				else if (isPwrUp(leftPos))
				{
					m_foreground[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ - 1] = "0";
					cout << "PowerUp found!" << endl;
				}
				else if (canBreakBlock(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ - 1)) {
					m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ - 1] = "0";
					m_foreground[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ - 1] = "0";
				}
				//Right
				string rightPos = m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ + 1];
				if (isPlayer(rightPos)) {
					DeadPlayerText(m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ + 1)]);

					m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ + 1)].alive = false;
					m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ + 1] = "0";
				}
				else if (isPwrUp(rightPos))
				{
					m_foreground[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ + 1] = "0";
					cout << "PowerUp found!" << endl;
				}
				else if (canBreakBlock(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ + 1)) {
					m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ + 1] = "0";
					m_foreground[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ + 1] = "0";
				}

				//Delete Bomb
				if (isPlayer(m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ])) {
					DeadPlayerText(m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ)]);
					m_players[getPlayer(m_players[i].bombs[j].posX, m_players[i].bombs[j].posZ)].alive = false;
				}
				m_objects[m_players[i].bombs[j].posX][m_players[i].bombs[j].posZ] = "0";
				m_players[i].bombs.erase(m_players[i].bombs.begin());
				m_players[i].num_bombs = m_players[i].bombs.size();
				cout << "BOOM!" << endl;
			}
		}
	}
}
void PowerUpTimer() {
	for (int i = 0; i < m_players.size(); i++)
	{
		m_players[i].pwrUp.time--;
		if (m_players[i].pwrUp.time > 0.0f) { m_players[i].max_bombs = 3; }
		else { m_players[i].max_bombs = 2; }
	}
}
void WinCondition() {
	float PlayersLeft = 0;
	m_winner = "_";
	for (int i = 0; i < m_players.size(); i++) {
		if (m_players[i].alive) { PlayersLeft++; m_winner = m_players[i].playerNum; }
	}

	if (PlayersLeft <= 1) { GameOver = true; }
}

int main(void) {
	InitAudioDevice();
	ImportFile();

	// Initialization
	const int screenWidth = 1600;
	const int screenHeight = 900;

	//Map Offsets
	m_marginX = m_bgWidth % 2 == 0 ? .5 : 0;
	m_marginZ = m_bgHeight % 2 == 0 ? .5 : 0;
	m_offsetX = m_bgWidth / 2;
	m_offsetZ = m_bgHeight / 2;

	InitWindow(screenWidth, screenHeight, m_title.c_str());
	LoadTextures();
	Initplayers();

	//Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = { 0.0f, 10.0f, 2.0f };	//Camera position
	camera.target = { 0.0f, 0.0f, 0.0f };		//Camera looking at point
	camera.up = { 0.0f, 1.0f, 0.0f };			//Camera up vector (rotation towards target)
	camera.fovy = 45.0f;						//Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;     //Camera mode type

	SetTargetFPS(60);               //Set our game to run at 60 frames-per-second

	//Main game loop
	while (!WindowShouldClose())    //Detect window close button or ESC key
	{
		//Update
		if (!GameOver) { WinCondition(); }
		if (!IsSoundPlaying(m_bgMusic) && m_version >= 0.5f) {	//BGM
			PlaySound(m_bgMusic);
		}

		BombPlacement(KEY_RIGHT_CONTROL, 0);
		BombPlacement(KEY_SPACE, 1);

		BombTimer();
		PowerUpTimer();

		if (isAlive(0)) { PlayerMovement(KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT, 0); }
		if (isAlive(1)) { PlayerMovement(KEY_W, KEY_S, KEY_D, KEY_A, 1); }

		//Draw Things
		BeginDrawing();

		if (!GameOver) { ClearBackground(RAYWHITE); }
		else { ClearBackground(DARKGRAY); }
		BeginMode3D(camera);

		if (!GameOver) {
			DrawLevel();
			DrawObjects();
		}

		EndMode3D();

		if (!GameOver) {
			DrawText("Bomb them!!", 10, 40, 20, DARKGRAY);
			DrawFPS(10, 10);
		}
		else {
			DrawText("Game Over!", screenWidth / 2.85f, screenHeight / 3.0f, 80, RED);
			string winText = "Winner is: "+m_winner;
			DrawText(winText.c_str(), screenWidth / 2.5f, screenHeight / 2.0f, 50, RED);
		}

		EndDrawing();
	}

	//Unload music
	if (m_version >= 0.5f) { UnloadSound(m_bgMusic); }

	// De-Initialization
	CloseAudioDevice();
	CloseWindow();
	return 0;
}