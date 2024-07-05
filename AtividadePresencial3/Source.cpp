
#define _CRT_SECURE_NO_WARNINGS
#include "gl_utils.cpp"
#include "stb_image.h"
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#include "TileMap.cpp"
#include "DiamondView.h"
#include <fstream>

using namespace std;

float fw = 0.25f;
float fh = 0.25f;
int frameAtual;
int acao = 3;
float offsetx = fw * (float)frameAtual, offsety = fh * (float)acao;
int sign = 1;
float previous = glfwGetTime();

int g_gl_width = 900;
int g_gl_height = 900;
float xi = -1.0f;
float xf = 1.0f;
float yi = -1.0f;
float yf = 1.0f;
float w = xf - xi;
float h = yf - yi;
float tw, th, tw2, th2;
int tileSetCols = 9, tileSetRows = 9;
float tileW, tileW2;
float tileH, tileH2;
int cx , cy ;
float tx, ty;
int collectedCoins;
bool jogoFinalizado = false;

TilemapView* tview = new DiamondView();
TileMap* tmap = NULL;
TileMap* collideMap = NULL;

struct Coin {
	int cx;
	int cy;
	bool create;

	Coin(int cx, int cy, bool create) : cx(cx), cy(cy), create(create) {}
};

std::vector<Coin> coins;

GLFWwindow* g_window = NULL;

TileMap* readMap(const char* filename) {
	ifstream arq(filename);
	int w, h;
	arq >> w >> h;
	TileMap* tmap = new TileMap(w, h, 0);
	for (int r = 0; r < h; r++) {
		for (int c = 0; c < w; c++) {
			int tid;
			arq >> tid;
			tmap->setTile(c, h - r - 1, tid);
		}
	}
	arq.close();
	return tmap;
}

int loadTexture(unsigned int& texture, const char* filename)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


	int width, height, nrChannels;

	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (nrChannels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			cout << "Without Alpha channel" << endl;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	return 1;
}

void initializeCoins() {
	coins.emplace_back(5, 13, true);   // Moeda 1
	coins.emplace_back(12, 12, true);  // Moeda 2
	coins.emplace_back(9, 8, true);    // Moeda 3
	coins.emplace_back(0, 7, true);    // Moeda 4
	coins.emplace_back(11, 0, true);   // Moeda 5
}


void createCoin(GLuint moedaTexture, GLuint shader_programme, int cx, int cy) {
	glUniform1f(glGetUniformLocation(shader_programme, "isObject"), true); 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, moedaTexture);
	glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);

	tview->computeDrawPosition(cx, cy, tw, th, tx, ty);
	glUniform1f(glGetUniformLocation(shader_programme, "tx"), 1.23f + tx); 
	glUniform1f(glGetUniformLocation(shader_programme, "ty"), 0.78f + ty);
	glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), 1);
	glUniform1f(glGetUniformLocation(shader_programme, "offsety"), 1);
	glUniform1f(glGetUniformLocation(shader_programme, "applyOffset"), false);
	glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), 0.10f);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void renderCoins(GLuint moedaTexture, GLuint shader_programme) {
	for (const auto& coin : coins) {
		if (coin.create) {
			createCoin(moedaTexture, shader_programme, coin.cx, coin.cy);
		}
	}
}

void moveCharacter(int c, int r, const int direction) {
	tview->computeTileWalking(c, r, direction);

	if (c == 5 && r == 13 && coins[0].create) {
		collectedCoins++;
		coins[0].create = false;
	}
	else if (c == 12 && r == 12 && coins[1].create) {
		collectedCoins++;
		coins[1].create = false;
	}
	else if (c == 9 && r == 8 && coins[2].create) {
		collectedCoins++;
		coins[2].create = false;
	}
	else if (c == 0 && r == 7 && coins[3].create) {
		collectedCoins++;
		coins[3].create = false;
	}
	else if (c == 11 && r == 0 && coins[4].create) {
		collectedCoins++;
		coins[4].create = false;
	}


	if ((c < 0) || (c >= collideMap->getWidth()) || (r < 0) || (r >= collideMap->getHeight())) {
		cout << "Game Over! O cavaleiro caiu no abismo... Para jogar novamente, pressione espaco." << endl;
		jogoFinalizado = true;
		cx = -1;
		cy = -1;
		return; 
	}

	if (c == 14 && r == 10) {
		if (collectedCoins == 5) {
			cout << "Voce Venceu! O cavaleiro chegou ao seu destino final. Para jogar novamente, pressione espaco." << endl;
			jogoFinalizado = true;
			cx = -1;
			cy = -1;
			return;
		}
		else {
			cout << "Voce ainda nao coletou todas a moedas. Colete-as para ganhar o jogo." << endl;
		}
	}	

	unsigned char t_id = collideMap->getTile(c, r);
	
	if (t_id == 0) {
		cout << "Game Over! O cavaleiro caiu na agua... Para jogar novamente, pressione espaco." << endl;
		jogoFinalizado = true;
		cx = -1;
		cy = -1;
		return;
	};


	cx = c; cy = r;
}

void restart() {
	frameAtual = 0;
	acao = 3;
	collectedCoins = 0;
	offsetx = fw * (float)frameAtual, offsety = fh * (float)acao;

	for (auto& coin : coins) {
		coin.create = true;
	}
	
	cx = 0;
	cy = 0;
	jogoFinalizado = false;
}

int main()
{
	restart_gl_log();
	start_gl();
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	tmap = readMap("terrain1.tmap");
	collideMap = readMap("collide.tmap");
	tw = w / (float)tmap->getWidth();
	th = tw / 2.0f;
	tw2 = th;
	th2 = th / 2.0f;
	tileW = 1.0f / (float)tileSetCols;
	tileW2 = tileW / 2.0f;
	tileH = 1.0f / (float)tileSetRows;
	tileH2 = tileH / 2.0f;

	
	GLuint tid;
	loadTexture(tid, "terrain.png");

	tmap->setTid(tid);

	GLuint texturaObjeto;
	loadTexture(texturaObjeto, "character.png");

	GLuint moedaTexture;
	loadTexture(moedaTexture, "coin.png");


	float verticesCenario[] = {
		// positions   // texture coords
		xi    , yi + th2, 0.0f, tileH2,   // left
		xi + tw2, yi    , tileW2, 0.0f,   // bottom
		xi + tw , yi + th2, tileW, tileH2,  // right
		xi + tw2, yi + th , tileW2, tileH,  // top
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		3, 1, 2  // second triangle
	};

	float verticesObjeto[] = {
		 -2.6f * 0.8f, -0.8f * 0.8f, 0.25f, 0.25f, // top right
		 -2.6f * 0.8f, -1.0f * 0.8f, 0.25f, 0.0f, // bottom right
		 -2.8f * 0.8f, -1.0f * 0.8f, 0.0f, 0.0f, // bottom left
		 -2.8f * 0.8f, -0.8f * 0.8f, 0.0f, 0.25f, // top left
	};

	float verticesMoeda[] = {
		 -2.6f * 0.2f, -0.8f * 0.2f, 0.25f, 0.25f, // top right
		 -2.6f * 0.2f, -1.0f * 0.2f, 0.25f, 0.0f, // bottom right
		 -2.8f * 0.2f, -1.0f * 0.2f, 0.0f, 0.0f, // bottom left
		 -2.8f * 0.2f, -0.8f * 0.2f, 0.0f, 0.25f, // top left
	};

	
	unsigned int VBOCenario, VBOObjeto, VBOMoeda, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOCenario);
	glGenBuffers(1, &VBOObjeto);
	glGenBuffers(1, &VBOMoeda);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	
	glBindBuffer(GL_ARRAY_BUFFER, VBOCenario);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCenario), verticesCenario, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	
	glBindBuffer(GL_ARRAY_BUFFER, VBOObjeto);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesObjeto), verticesObjeto, GL_STATIC_DRAW);
	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(3);


	glBindBuffer(GL_ARRAY_BUFFER, VBOMoeda);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesMoeda), verticesMoeda, GL_STATIC_DRAW);

	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(5);

	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);


	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	parse_file_into_str("vertex_shader.glsl", vertex_shader, 1024 * 256);
	parse_file_into_str("fragment_shader.glsl", fragment_shader, 1024 * 256);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* p = (const GLchar*)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);


	// check for compile errors
	int params = -1;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: GL shader index %i did not compile.\n", vs);
		print_shader_info_log(vs);
		return 1; // or exit or something
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const GLchar*)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);

	// check for compile errors
	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: GL shader index %i did not compile.\n", fs);
		print_shader_info_log(fs);
		return 1; // or exit or something
	}

	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params)
	{
		fprintf(stderr, "ERROR: could not link shader programme GL index %i.\n",
			shader_programme);
		return false;
	}

	for (int r = 0; r < tmap->getHeight(); r++) {
		for (int c = 0; c < tmap->getWidth(); c++) {
			unsigned char t_id = tmap->getTile(c, r);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	bool rightPressed = false;
	bool leftPressed = false;
	bool upPressed = false;
	bool downPressed = false;
	bool shiftPressed = false;
	bool ctrlPressed = false;
	bool spacePressed = false;

	initializeCoins();

	glfwSetWindowTitle(g_window, "Jogo Trabalho GB - Laura Skorupski e Willian da Rosa");

	cout << "Bem-vindo ao jogo! Colete todas a moedas e tente fazer o cavaleiro chegar ao destino final sem sair do mapa ou cair na agua." << endl;

	while (!glfwWindowShouldClose(g_window))
	{

		double current_seconds = glfwGetTime();

		// wipe the drawing surface clear
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glViewport(0, 0, g_gl_width, g_gl_height);

		glUseProgram(shader_programme);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBOCenario);
		glUniform1f(glGetUniformLocation(shader_programme, "isObject"), false);

		float x, y;
		int r = 0, c = 0;
		for (int r = 0; r < tmap->getHeight(); r++) {
			for (int c = 0; c < tmap->getWidth(); c++) {
				int t_id = (int)tmap->getTile(c, r);
				int u = t_id % tileSetCols;
				int v = t_id / tileSetCols;

				tview->computeDrawPosition(c, r, tw, th, x, y);

				glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), u * tileW);
				glUniform1f(glGetUniformLocation(shader_programme, "offsety"), v * tileH);
				glUniform1f(glGetUniformLocation(shader_programme, "applyOffset"), true);
				glUniform1f(glGetUniformLocation(shader_programme, "tx"), x);
				glUniform1f(glGetUniformLocation(shader_programme, "ty"), y + 1.0);
				glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), 0.50);

				
				if (c == tmap->getWidth() - 1 && r == (tmap->getHeight() - 5)) {
					glUniform1f(glGetUniformLocation(shader_programme, "weight"), 0.5);
				}
				else {
					glUniform1f(glGetUniformLocation(shader_programme, "weight"), (c == cx) && (r == cy) ? 0.38 : 0.0);
				}

				glBindTexture(GL_TEXTURE_2D, tmap->getTileSet());
				glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBOObjeto);
		glUniform1f(glGetUniformLocation(shader_programme, "isObject"), true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturaObjeto);
		glUniform1i(glGetUniformLocation(shader_programme, "sprite"), 0);


		glUseProgram(shader_programme);

		tview->computeDrawPosition(cx, cy, tw, th, tx, ty);
		glUniform1f(glGetUniformLocation(shader_programme, "tx"), 1.22 + tx);
		glUniform1f(glGetUniformLocation(shader_programme, "ty"), 0.82 + ty);
		glUniform1f(glGetUniformLocation(shader_programme, "offsetx"), offsetx);
		glUniform1f(glGetUniformLocation(shader_programme, "offsety"), offsety);
		glUniform1f(glGetUniformLocation(shader_programme, "applyOffset"), true);
		glUniform1f(glGetUniformLocation(shader_programme, "layer_z"), 0.10);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		renderCoins(moedaTexture, shader_programme);
		
		glfwPollEvents();

		const int up = glfwGetKey(g_window, GLFW_KEY_UP);
		const int down = glfwGetKey(g_window, GLFW_KEY_DOWN);
		const int right = glfwGetKey(g_window, GLFW_KEY_RIGHT);
		const int left = glfwGetKey(g_window, GLFW_KEY_LEFT);
		const int shift = glfwGetKey(g_window, GLFW_KEY_LEFT_SHIFT);
		const int ctrl = glfwGetKey(g_window, GLFW_KEY_LEFT_CONTROL);
		const int space = glfwGetKey(g_window, GLFW_KEY_SPACE);

		if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(g_window, 1);
		}

		if (GLFW_PRESS == up) {
			upPressed = true;
		}
		if (GLFW_PRESS == down) {
			downPressed = true;
		}
		if (GLFW_PRESS == right) {
			rightPressed = true;
		}
		if (GLFW_PRESS == left) {
			leftPressed = true;
		}
		if (GLFW_PRESS == shift) {
			shiftPressed = true;
		}
		if (GLFW_PRESS == ctrl) {
			ctrlPressed = true;
		}
		if (GLFW_PRESS == space) {
			spacePressed = true;
		}

		if (ctrlPressed) {
			if (GLFW_RELEASE == down && downPressed) {
				moveCharacter(cx, cy, DIRECTION_SOUTHEAST);
				acao = 2;
				frameAtual = (frameAtual + 1) % 4;
				offsetx = fw * (float)frameAtual;
				offsety = fh * (float)acao;
				downPressed = false;
			}
			else if (GLFW_RELEASE == up && upPressed) {
				moveCharacter(cx, cy, DIRECTION_NORTHEAST);
				acao = 2;
				frameAtual = (frameAtual + 1) % 4;
				offsetx = fw * (float)frameAtual;
				offsety = fh * (float)acao;
				upPressed = false;
			}

			ctrlPressed = false;
		}

		if (shiftPressed) {
			if (GLFW_RELEASE == down && downPressed) {
				moveCharacter(cx, cy, DIRECTION_SOUTHWEST);
				acao = 1;
				frameAtual = (frameAtual + 1) % 4;
				offsetx = fw * (float)frameAtual;
				offsety = fh * (float)acao;
				downPressed = false;
			}
			else if (GLFW_RELEASE == up && upPressed) {
				moveCharacter(cx, cy, DIRECTION_NORTHWEST);
				acao = 1;
				frameAtual = (frameAtual + 1) % 4;
				offsetx = fw * (float)frameAtual;
				offsety = fh * (float)acao;
				upPressed = false;
			}

			shiftPressed = false;
		}

		if (GLFW_RELEASE == up && upPressed) {
			moveCharacter(cx, cy, DIRECTION_NORTH);
			acao = 0;
			frameAtual = (frameAtual + 1) % 4;
			offsetx = fw * (float)frameAtual;
			offsety = fh * (float)acao;
			upPressed = false;
		}

		if (GLFW_RELEASE == down && downPressed) {
			moveCharacter(cx, cy, DIRECTION_SOUTH);
			acao = 3;
			frameAtual = (frameAtual + 1) % 4;
			offsetx = fw * (float)frameAtual;
			offsety = fh * (float)acao;
			downPressed = false;
		}

		if (GLFW_RELEASE == left && leftPressed) {
			moveCharacter(cx, cy, DIRECTION_EAST);
			acao = 1;
			frameAtual = (frameAtual + 1) % 4;
			offsetx = fw * (float)frameAtual;
			offsety = fh * (float)acao;
			leftPressed = false;
		}
		
		if (GLFW_RELEASE == right && rightPressed) {
			moveCharacter(cx, cy, DIRECTION_WEST);
			acao = 2;
			frameAtual = (frameAtual + 1) % 4;
			offsetx = fw * (float)frameAtual;
			offsety = fh * (float)acao;
			rightPressed = false;
		}

		if (GLFW_RELEASE == space && spacePressed) {
			restart();
			spacePressed = false;
		}

		glfwSwapBuffers(g_window);
	}

	glfwTerminate();
	delete tmap;
	delete collideMap;
	return 0;
}

